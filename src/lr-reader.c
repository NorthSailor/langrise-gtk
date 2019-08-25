#include "lr-reader.h"
#include "lr-splitter.h"
#include "lr-lemmatizer.h"
#include "lr-lemma-suggestion.h"
#include "lr-lemma.h"
#include "lr-lemma-instance.h"
#include <gtk/gtk.h>

typedef struct
{
  /* List of lr_range_t* */
  GList *words;
  LrLemmaInstance *instance;
} instance_range_t;

struct _LrReader
{
  GtkBox parent_instance;

  LrText *text;
  LrDatabase *db;

  LrSplitter *splitter;
  LrLemmatizer *lemmatizer;

  GtkWidget *textview;
  GtkWidget *word_popover;
  GtkWidget *word_label;

  GtkTextTag *selection_tag;
  GtkTextTag *instance_tag;
  GtkTextTag *highlighted_instance_tag;

  /* A list of lr_range_t's */
  GList *selection;

  instance_range_t *selected_instance;

  GListStore *suggestions;
  GtkWidget *suggestion_listbox;

  GListStore *instance_store;

  /* A list of instance_range_t */
  GList *instance_ranges;
};

G_DEFINE_TYPE (LrReader, lr_reader, GTK_TYPE_BOX)

static void
free_instance_range (instance_range_t *range)
{
  g_list_free (range->words);
  g_free (range);
}

/**
 * Returns whether a given UTF-8 index is within any of the
 * words in the given list.
 */
static gboolean
is_index_in_words (GList *words, const lr_range_t *word)
{
  for (GList *l = words; l != NULL; l = l->next)
    {
      const lr_range_t *range = l->data;
      if (word == range)
        return TRUE;
    }
  return FALSE;
}

static void selection_changed (LrReader *self);

static void
clear_selection (LrReader *self)
{
  g_list_free (self->selection);
  self->selection = NULL;
  self->selected_instance = NULL;
}

static void
apply_tag_to_word (LrReader *self, const lr_range_t *range, GtkTextTag *tag)
{
  const gchar *text = lr_text_get_text (self->text);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));

  /* Convert the byte indices to character offsets */
  int start_offset = g_utf8_pointer_to_offset (text, &text[range->start]);
  int end_offset = g_utf8_pointer_to_offset (text, &text[range->end]);

  GtkTextIter word_start, word_end;
  gtk_text_buffer_get_iter_at_offset (buffer, &word_start, start_offset);
  gtk_text_buffer_get_iter_at_offset (buffer, &word_end, end_offset);

  gtk_text_buffer_apply_tag (buffer, tag, &word_start, &word_end);
}

static void
apply_selection_tag (LrReader *self)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));

  /* Remove all previously applied instances of the selection tag */
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds (buffer, &start, &end);

  gtk_text_buffer_remove_tag (buffer, self->selection_tag, &start, &end);

  /* Apply the tag for each selected word */
  for (GList *l = self->selection; l != NULL; l = l->next)
    {
      const lr_range_t *range = (const lr_range_t *)l->data;
      apply_tag_to_word (self, range, self->selection_tag);
    }
}

static void
highlight_selected_instance (LrReader *self)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));

  GtkTextIter start, end;
  gtk_text_buffer_get_bounds (buffer, &start, &end);

  gtk_text_buffer_remove_tag (buffer, self->highlighted_instance_tag, &start, &end);

  if (self->selected_instance)
    {
      GList *word_list = self->selected_instance->words;
      for (GList *l = word_list; l != NULL; l = l->next)
        {
          apply_tag_to_word (self, (lr_range_t *)l->data, self->highlighted_instance_tag);
        }
    }
}

static void
selection_changed (LrReader *self)
{
  apply_selection_tag (self);

  gchar *message = lr_lemmatizer_populate_suggestions (
    self->lemmatizer, self->suggestions, lr_text_get_text (self->text), self->selection);
  g_free (message);
}

