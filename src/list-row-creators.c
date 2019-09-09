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
