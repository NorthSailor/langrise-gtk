#include "lr-main-window.h"
#include "lr-database.h"
#include "lr-language-editor-dialog.h"
#include "lr-language-manager-dialog.h"
#include "lr-reader.h"
#include "lr-text-selector.h"
#include "lr-vocabulary-view.h"

struct _LrMainWindow
{
  GtkApplicationWindow parent_instance;

  LrDatabase *db;

  /* Widgets */
  GtkWidget *headerbar;
  GtkWidget *lang_menu_button;
  GtkWidget *lang_name_label;

  GtkWidget *back_button;

  GtkWidget *global_stack;
  GtkWidget *home_stack;
  GtkWidget *header_stack;
  GtkWidget *text_title_label;

  GtkWidget *home_switcher;

  GtkWidget *text_selector;
  GtkWidget *vocabulary_view;
  GtkWidget *reader;

  GMenu *lang_menu;

  /* The list of languages */
  GListStore *lang_store;
  LrLanguage *active_lang; /* Active language or NULL if none */

  /* State */
  /* id of the active language - Needed to select the correct language after a database repopulation */
  int lang_id;

  /* Set by populate_languages depending on the number of languages */
  gboolean language_menu_visible;

  /* Mode (home/reading) */
  int mode;
};

G_DEFINE_TYPE (LrMainWindow, lr_main_window, GTK_TYPE_APPLICATION_WINDOW)

