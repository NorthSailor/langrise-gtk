#include "lr-reader.h"
#include <gtk/gtk.h>

struct _LrReader
{
  GtkBox parent_instance;

  LrText *text;

  GtkWidget *textview;
  GtkWidget *word_popover;
  GtkWidget *word_label;
};

G_DEFINE_TYPE (LrReader, lr_reader, GTK_TYPE_BOX)

static gboolean
lr_reader_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  LrReader *reader = LR_READER (user_data);
  GtkWidget *textview = reader->textview;
  g_message ("Mouse click at (%f, %f)", event->x, event->y);
  gint win_x = (gint)event->x;
  gint win_y = (gint)event->y;

  gint buff_x, buff_y;
  gtk_text_view_window_to_buffer_coords (
    GTK_TEXT_VIEW (textview), GTK_TEXT_WINDOW_WIDGET, win_x, win_y, &buff_x, &buff_y);

  GtkTextIter click_iter;
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (textview), &click_iter, buff_x, buff_y);

  /* Make sure the user clicked a word. */
  if (gtk_text_iter_inside_word (&click_iter) == FALSE)
    {
      g_message ("Clicked outside of word!");
      return TRUE;
    }

  GtkTextIter word_start = click_iter;
  GtkTextIter word_end = click_iter;

  GtkTextIter sentence_start = click_iter, sentence_end = click_iter;

  if (!gtk_text_iter_starts_word (&word_start))
    gtk_text_iter_backward_word_start (&word_start);

  if (!gtk_text_iter_ends_word (&word_end))
    gtk_text_iter_forward_word_end (&word_end);

  if (!gtk_text_iter_starts_sentence (&sentence_start))
    gtk_text_iter_backward_sentence_start (&sentence_start);

  if (!gtk_text_iter_ends_sentence (&sentence_end))
    gtk_text_iter_forward_sentence_end (&sentence_end);

  gchar *word = gtk_text_iter_get_text (&word_start, &word_end);
  g_message ("Clicked on word: '%s'", word);

  gchar *sentence = gtk_text_iter_get_text (&sentence_start, &sentence_end);
  g_message ("Clicked on sentence: '%s'", sentence);

  GdkRectangle start_rect;
  GdkRectangle end_rect;
  gtk_text_view_get_iter_location (GTK_TEXT_VIEW (textview), &word_start, &start_rect);
  gtk_text_view_get_iter_location (GTK_TEXT_VIEW (textview), &word_end, &end_rect);

  gint word_start_x, word_end_x;
  GdkRectangle word_rect;

  gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (textview),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         start_rect.x,
                                         start_rect.y,
                                         &word_start_x,
                                         NULL);
  gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (textview),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         end_rect.x,
                                         end_rect.y,
                                         &word_end_x,
                                         &word_rect.y);

  word_rect.x = word_start_x;
  word_rect.width = word_end_x - word_start_x; /* Assuming no line breaks!!! */
  word_rect.height = start_rect.height;

  gtk_popover_set_pointing_to (GTK_POPOVER (reader->word_popover), &word_rect);

  gchar *message = g_strdup_printf ("<b>%s</b>\n%s", word, sentence);
  gtk_label_set_markup (GTK_LABEL (reader->word_label), message);
  g_free (message);
  gtk_popover_popup (GTK_POPOVER (reader->word_popover));

  return TRUE;
}

static void
apply_textview_style (GtkWidget *view)
{
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 20);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 20);
  gtk_text_view_set_top_margin (GTK_TEXT_VIEW (view), 20);
  gtk_text_view_set_bottom_margin (GTK_TEXT_VIEW (view), 20);
}

static void
add_form_button (GtkWidget *button, gpointer data)
{
  GtkPopover *popover = GTK_POPOVER (data);
  gtk_popover_popdown (popover);
}

static void
lr_reader_init (LrReader *reader)
{
  gtk_widget_init_template (GTK_WIDGET (reader));
  gtk_widget_add_events (GTK_WIDGET (reader), GDK_KEY_PRESS_MASK);
  gtk_widget_set_can_focus (GTK_WIDGET (reader), TRUE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (reader->textview), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (reader->textview), FALSE);

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (reader->textview), GTK_WRAP_WORD);

  g_signal_connect (
    reader->textview, "button-press-event", (GCallback)lr_reader_button_press_event, reader);

  apply_textview_style (reader->textview);

  reader->word_popover = gtk_popover_new (reader->textview);

  reader->word_label = gtk_label_new ("Definition goes here");
  GtkWidget *pop_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add (GTK_CONTAINER (pop_box), reader->word_label);
  gtk_container_add (GTK_CONTAINER (pop_box), gtk_button_new_with_label ("New lexeme"));
  GtkWidget *form_button = gtk_button_new_with_label ("Add form");
  g_signal_connect (form_button, "clicked", (GCallback)add_form_button, reader->word_popover);

  gtk_container_add (GTK_CONTAINER (pop_box), form_button);
  gtk_widget_show_all (pop_box);

  gtk_popover_set_position (GTK_POPOVER (reader->word_popover), GTK_POS_BOTTOM);

  gtk_container_add (GTK_CONTAINER (reader->word_popover), pop_box);
}

static void
lr_reader_class_init (LrReaderClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class, "/com/langrise/Langrise/lr-reader.ui");

  gtk_widget_class_bind_template_child (widget_class, LrReader, textview);
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
  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));
  gtk_text_buffer_set_text (text_buffer, lr_text_get_text (text), -1);
}