static void
selected_instance_updated (LrReader *self)
{
  /* TODO Unload the previous lemma and save any changes */
  if (!self->selected_instance)
    {
      g_message ("Instance selection cleared");
      return;
    }

  /* Load the lemma */
  LrLemmaInstance *instance = self->selected_instance->instance;
  LrLemma *lemma = lr_database_load_lemma_from_instance (self->db, instance);
  g_assert (LR_IS_LEMMA (lemma));

  g_message (
    "Selected lemma '%s' -> '%s'", lr_lemma_get_lemma (lemma), lr_lemma_get_translation (lemma));

  g_object_unref (lemma);
}

static void
update_instances (LrReader *self)
{
  lr_database_populate_lemma_instances (self->db, self->instance_store, self->text);

  g_list_free_full (self->instance_ranges, (GDestroyNotify)free_instance_range);
  self->instance_ranges = NULL;

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));

  GtkTextIter start, end;
  gtk_text_buffer_get_bounds (buffer, &start, &end);

  gtk_text_buffer_remove_tag (buffer, self->instance_tag, &start, &end);

  int n_instances = g_list_model_get_n_items (G_LIST_MODEL (self->instance_store));
  for (int i = 0; i < n_instances; ++i)
    {
      LrLemmaInstance *instance = g_list_model_get_item (G_LIST_MODEL (self->instance_store), i);

      GList *ranges =
        lr_splitter_ranges_from_string (self->splitter, lr_lemma_instance_get_words (instance));

      instance_range_t *instance_range = g_malloc (sizeof (instance_range_t));
      instance_range->words = ranges;
      instance_range->instance = instance;

      self->instance_ranges = g_list_append (self->instance_ranges, instance_range);

      g_object_unref (instance);

      for (GList *l = ranges; l != NULL; l = l->next)
        {
          const lr_range_t *range = (const lr_range_t *)l->data;
          apply_tag_to_word (self, range, self->instance_tag);
        }
    }
}

static gboolean
lr_reader_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  LrReader *self = LR_READER (user_data);
  GtkWidget *textview = self->textview;
  gint win_x = (gint)event->x;
  gint win_y = (gint)event->y;

  gint buff_x, buff_y;
  gtk_text_view_window_to_buffer_coords (
    GTK_TEXT_VIEW (textview), GTK_TEXT_WINDOW_WIDGET, win_x, win_y, &buff_x, &buff_y);

  GtkTextIter click_iter;
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (textview), &click_iter, buff_x, buff_y);

  const char *text = lr_text_get_text (self->text);
  const char *byte_index = g_utf8_offset_to_pointer (text, gtk_text_iter_get_offset (&click_iter));
  int index = byte_index - text;

  const lr_range_t *range = lr_splitter_get_word_at_index (self->splitter, index);

  instance_range_t *selected_instance = NULL;
  for (GList *l = self->instance_ranges; l != NULL; l = l->next)
    {
      GList *words = ((instance_range_t *)l->data)->words;
      if (is_index_in_words (words, range))
        selected_instance = (instance_range_t *)l->data;
    }

  /* If we clicked on a reference, clear the selection and
   * set that reference as the selected one.
   */
  if (selected_instance)
    {
      clear_selection (self);
      /* TODO Probably do extra things with the previous instance */
      self->selected_instance = selected_instance;
    }
  else
    {
      /* Otherwise, clear the selected instance and update the selection */
      self->selected_instance = NULL;

      /* Unless the Control key was pressed, clear the previous selection */
      GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask ();
      if ((event->state & modifiers) != GDK_CONTROL_MASK)
        clear_selection (self);

      /* If a word was selected, add it to the selection */
      if (range != NULL)
        self->selection = g_list_append (self->selection, (gpointer)range);
    }

  selection_changed (self);
  highlight_selected_instance (self);
  selected_instance_updated (self);

  return TRUE;
}

static void
lemma_changed_cb (GtkEntry *entry, LrReader *self)
{
  g_assert (GTK_IS_ENTRY (entry));
  g_assert (LR_IS_READER (self));
  g_message ("lemma_changed_cb ()");
}

static void
translation_changed_cb (GtkEntry *entry, LrReader *self)
{
  g_assert (GTK_IS_ENTRY (entry));
  g_assert (LR_IS_READER (self));
  g_message ("translation_changed_cb ()");
}