enum
{
  PROP_0,
  PROP_DATABASE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

enum
{
  MODE_HOME = 1,
  MODE_READING
};

static void
switch_to_mode (LrMainWindow *self, guint mode)
{
  /* Switcher enabled? */
  switch (mode)
    {
    case MODE_READING:
      {
        gtk_stack_set_visible_child_full (
          GTK_STACK (self->global_stack), "reading", GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
        gtk_stack_set_visible_child_name (GTK_STACK (self->header_stack), "reading_header");
        gtk_widget_hide (self->lang_menu_button);
        gtk_widget_show (self->back_button);
      }
      break;
    case MODE_HOME:
      {
        gtk_stack_set_visible_child_full (
          GTK_STACK (self->global_stack), "home", GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
        gtk_stack_set_visible_child_name (GTK_STACK (self->header_stack), "home_header");
        gtk_widget_set_visible (self->lang_menu_button, self->language_menu_visible);
        gtk_widget_set_sensitive (self->home_switcher, TRUE);
        gtk_widget_hide (self->back_button);
      }
      break;
    }
  self->mode = mode;
}

/*
 * Switches to a new language.
 * @id - The ID of the new language, or -1 to switch to the start screen.
 */
static void
switch_to_language (LrMainWindow *self)
{
  int lang_id = self->lang_id;

  /* Free the previous active language, if any */
  g_clear_object (&self->active_lang);

  int n_languages = g_list_model_get_n_items (G_LIST_MODEL (self->lang_store));
  if (n_languages == 0)
    {
      /* No languages mode */
      gtk_widget_set_sensitive (self->home_switcher, FALSE);
      gtk_stack_set_visible_child_name (GTK_STACK (self->global_stack), "no_languages");
      gtk_widget_hide (self->lang_menu_button); /* Should never be visible without any languages */
      self->active_lang = NULL;
      self->lang_id = -1;
    }
  else
    {
      gtk_widget_set_sensitive (self->home_switcher, TRUE);

      /* Find the new language, first one by default */
      LrLanguage *next_lang = g_list_model_get_item (G_LIST_MODEL (self->lang_store), 0);
      for (int i = 0; i < n_languages; ++i)
        {
          LrLanguage *lang = g_list_model_get_item (G_LIST_MODEL (self->lang_store), i);
          if (lr_language_get_id (lang) == lang_id)
            {
              next_lang = lang;
              break;
            }
          else
            g_object_unref (lang);
        }

      g_assert (next_lang != NULL); /* This should not be possible */

      gtk_label_set_text (GTK_LABEL (self->lang_name_label), lr_language_get_name (next_lang));

      lr_text_selector_set_language (LR_TEXT_SELECTOR (self->text_selector), next_lang);
      lr_vocabulary_view_set_language (LR_VOCABULARY_VIEW (self->vocabulary_view), next_lang);

      /* If we actually changed language, switch back to home mode */
      /* We switch back to home mode if we are already in home mode so that 
       * the language menu visibility will be updated.
       */
      if ((lang_id != lr_language_get_id (next_lang)) || (self->mode == MODE_HOME))
        switch_to_mode (self, MODE_HOME);

      self->active_lang = next_lang;
      self->lang_id = lr_language_get_id (next_lang);
    }
}

static void
populate_languages (LrMainWindow *self)
{
  lr_database_populate_languages (self->db, self->lang_store);

  /* Should the button be visible? */
  int n_languages = g_list_model_get_n_items (G_LIST_MODEL (self->lang_store));
  self->language_menu_visible = n_languages > 1;

  /* Create the menu model */
  g_menu_remove_all (self->lang_menu);

  GMenu *lang_menu = g_menu_new ();

  for (int i = 0; i < n_languages; ++i)
    {
      LrLanguage *lang = g_list_model_get_item (G_LIST_MODEL (self->lang_store), i);
      gchar *detailed_action_name =
        g_strdup_printf ("win.switchlanguage(%d)", lr_language_get_id (lang));
      g_menu_append (lang_menu, lr_language_get_name (lang), detailed_action_name);
      g_free (detailed_action_name);
      g_object_unref (lang);
    }

  GMenuItem *lang_menu_header =
    g_menu_item_new_section ("Switch language", G_MENU_MODEL (lang_menu));
  g_menu_append_item (self->lang_menu, lang_menu_header);

  /* Switch to the proper language */
  switch_to_language (self);
}


/* Actions */
static void
switch_language_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  int id = g_variant_get_int32 (parameter);

  LrMainWindow *self = LR_MAIN_WINDOW (user_data);

  /* No need to switch if the same language is selected */
  if ((self->active_lang == NULL) || (id != self->lang_id))
    {
      self->lang_id = id;
      switch_to_language (self);
    }
}

static GActionEntry win_entries[] = {
  { "switchlanguage", switch_language_activated, "i", NULL, NULL }
};

static void
go_back_cb (LrMainWindow *self, GtkButton *button)
{
  switch_to_mode (self, MODE_HOME);
}

static void
read_text_cb (LrTextSelector *selector, LrText *text, LrMainWindow *self)
{
  g_assert (LR_IS_TEXT_SELECTOR (selector));
  g_assert (LR_IS_TEXT (text));
  g_assert (LR_IS_MAIN_WINDOW (self));

  lr_database_load_text (self->db, text);

  lr_reader_set_text (LR_READER (self->reader), text, self->db);

  gtk_label_set_text (GTK_LABEL (self->text_title_label), lr_text_get_title (text));
  switch_to_mode (self, MODE_READING);
}

static void
about_cb (LrMainWindow *self, GtkWidget *button)
{
  static const gchar *authors[] = { "Jason Barmparesos", NULL };

  /* Load the license file. */
  GBytes *bytes = g_resources_lookup_data (
    "/com/langrise/Langrise/license.txt", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

  gtk_show_about_dialog (GTK_WINDOW (self),
                         "program-name",
                         "Langrise",
                         "version",
                         "0.1 pre-alpha",
                         "comments",
                         "Expanding L2 vocabulary in context.",
                         "copyright",
                         "\u00A9 Jason Barmparesos 2019. All rights reserved.\n"
                         "Licensed under the \"Fair Source 1, version 0.9\" license.",
                         "license",
                         g_bytes_get_data (bytes, NULL),
                         "license-type",
                         GTK_LICENSE_CUSTOM,
                         "wrap-license",
                         TRUE,
                         "logo",
                         gdk_pixbuf_new_from_resource_at_scale (
                           "/com/langrise/Langrise/logo256.png", 256, 256, FALSE, NULL),
                         "authors",
                         authors,
                         NULL);
  g_bytes_unref (bytes);
}

static void
manage_languages_cb (LrMainWindow *self, GtkWidget *button)
{
  g_assert (LR_IS_MAIN_WINDOW (self));
  g_assert (GTK_IS_BUTTON (button));

  GtkWidget *manager_dialog = lr_language_manager_dialog_new (self->db);

  gtk_window_set_transient_for (GTK_WINDOW (manager_dialog), GTK_WINDOW (self));

  gtk_dialog_run (GTK_DIALOG (manager_dialog));

  gtk_widget_destroy (manager_dialog);

  populate_languages (self);
}

static void
quit_cb (GtkWidget *button, LrMainWindow *self)
{
  gtk_widget_destroy (GTK_WIDGET (self));
}

static void
add_language_cb (LrMainWindow *self, GtkWidget *button)
{
  g_assert (LR_IS_MAIN_WINDOW (self));
  g_assert (GTK_IS_BUTTON (button));

  LrLanguage *new_language = lr_language_new (0, "English", "en", "[a-zA-Z]+", ". ");
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
text_modified_cb (LrTextSelector *selector, LrMainWindow *self)
{
  g_assert (LR_IS_TEXT_SELECTOR (selector));
  g_assert (LR_IS_MAIN_WINDOW (self));
  lr_vocabulary_view_set_language (LR_VOCABULARY_VIEW (self->vocabulary_view), self->active_lang);
}

static void
lr_main_window_constructed (GObject *obj)
{
  LrMainWindow *self = LR_MAIN_WINDOW (obj);
  self->lang_menu = g_menu_new ();
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->lang_menu_button),
                                  G_MENU_MODEL (self->lang_menu));

  g_action_map_add_action_entries (
    G_ACTION_MAP (obj), win_entries, G_N_ELEMENTS (win_entries), obj);

  switch_to_mode (self, MODE_HOME);
  switch_to_language (self);

  /* Add the text selector to the stack view */
  self->text_selector = lr_text_selector_new ();
  lr_text_selector_set_database (LR_TEXT_SELECTOR (self->text_selector), self->db);
  gtk_stack_add_titled (GTK_STACK (self->home_stack), self->text_selector, "texts", "Texts");

  /* Add the vocabulary view to the stack view */
  self->vocabulary_view = lr_vocabulary_view_new (self->db);
  gtk_stack_add_titled (
    GTK_STACK (self->home_stack), self->vocabulary_view, "vocabulary", "Vocabulary");

  /* Add the reader to the global stack view */
  self->reader = lr_reader_new ();
  gtk_stack_add_named (GTK_STACK (self->global_stack), self->reader, "reading");

  /* Connect the "read-text" signal from the selector */
  g_signal_connect (self->text_selector, "read-text", (GCallback)read_text_cb, self);
  g_signal_connect (self->text_selector, "text-modified", (GCallback)text_modified_cb, self);

  /* Populate the language menu */
  populate_languages (self);

  G_OBJECT_CLASS (lr_main_window_parent_class)->constructed (obj);
}

static void
lr_main_window_finalize (GObject *obj)
{
  LrMainWindow *self = LR_MAIN_WINDOW (obj);

  g_clear_object (&self->active_lang);

  /* Free the language list */
  g_clear_object (&self->lang_store);
  G_OBJECT_CLASS (lr_main_window_parent_class)->finalize (obj);
}

static void
lr_main_window_init (LrMainWindow *window)
{
  window->active_lang = NULL;
  window->lang_id = -1;

  window->lang_store = g_list_store_new (LR_TYPE_LANGUAGE);

  gtk_widget_init_template (GTK_WIDGET (window));
}

static void
lr_main_window_set_property (GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  LrMainWindow *self = LR_MAIN_WINDOW (object);

  switch (property_id)
    {
    case PROP_DATABASE:
      lr_main_window_set_database (self, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_main_window_class_init (LrMainWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->constructed = lr_main_window_constructed;
  object_class->finalize = lr_main_window_finalize;
  object_class->set_property = lr_main_window_set_property;

  obj_properties[PROP_DATABASE] = g_param_spec_object ("database",
                                                       "Database",
                                                       "The database",
                                                       LR_TYPE_DATABASE,
                                                       G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-main-window.ui");

  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, lang_name_label);
  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, lang_menu_button);
  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, back_button);

  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, global_stack);
  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, home_stack);
  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, header_stack);
  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, home_switcher);
  gtk_widget_class_bind_template_child (widget_class, LrMainWindow, text_title_label);

  gtk_widget_class_bind_template_callback (widget_class, about_cb);
  gtk_widget_class_bind_template_callback (widget_class, manage_languages_cb);
  gtk_widget_class_bind_template_callback (widget_class, go_back_cb);
  gtk_widget_class_bind_template_callback (widget_class, quit_cb);
  gtk_widget_class_bind_template_callback (widget_class, add_language_cb);
}

GtkWidget *
lr_main_window_new (GtkApplication *application, LrDatabase *db)
{
  g_return_val_if_fail (application != NULL, NULL);

  GtkWidget *window =
    g_object_new (LR_TYPE_MAIN_WINDOW, "database", db, "application", application, NULL);
  return window;
}

void
lr_main_window_set_database (LrMainWindow *self, LrDatabase *db)
{
  g_return_if_fail (LR_IS_MAIN_WINDOW (self));
  g_return_if_fail (LR_IS_DATABASE (db));

  self->db = db;
}

LrDatabase *
lr_main_window_get_database (LrMainWindow *self)
{
  g_return_val_if_fail (LR_IS_MAIN_WINDOW (self), NULL);

  return self->db;
}
