/* 
 * Langrise, expanding L2 vocabulary in context.
 * Copyright (C) 2019 Iason Barmparesos
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "export-text.h"
#include "lr-splitter.h"

char *
get_filename (GtkWindow *toplevel, const gchar *filter_name, const gchar *filter)
{
  GtkWidget *dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;

  dialog = gtk_file_chooser_dialog_new ("Save File",
                                        GTK_WINDOW (toplevel),
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Save",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  // gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

  GtkFileFilter *file_filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (file_filter, filter);
  gtk_file_filter_set_name (file_filter, filter_name);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), file_filter);

  GtkFileFilter *all_filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (all_filter, "*");
  gtk_file_filter_set_name (all_filter, "All files");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), all_filter);

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  char *filename = NULL;

  if (res == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      filename = gtk_file_chooser_get_filename (chooser);
    }

  gtk_widget_destroy (dialog);

  return filename;
}

void
lr_export_text (GtkWidget *toplevel,
                LrDatabase *db,
                GList *items,
                const gchar *prefix,
                const gchar *postfix,
                const gchar *field_sep,
                const gchar *preamble,
                const gchar *postamble,
                const gchar *filter_name,
                const gchar *filter)
{
  /* Get the file path to export to */
  char *filename = get_filename (GTK_WINDOW (toplevel), filter_name, filter);

  if (!filename)
    {
      g_warning ("Exporting aborted!");
      return;
    }

  GFile *file = g_file_new_for_path (filename);
  GFileOutputStream *ostream = g_file_replace (file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL);

  g_output_stream_write (
    G_OUTPUT_STREAM (ostream), preamble, g_utf8_strlen (preamble, -1), NULL, NULL);

  LrText *text = NULL;
  LrSplitter *splitter = NULL;

  for (GList *l = items; l != NULL; l = l->next)
    {
      lr_vocabulary_item_t *item = (lr_vocabulary_item_t *)l->data;
      if (text != item->text)
        {
          /* Load the text and split it */
          g_clear_object (&splitter); /* Destroy the old one */
          g_clear_object (&text);

          text = item->text;
          lr_database_load_text (db, text);

          splitter = lr_splitter_new (text);
        }

      GList *selection = lr_splitter_ranges_from_string (splitter, item->words);

      gchar *context, *answer;
      lr_splitter_context_from_selection (splitter, &selection, &context, &answer, "______");

      GString *line_str = g_string_new (prefix);
      g_string_append (line_str, context);
      g_string_append (line_str, field_sep);
      g_string_append (line_str, answer);
      g_string_append (line_str, field_sep);
      g_string_append (line_str, item->lemma);
      g_string_append (line_str, field_sep);
      g_string_append (line_str, item->translation);
      g_string_append (line_str, field_sep);
      g_string_append (line_str, item->note);
      g_string_append (line_str, postfix);

      /* Write the line to the stream */
      g_output_stream_write (G_OUTPUT_STREAM (ostream), line_str->str, line_str->len, NULL, NULL);

      g_string_free (line_str, TRUE);

      g_free (context);
      g_free (answer);

      g_list_free (selection);
    }

  g_clear_object (&splitter);

  g_output_stream_write (
    G_OUTPUT_STREAM (ostream), postamble, g_utf8_strlen (postamble, -1), NULL, NULL);

  g_output_stream_close (G_OUTPUT_STREAM (ostream), NULL, NULL);

  g_free (filename);
}
