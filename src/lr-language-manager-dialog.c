#include "lr-language-manager-dialog.h"
#include "lr-language-editor-dialog.h"

struct _LrLanguageManagerDialog
{
  GtkDialog parent_instance;

  LrDatabase *db;
  GtkWidget *language_listbox;

  GtkWidget *new_button;
  GtkWidget *edit_button;
  GtkWidget *delete_button;

  LrLanguage *selected_language;

  GListStore *language_store;
};

enum
{
  PROP_0,
  PROP_DATABASE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrLanguageManagerDialog, lr_language_manager_dialog, GTK_TYPE_DIALOG)

static void
populate_languages (LrLanguageManagerDialog *self)
{
  lr_database_populate_languages (self->db, self->language_store);
}

static void
new_cb (LrLanguageManagerDialog *self, GtkWidget *button)
{
  LrLanguage *new_language = lr_language_new (0, "English", "en", "[a-zA-Z]+");
  GtkWidget *dialog = lr_language_editor_dialog_new (new_language, FALSE);

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (self));

  int response = gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);

  if (response == GTK_RESPONSE_OK)
    {
      lr_database_insert_language (self->db, new_language);
      populate_languages (self);
    }
  g_clear_object (&new_language);
}

static void
edit_cb (LrLanguageManagerDialog *self, GtkWidget *button)
{
  g_message ("edit_cb ()");
}

static void
delete_cb (LrLanguageManagerDialog *self, GtkWidget *button)
{
  g_message ("delete_cb ()");
}

static void
selection_changed_cb (LrLanguageManagerDialog *self, GtkWidget *list_box)
{
  g_assert (LR_IS_LANGUAGE_MANAGER_DIALOG (self));
  g_assert (GTK_IS_LIST_BOX (list_box));

  /* If no items are selected, disable the editing controls */
  GtkListBoxRow *row = gtk_list_box_get_selected_row (GTK_LIST_BOX (list_box));

  gtk_widget_set_sensitive (self->edit_button, row != NULL);
  gtk_widget_set_sensitive (self->delete_button, row != NULL);

  if (row != NULL)
    {
      int index = gtk_list_box_row_get_index (row);
      self->selected_language =
        LR_LANGUAGE (g_list_model_get_item (G_LIST_MODEL (self->language_store), index));
    }
  else
    {
      self->selected_language = NULL;
    }
}

static void
lr_language_manager_dialog_init (LrLanguageManagerDialog *self)
{
  gtk_window_set_title (GTK_WINDOW (self), "Languages");

  gtk_dialog_add_button (GTK_DIALOG (self), "Close", GTK_RESPONSE_CLOSE);
  gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_CLOSE);

  GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (self));

  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/langrise/Langrise/lr-language-manager-dialog.ui");
  GtkWidget *root_box = GTK_WIDGET (gtk_builder_get_object (builder, "box"));
  gtk_container_add (GTK_CONTAINER (box), root_box);

  self->language_listbox = GTK_WIDGET (gtk_builder_get_object (builder, "language_listbox"));

  self->new_button = GTK_WIDGET (gtk_builder_get_object (builder, "new_button"));
  self->edit_button = GTK_WIDGET (gtk_builder_get_object (builder, "edit_button"));
  self->delete_button = GTK_WIDGET (gtk_builder_get_object (builder, "delete_button"));

  g_signal_connect_swapped (self->new_button, "clicked", (GCallback)new_cb, self);
  g_signal_connect_swapped (self->edit_button, "clicked", (GCallback)edit_cb, self);
  g_signal_connect_swapped (self->delete_button, "clicked", (GCallback)delete_cb, self);

  g_signal_connect_swapped (
    self->language_listbox, "selected-rows-changed", (GCallback)selection_changed_cb, self);

  g_object_unref (builder);
}

static GtkWidget *
language_to_widget (LrLanguage *language, gpointer user_data)
{
  g_assert (LR_IS_LANGUAGE (language));

  GtkWidget *row = gtk_list_box_row_new ();

  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/langrise/Langrise/lr-language-manager-row.ui");

  GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "box"));
  GtkWidget *title_label = GTK_WIDGET (gtk_builder_get_object (builder, "title_label"));
  GtkWidget *code_label = GTK_WIDGET (gtk_builder_get_object (builder, "code_label"));

  gtk_container_add (GTK_CONTAINER (row), box);
  gtk_label_set_text (GTK_LABEL (title_label), lr_language_get_name (language));
  gtk_label_set_text (GTK_LABEL (code_label), lr_language_get_code (language));

  g_object_unref (builder);
  gtk_widget_show_all (row);

  return row;
}

static void
lr_language_manager_dialog_constructed (GObject *object)
{
  LrLanguageManagerDialog *self = LR_LANGUAGE_MANAGER_DIALOG (object);
  g_assert (LR_IS_DATABASE (self->db));

  self->language_store = g_list_store_new (LR_TYPE_LANGUAGE);
  gtk_list_box_bind_model (GTK_LIST_BOX (self->language_listbox),
                           G_LIST_MODEL (self->language_store),
                           (GtkListBoxCreateWidgetFunc)language_to_widget,
                           NULL,
                           NULL);
  populate_languages (self);

  selection_changed_cb (self, self->language_listbox);

  G_OBJECT_CLASS (lr_language_manager_dialog_parent_class)->constructed (object);
}

static void
lr_language_manager_dialog_finalize (GObject *object)
{
  LrLanguageManagerDialog *self = LR_LANGUAGE_MANAGER_DIALOG (object);

  g_clear_object (&self->language_store);

  G_OBJECT_CLASS (lr_language_manager_dialog_parent_class)->finalize (object);
}

static void
lr_language_manager_dialog_set_property (GObject *object,
                                         guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
  LrLanguageManagerDialog *self = LR_LANGUAGE_MANAGER_DIALOG (object);

  switch (property_id)
    {
    case PROP_DATABASE:
      self->db = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_language_manager_dialog_class_init (LrLanguageManagerDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_language_manager_dialog_finalize;
  object_class->set_property = lr_language_manager_dialog_set_property;
  object_class->constructed = lr_language_manager_dialog_constructed;

  obj_properties[PROP_DATABASE] = g_param_spec_object ("database",
                                                       "database",
                                                       "The database",
                                                       LR_TYPE_DATABASE,
                                                       G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

GtkWidget *
lr_language_manager_dialog_new (LrDatabase *database)
{
  return g_object_new (
    LR_TYPE_LANGUAGE_MANAGER_DIALOG, "database", database, "use-header-bar", TRUE, NULL);
}

