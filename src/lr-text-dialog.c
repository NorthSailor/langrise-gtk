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

#include "lr-text-dialog.h"

struct _LrTextDialog
{
  GtkDialog parent_instance;

  GtkWidget *title_entry;
  GtkWidget *tags_entry;
  GtkWidget *textview;

  LrText *text;
};

enum
{
  PROP_0,
  PROP_TEXT,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrTextDialog, lr_text_dialog, GTK_TYPE_DIALOG)

static void
lr_text_dialog_init (LrTextDialog *self)
{
  /* Instance initialization */
  gtk_dialog_add_button (GTK_DIALOG (self), "OK", GTK_RESPONSE_OK);
  gtk_dialog_add_button (GTK_DIALOG (self), "Cancel", GTK_RESPONSE_CANCEL);
  gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_OK);

  GtkBuilder *builder = gtk_builder_new_from_resource ("/com/langrise/Langrise/lr-text-dialog.ui");
  GtkWidget *grid = GTK_WIDGET (gtk_builder_get_object (builder, "grid"));

  GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_add (GTK_CONTAINER (box), grid);

  self->title_entry = GTK_WIDGET (gtk_builder_get_object (builder, "title_entry"));
  self->tags_entry = GTK_WIDGET (gtk_builder_get_object (builder, "tags_entry"));
  self->textview = GTK_WIDGET (gtk_builder_get_object (builder, "textview"));

  g_object_unref (builder);
}

static void
dialog_response (GtkDialog *dialog, gint response_id)
{
  LrTextDialog *self = LR_TEXT_DIALOG (dialog);
  if (response_id == GTK_RESPONSE_OK)
    {
      /* Update the associated text structure. */
      lr_text_set_title (self->text, gtk_entry_get_text (GTK_ENTRY (self->title_entry)));
      lr_text_set_tags (self->text, gtk_entry_get_text (GTK_ENTRY (self->tags_entry)));

      GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));
      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);
      gchar *text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
      lr_text_set_text (self->text, text);
      g_free (text);
    }
}

static void
lr_text_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (lr_text_dialog_parent_class)->finalize (object);
}

static void
lr_text_dialog_set_property (GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  LrTextDialog *self = LR_TEXT_DIALOG (object);

  switch (property_id)
    {
    case PROP_TEXT:
      lr_text_dialog_set_text (self, LR_TEXT (g_value_get_object (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_text_dialog_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  LrTextDialog *self = LR_TEXT_DIALOG (object);

  switch (property_id)
    {
    case PROP_TEXT:
      g_value_set_object (value, lr_text_dialog_get_text (self));
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_text_dialog_class_init (LrTextDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_text_dialog_finalize;
  object_class->set_property = lr_text_dialog_set_property;
  object_class->get_property = lr_text_dialog_get_property;

  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);
  dialog_class->response = dialog_response;

  /* Add additional properties to obj_properties here */

  obj_properties[PROP_TEXT] = g_param_spec_object ("text",
                                                   "Text",
                                                   "The LrText associated with the dialog.",
                                                   LR_TYPE_TEXT,
                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

GtkWidget *
lr_text_dialog_new (LrText *text)
{
  return g_object_new (LR_TYPE_TEXT_DIALOG, "text", text, "use-header-bar", TRUE, NULL);
}

void
lr_text_dialog_set_text (LrTextDialog *self, LrText *text)
{
  self->text = text;

  /* Load the text data into the controls. */
  gtk_entry_set_text (GTK_ENTRY (self->title_entry), lr_text_get_title (self->text));
  gtk_entry_set_text (GTK_ENTRY (self->tags_entry), lr_text_get_tags (self->text));

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->textview));
  gtk_text_buffer_set_text (buffer, lr_text_get_text (self->text), -1);
}

LrText *
lr_text_dialog_get_text (LrTextDialog *self)
{
  return self->text;
}

