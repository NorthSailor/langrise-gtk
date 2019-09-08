#include "lr-reader.h"
#include "lr-dictionary.h"
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
  GtkWidget *right_panel_box;
  GtkWidget *dictionary;

  GtkWidget *word_stack;
  GtkWidget *lemma_label;
  GtkWidget *translation_entry;
  GtkWidget *instance_note_entry;

  GtkWidget *lemmatizer_note_label;
  GtkWidget *root_form_entry;

  GtkTextTag *selection_tag;
  GtkTextTag *instance_tag;
  GtkTextTag *highlighted_instance_tag;

  /* A list of lr_range_t's */
  GList *selection;

  instance_range_t *selected_instance;
  LrLemma *active_lemma;

  GListStore *suggestions;
  GtkWidget *suggestion_listbox;
  GtkWidget *suggestion_scrolled_window;

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

  /* If only a single word is selected, set it as the text
   * of the root form entry, as a heuristic
   */
  if ((self->selection) && (self->selection->next == NULL))
    {
      /* Only a single word is selected */
      const lr_range_t *range = self->selection->data;
      const gchar *text = lr_text_get_text (self->text);
      gchar *word = g_strndup (text + range->start, (range->end - range->start));

      gtk_entry_set_text (GTK_ENTRY (self->root_form_entry), word);

      g_free (word);
    }

  gchar *message = lr_lemmatizer_populate_suggestions (
    self->lemmatizer, self->suggestions, lr_text_get_text (self->text), self->selection);
  gtk_label_set_text (GTK_LABEL (self->lemmatizer_note_label), message);
  g_free (message);

  /* If there are no suggestions, hide the list box */
  int n_suggestions = g_list_model_get_n_items (G_LIST_MODEL (self->suggestions));
  gtk_widget_set_visible (self->suggestion_scrolled_window, n_suggestions > 0);
}

