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

#include "lr-text-selector.h"
#include "lr-text.h"
#include "lr-text-dialog.h"
#include "list-row-creators.h"

struct _LrTextSelector
{
  GtkBox parent_instance;

  LrDatabase *db;
  LrLanguage *lang; /* Active language */

  GtkWidget *title_label;
  GtkWidget *read_button;
  GtkWidget *edit_button;
  GtkWidget *delete_button;
  GtkWidget *list_box;

  GListStore *text_store;

  int selected_index;
  LrText *selected_text; /* Currently selected text or NULL */
};

G_DEFINE_TYPE (LrTextSelector, lr_text_selector, GTK_TYPE_BOX)

/* Signals */
enum
{
  READ_TEXT,
  TEXT_MODIFIED,
  N_SIGNALS
};

static guint obj_signals[N_SIGNALS] = {
  0,
};

static void
populate_text_list (LrTextSelector *self)
{
  lr_database_populate_texts (self->db, self->text_store, self->lang);
}

static void
new_text_cb (LrTextSelector *self, GtkWidget *button)
{
  LrText *new_text = lr_text_new (-1, self->lang, "", "");
  lr_text_set_text (new_text, "");

  GtkWidget *text_dialog = lr_text_dialog_new (new_text);
  gtk_window_set_title (GTK_WINDOW (text_dialog), "New text");
  gtk_window_set_transient_for (GTK_WINDOW (text_dialog),
                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self))));

  int response = gtk_dialog_run (GTK_DIALOG (text_dialog));
  gtk_widget_destroy (text_dialog);

  if (response == GTK_RESPONSE_OK)
    {
      lr_database_insert_text (self->db, new_text);
      populate_text_list (self);
      g_signal_emit (self, obj_signals[TEXT_MODIFIED], 0);
    }

  g_object_unref (new_text);
}

static void
read_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_assert (LR_IS_TEXT (self->selected_text));
  g_signal_emit (self, obj_signals[READ_TEXT], 0, self->selected_text);
}

static void
edit_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_assert (self->selected_index >= 0);
  g_assert (LR_IS_TEXT (self->selected_text));

  /* Load the text if it's not loaded yet. */
  if (lr_text_get_text (self->selected_text) == NULL)
    lr_database_load_text (self->db, self->selected_text);

  GtkWidget *text_dialog = lr_text_dialog_new (self->selected_text);

  gchar *title = g_strdup_printf ("Edit text '%s'", lr_text_get_title (self->selected_text));
  gtk_window_set_title (GTK_WINDOW (text_dialog), title);
  g_free (title);

  gtk_window_set_transient_for (GTK_WINDOW (text_dialog),
                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self))));

  int response = gtk_dialog_run (GTK_DIALOG (text_dialog));

  gtk_widget_destroy (text_dialog);

  if (response == GTK_RESPONSE_OK)
    {
      lr_database_update_text (self->db, self->selected_text);
      populate_text_list (self);
      g_signal_emit (self, obj_signals[TEXT_MODIFIED], 0);
    }
}

static void
delete_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_assert (self->selected_index >= 0);
  g_assert (LR_IS_TEXT (self->selected_text));

  GtkWidget *message_box = gtk_message_dialog_new (
    GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self))),
    GTK_DIALOG_USE_HEADER_BAR | GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_WARNING,
    GTK_BUTTONS_YES_NO,
    "Delete text '%s'?",
    lr_text_get_title (self->selected_text));

  int answer = gtk_dialog_run (GTK_DIALOG (message_box));
  gtk_widget_destroy (message_box);

  switch (answer)
    {
    case GTK_RESPONSE_YES:
      lr_database_delete_text (self->db, self->selected_text);
      g_list_store_remove (self->text_store, self->selected_index);
      populate_text_list (self);
      g_signal_emit (self, obj_signals[TEXT_MODIFIED], 0);
      break;
    case GTK_RESPONSE_NO:
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_NONE:
    default:
      break;
    }
}

