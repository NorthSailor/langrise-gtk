#include "lr-reader.h"
#include "lr-splitter.h"
#include "lr-lemmatizer.h"
#include "lr-lemma-suggestion.h"
#include <gtk/gtk.h>

struct _LrReader
{
  GtkBox parent_instance;

  LrText *text;

  LrSplitter *splitter;
  LrLemmatizer *lemmatizer;

  GtkWidget *textview;
  GtkWidget *word_popover;
  GtkWidget *word_label;

  GtkTextTag *selection_tag;

  /* A list of lr_range_t's */
  GList *selection;

  GListStore *suggestions;
  GtkWidget *suggestion_listbox;
};

G_DEFINE_TYPE (LrReader, lr_reader, GTK_TYPE_BOX)

static void selection_changed (LrReader *self);

static void
clear_selection (LrReader *self)
{
  g_list_free (self->selection);
  self->selection = NULL;
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
  const gchar *text = lr_text_get_text (self->text);
  for (GList *l = self->selection; l != NULL; l = l->next)
    {
      lr_range_t *range = (lr_range_t *)l->data;

      /* Convert the byte indices to character offsets */
      int start_offset = g_utf8_pointer_to_offset (text, &text[range->start]);
      int end_offset = g_utf8_pointer_to_offset (text, &text[range->end]);

      GtkTextIter word_start, word_end;
      gtk_text_buffer_get_iter_at_offset (buffer, &word_start, start_offset);
      gtk_text_buffer_get_iter_at_offset (buffer, &word_end, end_offset);

      gtk_text_buffer_apply_tag (buffer, self->selection_tag, &word_start, &word_end);
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

  /* Unless the Control key was pressed, clear the previous selection */
  GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask ();
  if ((event->state & modifiers) != GDK_CONTROL_MASK)
    clear_selection (self);

  /* If a word was selected, add it to the selection */
  if (range != NULL)
    self->selection = g_list_append (self->selection, (gpointer)range);

  selection_changed (self);
  return TRUE;
}

static void
add_form_button (GtkWidget *button, gpointer data)
{
  GtkPopover *popover = GTK_POPOVER (data);
  gtk_popover_popdown (popover);
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

  self->word_popover = gtk_popover_new (self->textview);

  self->word_label = gtk_label_new ("Definition goes here");
  GtkWidget *pop_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add (GTK_CONTAINER (pop_box), self->word_label);
  gtk_container_add (GTK_CONTAINER (pop_box), gtk_button_new_with_label ("New lexeme"));
  GtkWidget *form_button = gtk_button_new_with_label ("Add form");
  g_signal_connect (form_button, "clicked", (GCallback)add_form_button, self->word_popover);

  gtk_container_add (GTK_CONTAINER (pop_box), form_button);
  gtk_widget_show_all (pop_box);

  gtk_popover_set_position (GTK_POPOVER (self->word_popover), GTK_POS_BOTTOM);

  gtk_container_add (GTK_CONTAINER (self->word_popover), pop_box);

  self->selection = NULL;
  self->selection_tag =
    gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview)),
                                "selection",
                                "background",
                                "#88ddff",
                                NULL);

  self->suggestions = g_list_store_new (LR_TYPE_LEMMA_SUGGESTION);

  gtk_list_box_bind_model (GTK_LIST_BOX (self->suggestion_listbox),
                           G_LIST_MODEL (self->suggestions),
                           create_widget_for_lemma_suggestion,
                           NULL,
                           NULL);

  g_list_store_append (self->suggestions,
                       lr_lemma_suggestion_new ("to think", "1st person. present tense"));
}

static void
lr_reader_finalize (GObject *obj)
{
  LrReader *self = LR_READER (obj);

  clear_selection (self);

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
}

GtkWidget *
lr_reader_new (void)
{
  return g_object_new (LR_TYPE_READER, NULL);
}

void
lr_reader_set_text (LrReader *self, LrText *text)
{
  self->text = text;

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
}