static void
open_lemma_from_active_instance (LrReader *self)
{
  g_clear_object (&self->active_lemma);

  if (!self->selected_instance)
    return;

  /* Load the lemma */
  LrLemmaInstance *instance = self->selected_instance->instance;
  self->active_lemma = lr_database_load_lemma_from_instance (self->db, instance);
  g_assert (LR_IS_LEMMA (self->active_lemma));

  /* Load the lemma to the edit view */
  gtk_label_set_text (GTK_LABEL (self->lemma_label), lr_lemma_get_lemma (self->active_lemma));
  gtk_entry_set_text (GTK_ENTRY (self->translation_entry),
                      lr_lemma_get_translation (self->active_lemma));

  gtk_entry_set_text (GTK_ENTRY (self->instance_note_entry), lr_lemma_instance_get_note (instance));

  gtk_widget_grab_focus (self->translation_entry);
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

/* Highlight an instance, open the edit panel and load the lemma.
 * Clears the selection.
 */
static void
activate_instance (LrReader *self, instance_range_t *instance)
{
  clear_selection (self);
  gtk_stack_set_visible_child_name (GTK_STACK (self->word_stack), "edit-instance");
  self->selected_instance = instance;

  selection_changed (self);
  highlight_selected_instance (self);
  open_lemma_from_active_instance (self);
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

  /* If we clicked on an instance, clear the selection and
   * set that instance as the selected one.
   */
  if (selected_instance)
    {
      activate_instance (self, selected_instance);
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
        {
          /* Make sure the word is not already in the selection */
          gboolean in_selection = FALSE;
          for (GList *l = self->selection; l != NULL; l = l->next)
            {
              if (l->data == (gpointer)range)
                {
                  in_selection = TRUE;
                  break;
                }
            }
          if (!in_selection)
            self->selection = g_list_append (self->selection, (gpointer)range);
        }

      selection_changed (self);
      highlight_selected_instance (self);

      /* Set the stack to the right page */
      if (self->selection)
        {
          gtk_stack_set_visible_child_name (GTK_STACK (self->word_stack), "new-instance");
        }
      else
        {
          gtk_stack_set_visible_child_name (GTK_STACK (self->word_stack), "no-selection");
        }
    }

  return TRUE;
}

static void
lookup_root_form_cb (LrReader *self, GtkButton *button)
{
  if (gtk_entry_get_text_length (GTK_ENTRY (self->root_form_entry)))
    lr_dictionary_lookup (LR_DICTIONARY (self->dictionary),
                          gtk_entry_get_text (GTK_ENTRY (self->root_form_entry)));
}

static void
mark_instance_cb (GtkButton *button, LrReader *self)
{
  const gchar *root_form = gtk_entry_get_text (GTK_ENTRY (self->root_form_entry));

  /* Find or create the lemma for the given text */
  LrLemma *lemma = lr_lemma_new (-1, root_form, "", lr_text_get_language (self->text));
  lr_database_load_or_create_lemma (self->db, lemma);

  gchar *words = lr_splitter_selection_to_text (self->splitter, self->selection);

  /* Create a new lemma instance and set its lemma and word fields */
  LrLemmaInstance *instance =
    lr_lemma_instance_new (-1, lr_lemma_get_id (lemma), self->text, words, "");
  g_free (words);

  /* Persist it in the database */
  lr_database_insert_instance (self->db, instance);

  int new_id = lr_lemma_instance_get_id (instance);

  g_object_unref (instance);
  g_object_unref (lemma);

  /* Reload the instances */
  clear_selection (self);
  update_instances (self);

  /* Select the instance with the new id */
  instance_range_t *instance_range = NULL;

  for (GList *l = self->instance_ranges; l != NULL; l = l->next)
    {
      instance_range_t *ir = (instance_range_t *)l->data;
      int id = lr_lemma_instance_get_id (ir->instance);
      if (new_id == id)
        {
          instance_range = ir;
          break;
        }
    }
  g_assert (instance_range != NULL); /* We just added it, it has to be there! */

  activate_instance (self, instance_range);
}

/* Called when the root form entry is edited.
 * Should disable the "Mark instance" button
 * if the entry is empty.
 */
static void
root_form_changed_cb (GtkEntry *entry, GtkWidget *mark_instance_button)
{
  g_assert (GTK_IS_ENTRY (entry));
  g_assert (GTK_IS_BUTTON (mark_instance_button));

  if (gtk_entry_get_text_length (entry) == 0)
    gtk_widget_set_sensitive (mark_instance_button, FALSE);
  else
    gtk_widget_set_sensitive (mark_instance_button, TRUE);
}

static void
translation_changed_cb (GtkEntry *entry, LrReader *self)
{
  g_assert (LR_IS_READER (self));
  g_assert (LR_IS_LEMMA (self->active_lemma));

  const gchar *new_translation = gtk_entry_get_text (entry);

  lr_lemma_set_translation (self->active_lemma, new_translation);
  lr_database_update_lemma (self->db, self->active_lemma);

  gtk_widget_grab_focus (self->instance_note_entry);
}

static void
instance_note_changed_cb (GtkEntry *entry, LrReader *self)
{
  g_assert (GTK_IS_ENTRY (entry));
  g_assert (LR_IS_READER (self));

  const gchar *new_note = gtk_entry_get_text (entry);

  LrLemmaInstance *instance = self->selected_instance->instance;
  g_assert (LR_IS_LEMMA_INSTANCE (instance));

  lr_lemma_instance_set_note (instance, new_note);
  lr_database_update_instance (self->db, instance);
}

static void
remove_instance_cb (LrReader *self, GtkWidget *button)
{
  g_assert (GTK_IS_BUTTON (button));
  g_assert (LR_IS_READER (self));

  lr_database_delete_instance (self->db, self->selected_instance->instance);

  /* Close the edit panel and clear the active lemma and instance. */
  g_clear_object (&self->active_lemma);
  self->selected_instance = NULL;

  update_instances (self);
  highlight_selected_instance (self);

  gtk_stack_set_visible_child_name (GTK_STACK (self->word_stack), "no-selection");
}

static void
lookup_instance_cb (LrReader *self, GtkWidget *button)
{
  g_assert (LR_IS_READER (self));
  g_assert (LR_IS_LEMMA (self->active_lemma));

  lr_dictionary_lookup (LR_DICTIONARY (self->dictionary), lr_lemma_get_lemma (self->active_lemma));
}

static void
suggestion_selection_changed_cb (LrReader *self, GtkWidget *listbox)
{
  g_assert (LR_IS_READER (self));
  g_assert (GTK_IS_LIST_BOX (listbox));

  /* Get the current selection, and if a row is selected, copy the lemma suggestion text in
   * the lemma entry. */

  GtkListBoxRow *row = gtk_list_box_get_selected_row (GTK_LIST_BOX (listbox));

  if (row != NULL)
    {
      int row_id = gtk_list_box_row_get_index (row);
      LrLemmaSuggestion *suggestion =
        g_list_model_get_item (G_LIST_MODEL (self->suggestions), row_id);
      gtk_entry_set_text (GTK_ENTRY (self->root_form_entry),
                          lr_lemma_suggestion_get_lemma (suggestion));

      /* Move the focus to the root form entry */
      gtk_widget_grab_focus (self->root_form_entry);
    }
}

typedef struct
{
  LrReader *self;
  const gchar *text;
} suggestion_lookup_data_t;

static void
lookup_lemma_suggestion (GtkWidget *button, suggestion_lookup_data_t *data)
{
  lr_dictionary_lookup (LR_DICTIONARY (data->self->dictionary), data->text);
}

static GtkWidget *
create_widget_for_lemma_suggestion (gpointer item, LrReader *self)
{
  LrLemmaSuggestion *suggestion = LR_LEMMA_SUGGESTION (item);
  GtkWidget *row = gtk_list_box_row_new ();

  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/langrise/Langrise/lr-lemma-suggestion.ui");

  GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "box"));
  GtkWidget *label = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_label"));
  gtk_label_set_text (GTK_LABEL (label), lr_lemma_suggestion_get_lemma (suggestion));

  GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "lookup_button"));
  suggestion_lookup_data_t *data = g_malloc (sizeof (suggestion_lookup_data_t));
  data->self = self;
  data->text = lr_lemma_suggestion_get_lemma (suggestion);

  g_signal_connect_data (
    button, "clicked", (GCallback)lookup_lemma_suggestion, data, (GClosureNotify)g_free, 0);

  gtk_container_add (GTK_CONTAINER (row), box);
  gtk_widget_show_all (row);

  g_object_unref (builder);

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

  self->instance_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "instance",
                                "weight",
                                PANGO_WEIGHT_BOLD,
                                NULL);
  self->selection_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "selection",
                                "background",
                                "#ffcc00",
                                "foreground",
                                "black",
                                NULL);
  self->highlighted_instance_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "highlighted-instance",
                                "background",
                                "#ccff00",
                                "foreground",
                                "black",
                                NULL);

  self->suggestions = g_list_store_new (LR_TYPE_LEMMA_SUGGESTION);

  gtk_list_box_bind_model (GTK_LIST_BOX (self->suggestion_listbox),
                           G_LIST_MODEL (self->suggestions),
                           (GtkListBoxCreateWidgetFunc)create_widget_for_lemma_suggestion,
                           self,
                           NULL);

  self->instance_store = g_list_store_new (LR_TYPE_LEMMA_INSTANCE);

  /* Create the dictionary widget and add it to the right panel */
  self->dictionary = lr_dictionary_new ();
  gtk_box_pack_start (GTK_BOX (self->right_panel_box), self->dictionary, FALSE, FALSE, 0);

  gtk_widget_set_valign (self->dictionary, GTK_ALIGN_END);
}

