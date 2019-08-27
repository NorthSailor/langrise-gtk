#include "lr-language-editor-dialog.h"
#include "language_presets.h"

struct _LrLanguageEditorDialog
{
  GtkDialog parent_instance;

  LrLanguage *language;
  gboolean is_editing;

  GtkWidget *name_entry;
  GtkWidget *code_entry;
  GtkWidget *regex_entry;

  GtkWidget *preset_box;
  GtkWidget *preset_listbox;

  /* 
   * AWFUL HACK:
   * When the user selects a preset, the text of the entries is changed.
   * To prevent the just selected entry of the listbox to be unselected,
   * the following variable tells the signal callback to ignore the signal 
   * if it is positive, and decrement it by one.
   */
  int skip_unselect_all;
};

enum
{
  PROP_0,
  PROP_LANGUAGE,
  PROP_IS_EDITING,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrLanguageEditorDialog, lr_language_editor_dialog, GTK_TYPE_DIALOG)

static void
preset_edited (LrLanguageEditorDialog *self, GtkWidget *entry)
{
  /* Clear the selection in the preset listbox since the entries' content
   * no longer corresponds to a preset. */
  if (self->skip_unselect_all)
    self->skip_unselect_all--;
  else
    gtk_list_box_unselect_all (GTK_LIST_BOX (self->preset_listbox));
}

static void
preset_picked (LrLanguageEditorDialog *self, GtkWidget *listbox)
{
  GtkListBoxRow *row = gtk_list_box_get_selected_row (GTK_LIST_BOX (listbox));
  if (row == NULL)
    return; /* Selection cleared */

  int index = gtk_list_box_row_get_index (row);

  lr_language_preset_t *preset = &presets[index];

  self->skip_unselect_all = 3;
  gtk_entry_set_text (GTK_ENTRY (self->name_entry), preset->name);
  gtk_entry_set_text (GTK_ENTRY (self->code_entry), preset->code);
  gtk_entry_set_text (GTK_ENTRY (self->regex_entry), preset->regex);
}

static void
dialog_response (GtkDialog *dialog, int response_id)
{
  LrLanguageEditorDialog *self = LR_LANGUAGE_EDITOR_DIALOG (dialog);

  if (response_id == GTK_RESPONSE_OK)
    {
      lr_language_set_name (self->language, gtk_entry_get_text (GTK_ENTRY (self->name_entry)));
      lr_language_set_code (self->language, gtk_entry_get_text (GTK_ENTRY (self->code_entry)));
      lr_language_set_word_regex (self->language,
                                  gtk_entry_get_text (GTK_ENTRY (self->regex_entry)));
    }
}

static void
lr_language_editor_dialog_init (LrLanguageEditorDialog *self)
{
  self->skip_unselect_all = 0;

  gtk_dialog_add_button (GTK_DIALOG (self), "OK", GTK_RESPONSE_OK);
  gtk_dialog_add_button (GTK_DIALOG (self), "Cancel", GTK_RESPONSE_CANCEL);
  gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_OK);

  GtkBuilder *builder =
    gtk_builder_new_from_resource ("/com/langrise/Langrise/lr-language-editor-dialog.ui");
  GtkWidget *root = GTK_WIDGET (gtk_builder_get_object (builder, "root"));

  GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (self));
  gtk_container_add (GTK_CONTAINER (box), root);

  self->name_entry = GTK_WIDGET (gtk_builder_get_object (builder, "name_entry"));
  self->code_entry = GTK_WIDGET (gtk_builder_get_object (builder, "code_entry"));
  self->regex_entry = GTK_WIDGET (gtk_builder_get_object (builder, "regex_entry"));

  self->preset_box = GTK_WIDGET (gtk_builder_get_object (builder, "preset_box"));
  self->preset_listbox = GTK_WIDGET (gtk_builder_get_object (builder, "preset_listbox"));

  g_signal_connect_swapped (self->name_entry, "changed", (GCallback)preset_edited, self);
  g_signal_connect_swapped (self->code_entry, "changed", (GCallback)preset_edited, self);
  g_signal_connect_swapped (self->regex_entry, "changed", (GCallback)preset_edited, self);

  g_signal_connect_swapped (
    self->preset_listbox, "selected-rows-changed", (GCallback)preset_picked, self);

  g_object_unref (builder);
}

