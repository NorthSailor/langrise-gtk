#include "lr-text-selector.h"
#include "lr-text.h"

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

  LrText *selected_text; /* Currently selected text or NULL */
};

G_DEFINE_TYPE (LrTextSelector, lr_text_selector, GTK_TYPE_BOX)

static void
new_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("New text");
}

static void
read_text_cb (LrTextSelector *self, GtkWidget *button)
{
  /* Load the text */
  lr_database_load_text (self->db, self->selected_text);
  g_message ("Read text with title '%s'", lr_text_get_text (self->selected_text));
}

static void
edit_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("Edit text");
}

static void
delete_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("Delete text");
}

static GtkWidget *
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

static void
populate_text_list (LrTextSelector *self)
{
  lr_database_populate_texts (self->db, self->text_store, self->lang);
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

  /* Update the selected text */
  if (row == NULL)
    self->selected_text = NULL;
  else
    self->selected_text =
      g_list_model_get_item (G_LIST_MODEL (self->text_store), gtk_list_box_row_get_index (row));
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

  g_clear_object (&self->text_store);
}

static void
lr_text_selector_class_init (LrTextSelectorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_text_selector_finalize;

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