static void
lr_reader_finalize (GObject *obj)
{
  LrReader *self = LR_READER (obj);

  clear_selection (self);

  g_list_free_full (self->instance_ranges, (GDestroyNotify)free_instance_range);

  g_clear_object (&self->suggestions);
  g_clear_object (&self->instance_store);
  g_clear_object (&self->active_lemma);

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
  gtk_widget_class_bind_template_child (widget_class, LrReader, right_panel_box);
  gtk_widget_class_bind_template_child (widget_class, LrReader, suggestion_listbox);
  gtk_widget_class_bind_template_child (widget_class, LrReader, suggestion_scrolled_window);

  gtk_widget_class_bind_template_child (widget_class, LrReader, word_stack);
  gtk_widget_class_bind_template_child (widget_class, LrReader, lemma_label);
  gtk_widget_class_bind_template_child (widget_class, LrReader, translation_entry);
  gtk_widget_class_bind_template_child (widget_class, LrReader, instance_note_entry);
  gtk_widget_class_bind_template_child (widget_class, LrReader, root_form_entry);
  gtk_widget_class_bind_template_child (widget_class, LrReader, lemmatizer_note_label);

  gtk_widget_class_bind_template_callback (widget_class, root_form_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, translation_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, instance_note_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, remove_instance_cb);
  gtk_widget_class_bind_template_callback (widget_class, lookup_instance_cb);
  gtk_widget_class_bind_template_callback (widget_class, lookup_root_form_cb);
  gtk_widget_class_bind_template_callback (widget_class, mark_instance_cb);
  gtk_widget_class_bind_template_callback (widget_class, suggestion_selection_changed_cb);
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

  /* Set the stack to no selection */
  gtk_stack_set_visible_child_name (GTK_STACK (self->word_stack), "no-selection");
}