static GtkWidget *
create_row_for_preset (lr_language_preset_t *preset)
{
  GtkWidget *row = gtk_list_box_row_new ();

  GtkWidget *name_label = gtk_label_new (preset->name);
  gtk_container_add (GTK_CONTAINER (row), name_label);

  PangoAttrList *attr_list = pango_attr_list_new ();

  pango_attr_list_insert (attr_list, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
  pango_attr_list_insert (attr_list, pango_attr_size_new (15 * PANGO_SCALE));
  gtk_label_set_attributes (GTK_LABEL (name_label), attr_list);

  pango_attr_list_unref (attr_list);

  gtk_widget_set_margin_top (name_label, 10);
  gtk_widget_set_margin_bottom (name_label, 10);

  gtk_widget_show_all (row);
  return row;
}

static void
lr_language_editor_dialog_constructed (GObject *object)
{
  LrLanguageEditorDialog *self = LR_LANGUAGE_EDITOR_DIALOG (object);
  g_assert (LR_IS_LANGUAGE (self->language));

  if (self->is_editing)
    {
      gchar *title = g_strdup_printf ("Edit language '%s'", lr_language_get_name (self->language));
      gtk_window_set_title (GTK_WINDOW (self), title);
      g_free (title);

      /* Load the language settings into the entries */
      gtk_entry_set_text (GTK_ENTRY (self->name_entry), lr_language_get_name (self->language));
      gtk_entry_set_text (GTK_ENTRY (self->code_entry), lr_language_get_code (self->language));
      gtk_entry_set_text (GTK_ENTRY (self->regex_entry),
                          lr_language_get_word_regex (self->language));

      /* Disable the presets and editing of the code or word regex */
      gtk_widget_set_sensitive (self->code_entry, FALSE);
      gtk_widget_set_sensitive (self->regex_entry, FALSE);
      gtk_widget_hide (self->preset_box);
    }
  else
    {
      gtk_window_set_title (GTK_WINDOW (self), "New language");
    }

  /* Load the presets into the listbox */
  int n_presets = sizeof (presets) / sizeof (lr_language_preset_t);
  for (int i = 0; i < n_presets; i++)
    {
      lr_language_preset_t preset = presets[i];
      gtk_container_add (GTK_CONTAINER (self->preset_listbox), create_row_for_preset (&preset));
    }

  G_OBJECT_CLASS (lr_language_editor_dialog_parent_class)->constructed (object);
}

static void
lr_language_editor_dialog_set_property (GObject *object,
                                        guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  LrLanguageEditorDialog *self = LR_LANGUAGE_EDITOR_DIALOG (object);

  switch (property_id)
    {
    case PROP_LANGUAGE:
      self->language = g_value_get_object (value);
      break;
    case PROP_IS_EDITING:
      self->is_editing = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_language_editor_dialog_class_init (LrLanguageEditorDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->constructed = lr_language_editor_dialog_constructed;
  object_class->set_property = lr_language_editor_dialog_set_property;

  obj_properties[PROP_LANGUAGE] = g_param_spec_object ("language",
                                                       "language",
                                                       "The language",
                                                       LR_TYPE_LANGUAGE,
                                                       G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  obj_properties[PROP_IS_EDITING] =
    g_param_spec_boolean ("is-editing",
                          "is-editing",
                          "Whether an existing language is being edited.",
                          FALSE,
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);
  dialog_class->response = dialog_response;
}

GtkWidget *
lr_language_editor_dialog_new (LrLanguage *language, gboolean is_editing)
{
  return g_object_new (LR_TYPE_LANGUAGE_EDITOR_DIALOG,
                       "language",
                       language,
                       "is-editing",
                       is_editing,
                       "use-header-bar",
                       TRUE,
                       NULL);
}

LrLanguage *
lr_language_editor_dialog_get_language (LrLanguageEditorDialog *self)
{
  return self->language;
}