static void
instance_note_changed_cb (GtkEntry *entry, LrReader *self)
{
  g_assert (GTK_IS_ENTRY (entry));
  g_assert (LR_IS_READER (self));
  g_message ("instance_note_changed_cb ()");
}

static void
remove_instance_cb (LrReader *self, GtkWidget *button)
{
  g_assert (GTK_IS_BUTTON (button));
  g_assert (LR_IS_READER (self));
}

static GtkWidget *
create_widget_for_lemma_suggestion (gpointer item, gpointer user_data)
{
  LrLemmaSuggestion *suggestion = LR_LEMMA_SUGGESTION (item);
  GtkWidget *row = gtk_list_box_row_new ();

  gtk_container_add (GTK_CONTAINER (row),
                     gtk_label_new (lr_lemma_suggestion_get_lemma (suggestion)));
  gtk_widget_show_all (row);

  return row;
}

static void
lr_reader_init (LrReader *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_widget_add_events (GTK_WIDGET (self), GDK_KEY_PRESS_MASK);
  gtk_widget_set_can_focus (GTK_WIDGET (self), TRUE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (self->textview), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (self->textview), FALSE);

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->textview), GTK_WRAP_WORD);

  g_signal_connect (
    self->textview, "button-press-event", (GCallback)lr_reader_button_press_event, self);

  self->selection = NULL;

  GdkRGBA instance_color;
  gdk_rgba_parse (&instance_color, "#ffcc00");

  self->instance_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "instance",
                                "underline",
                                PANGO_UNDERLINE_SINGLE,
                                "underline-rgba",
                                &instance_color,
                                "foreground-rgba",
                                &instance_color,
                                NULL);
  self->selection_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "selection",
                                "background-rgba",
                                &instance_color,
                                NULL);
  self->highlighted_instance_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "highlighted-instance",
                                "background",
                                "#ccff00",
                                NULL);

  self->suggestions = g_list_store_new (LR_TYPE_LEMMA_SUGGESTION);

  gtk_list_box_bind_model (GTK_LIST_BOX (self->suggestion_listbox),
                           G_LIST_MODEL (self->suggestions),
                           create_widget_for_lemma_suggestion,
                           NULL,
                           NULL);

  self->instance_store = g_list_store_new (LR_TYPE_LEMMA_INSTANCE);
}

static void
lr_reader_finalize (GObject *obj)
{
  LrReader *self = LR_READER (obj);

  clear_selection (self);

  g_list_free_full (self->instance_ranges, (GDestroyNotify)free_instance_range);

  g_clear_object (&self->suggestions);
  g_clear_object (&self->instance_store);

  G_OBJECT_CLASS (lr_reader_parent_class)->finalize (obj);
}

static void
lr_reader_class_init (LrReaderClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  obj_class->finalize = lr_reader_finalize;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class, "/com/langrise/Langrise/lr-reader.ui");

  gtk_widget_class_bind_template_child (widget_class, LrReader, textview);
  gtk_widget_class_bind_template_child (widget_class, LrReader, suggestion_listbox);

  gtk_widget_class_bind_template_callback (widget_class, lemma_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, translation_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, instance_note_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, remove_instance_cb);
}

GtkWidget *
lr_reader_new (void)
{
  return g_object_new (LR_TYPE_READER, NULL);
}

void
lr_reader_set_text (LrReader *self, LrText *text, LrDatabase *db)
{
  g_assert (LR_IS_TEXT (text));
  g_assert (LR_IS_DATABASE (db));

  self->text = text;
  self->db = db;

  /* Destroy the old splitter (if any) and create a new one */
  g_clear_object (&self->splitter);
  self->splitter = lr_splitter_new (self->text);

  /* Destroy the old lemmatizer (if any) and create a new one */
  g_clear_object (&self->lemmatizer);
  const gchar *lang_code = lr_language_get_code (lr_text_get_language (text));
  self->lemmatizer = lr_lemmatizer_new_for_language (lang_code);

  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));
  gtk_text_buffer_set_text (text_buffer, lr_text_get_text (text), -1);

  clear_selection (self);

  update_instances (self);
}

