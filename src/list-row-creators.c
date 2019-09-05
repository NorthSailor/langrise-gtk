#include "list-row-creators.h"

GtkWidget *
create_widget_for_text (LrText *text, gpointer user_data)
{
  GtkWidget *row = gtk_list_box_row_new ();

  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/langrise/Langrise/lr-text-selector-row.ui");

  GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "box"));
  GtkWidget *title_label = GTK_WIDGET (gtk_builder_get_object (builder, "title_label"));
  GtkWidget *tags_label = GTK_WIDGET (gtk_builder_get_object (builder, "tags_label"));

  gtk_label_set_text (GTK_LABEL (title_label), lr_text_get_title (text));
  gtk_label_set_text (GTK_LABEL (tags_label), lr_text_get_tags (text));

  gtk_container_add (GTK_CONTAINER (row), box);
  gtk_widget_show_all (row);

  g_object_unref (builder);

  return row;
}