static void
selection_changed_cb (GtkListBox *box, LrTextSelector *self)
{
  g_assert (LR_IS_TEXT_SELECTOR (self));
  g_assert (GTK_IS_LIST_BOX (box));

  /* If no items are selected, disable the editing controls */
  GtkListBoxRow *row = gtk_list_box_get_selected_row (box);

  gtk_widget_set_sensitive (self->read_button, row != NULL);
  gtk_widget_set_sensitive (self->edit_button, row != NULL);
  gtk_widget_set_sensitive (self->delete_button, row != NULL);

  /* Unref the previously selected text */
  g_clear_object (&self->selected_text);

  /* Update the selected text */
  if (row == NULL)
    {
      self->selected_index = -1;
      self->selected_text = NULL;
    }
  else
    {
      self->selected_index = gtk_list_box_row_get_index (row);
      self->selected_text =
        g_list_model_get_item (G_LIST_MODEL (self->text_store), self->selected_index);
    }
}

static void
lr_text_selector_init (LrTextSelector *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  self->db = NULL;
  self->lang = NULL;

  self->text_store = g_list_store_new (LR_TYPE_TEXT);

  gtk_list_box_bind_model (GTK_LIST_BOX (self->list_box),
                           G_LIST_MODEL (self->text_store),
                           (GtkListBoxCreateWidgetFunc)create_widget_for_text,
                           NULL,
                           NULL);
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (self->list_box), GTK_SELECTION_SINGLE);
  gtk_list_box_set_placeholder (GTK_LIST_BOX (self->list_box), gtk_label_new ("No texts found"));
}

static void
lr_text_selector_finalize (GObject *obj)
{
  g_assert (LR_IS_TEXT_SELECTOR (obj));

  LrTextSelector *self = LR_TEXT_SELECTOR (obj);

  g_clear_object (&self->selected_text);
  g_clear_object (&self->text_store);
}

static void
lr_text_selector_class_init (LrTextSelectorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_text_selector_finalize;

  /* Register the signals */
  obj_signals[READ_TEXT] = g_signal_new ("read-text",
                                         G_TYPE_FROM_CLASS (klass),
                                         G_SIGNAL_RUN_LAST,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         G_TYPE_NONE,
                                         1,
                                         LR_TYPE_TEXT);
  obj_signals[TEXT_MODIFIED] = g_signal_new ("text-modified",
                                             G_TYPE_FROM_CLASS (klass),
                                             G_SIGNAL_RUN_LAST,
                                             0,
                                             NULL,
                                             NULL,
                                             NULL,
                                             G_TYPE_NONE,
                                             0);

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-text-selector.ui");

  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, title_label);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, read_button);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, edit_button);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, delete_button);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, list_box);

  gtk_widget_class_bind_template_callback (widget_class, new_text_cb);
  gtk_widget_class_bind_template_callback (widget_class, read_text_cb);
  gtk_widget_class_bind_template_callback (widget_class, edit_text_cb);
  gtk_widget_class_bind_template_callback (widget_class, delete_text_cb);

  gtk_widget_class_bind_template_callback (widget_class, selection_changed_cb);
}

GtkWidget *
lr_text_selector_new ()
{
  return g_object_new (LR_TYPE_TEXT_SELECTOR, NULL);
}

void
lr_text_selector_set_database (LrTextSelector *self, LrDatabase *db)
{
  g_assert (LR_IS_TEXT_SELECTOR (self));

  self->db = db;
}

void
lr_text_selector_set_language (LrTextSelector *self, LrLanguage *next_language)
{
  g_assert (LR_IS_TEXT_SELECTOR (self));
  g_assert (self->db != NULL); /* We need a database connection first */

  gchar *title = g_strdup_printf ("%s texts", lr_language_get_name (next_language));
  gtk_label_set_text (GTK_LABEL (self->title_label), title);
  g_free (title);

  self->lang = next_language;
  self->selected_text = NULL;

  populate_text_list (self);

  /* GtkListBox does not emit a selection-changed signal when its contents change
   * through the model, therefore we call our callback to make sure the controls 
   * correctly represent the current selection (namely, that no row is selected when 
   * switching languages or at startup) */
  selection_changed_cb (GTK_LIST_BOX (self->list_box), self);
}

