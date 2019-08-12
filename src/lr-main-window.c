#include "lr-main-window.h"
#include "lr-database.h"
#include "lr-text-selector.h"

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

  GtkWidget *home_switcher;

  GtkWidget *text_selector;

  GMenu *lang_menu;

  /* The list of languages */
  GListStore *lang_store;
  LrLanguage *active_lang; /* Active language or NULL if none */

  /* State */
  /* id of the active language - Needed to select the correct language after a database repopulation */
  int lang_id;
};

G_DEFINE_TYPE (LrMainWindow, lr_main_window, GTK_TYPE_APPLICATION_WINDOW)

enum
{
  MODE_HOME = 1,
  MODE_READING
};

/*
 * Switches to a new language.
 * @id - The ID of the new language, or -1 to switch to the start screen.
 */
static void
switch_to_language (LrMainWindow *self, int lang_id)
{
  if (lang_id == -1)
    {
      /* No languages mode */
      gtk_widget_set_sensitive (self->home_switcher, FALSE);
      gtk_stack_set_visible_child_name (GTK_STACK (self->global_stack), "no_languages");
      self->active_lang = NULL;
      self->lang_id = -1;
    }
  else
    {
      gtk_widget_set_sensitive (self->home_switcher, TRUE);
      gtk_stack_set_visible_child_name (GTK_STACK (self->global_stack), "home");

      /* Find the new language */
      LrLanguage *next_lang = NULL;
      int n_languages = g_list_model_get_n_items (G_LIST_MODEL (self->lang_store));
      for (int i = 0; i < n_languages; ++i)
        {
          LrLanguage *lang = g_list_model_get_item (G_LIST_MODEL (self->lang_store), i);
          if (lr_language_get_id (lang) == lang_id)
            {
              next_lang = lang;
              break;
            }
          g_object_unref (lang);
        }

      g_assert (next_lang != NULL); /* This would mean an invalid ID somehow */

      gtk_label_set_text (GTK_LABEL (self->lang_name_label), lr_language_get_name (next_lang));

      /* TODO Alert the active subview */

      lr_text_selector_set_language (LR_TEXT_SELECTOR (self->text_selector), next_lang);

      self->active_lang = next_lang;
      self->lang_id = lang_id;
    }
}

static void
populate_language_menu (LrMainWindow *self)
{
  /* Should the button be visible? */
  int n_languages = g_list_model_get_n_items (G_LIST_MODEL (self->lang_store));
  gtk_widget_set_visible (self->lang_menu_button, n_languages > 1);

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
}


/* Actions */
static void
switch_language_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  int id = g_variant_get_int32 (parameter);

  LrMainWindow *self = LR_MAIN_WINDOW (user_data);

  /* No need to switch if the same language is selected */
  if ((self->active_lang == NULL) || (id != self->lang_id))
    switch_to_language (self, id);
}

static GActionEntry win_entries[] = {
  { "switchlanguage", switch_language_activated, "i", NULL, NULL }
};

static void
switch_to_mode (LrMainWindow *self, guint mode)
{
  /* Switcher enabled? */
  switch (mode)
    {
    case MODE_READING:
      {
        gtk_stack_set_visible_child_name (GTK_STACK (self->global_stack), "reading");
        gtk_stack_set_visible_child_name (GTK_STACK (self->header_stack), "reading_header");
        gtk_widget_hide (self->lang_menu_button);
        gtk_widget_show (self->back_button);
      }
      break;
    case MODE_HOME:
      {
        gtk_stack_set_visible_child_name (GTK_STACK (self->global_stack), "home");
        gtk_stack_set_visible_child_name (GTK_STACK (self->header_stack), "home_header");
        gtk_widget_show (self->lang_menu_button);
        gtk_widget_set_sensitive (self->home_switcher, TRUE);
        gtk_widget_hide (self->back_button);
      }
      break;
    }
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
quit_cb (GtkWidget *button, LrMainWindow *self)
{
  gtk_widget_destroy (GTK_WIDGET (self));
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
  switch_to_language (self, -1);

  /* Add the text selector to the stack view */
  self->text_selector = lr_text_selector_new ();
  gtk_stack_add_titled (GTK_STACK (self->home_stack), self->text_selector, "texts", "Texts");

  G_OBJECT_CLASS (lr_main_window_parent_class)->constructed (obj);
}

static void
lr_main_window_finalize (GObject *obj)
{
  LrMainWindow *self = LR_MAIN_WINDOW (obj);

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
lr_main_window_class_init (LrMainWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->constructed = lr_main_window_constructed;
  object_class->finalize = lr_main_window_finalize;

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

  gtk_widget_class_bind_template_callback (widget_class, about_cb);
  gtk_widget_class_bind_template_callback (widget_class, quit_cb);
}

GtkWidget *
lr_main_window_new (GtkApplication *application)
{
  g_return_val_if_fail (application != NULL, NULL);

  GtkWidget *window = g_object_new (LR_TYPE_MAIN_WINDOW, "application", application, NULL);
  return window;
}

void
lr_main_window_set_database (LrMainWindow *self, LrDatabase *db)
{
  g_return_if_fail (LR_IS_MAIN_WINDOW (self));
  g_return_if_fail (db != NULL);

  self->db = db;

  /* Free the existing list */
  lr_database_populate_languages (db, self->lang_store);

  populate_language_menu (self);

  /* Alert the subviews */
  lr_text_selector_set_database (LR_TEXT_SELECTOR (self->text_selector), self->db);

  /* Select a language if we have any */
  if (g_list_model_get_n_items (G_LIST_MODEL (self->lang_store)) > 0)
    {
      if (self->lang_id == -1)
        {
          /* There was no language selected. Pick the first one */
          LrLanguage *first = g_list_model_get_item (G_LIST_MODEL (self->lang_store), 0);
          switch_to_language (self, lr_language_get_id (first));
          g_object_unref (first);
        }
      else
        {
          /* TODO Find the language with that ID */
        }
    }
  else
    {
      /* No languages in the database */
      switch_to_language (self, -1);
    }
}

LrDatabase *
lr_main_window_get_database (LrMainWindow *self)
{
  g_return_val_if_fail (LR_IS_MAIN_WINDOW (self), NULL);

  return self->db;
}
