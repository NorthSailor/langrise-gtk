#include "lr-main-window.h"
#include "lr-database.h"
#include "lr-text-selector.h"

struct _LrMainWindow
{
  GtkApplicationWindow parent_instance;

  lr_database_t *db;

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
  GList *lang_list;
  lr_language_t *active_lang; /* Active language or NULL if none */

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
      lr_language_t *next_lang = NULL;
      for (GList *l = self->lang_list; l != NULL; l = l->next)
        {
          lr_language_t *lang = (lr_language_t *)l->data;
          if (lang->id == lang_id)
            {
              next_lang = lang;
              break;
            }
        }

      g_assert (next_lang != NULL); /* This would mean an invalid ID somehow */

      gtk_label_set_text (GTK_LABEL (self->lang_name_label), next_lang->name);

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
  int n_languages = g_list_length (self->lang_list);
  gtk_widget_set_visible (self->lang_menu_button, n_languages > 1);

  /* Create the menu model */
  g_menu_remove_all (self->lang_menu);

  GMenu *lang_menu = g_menu_new ();

  for (GList *l = self->lang_list; l != NULL; l = l->next)
    {
      lr_language_t *lang = (lr_language_t *)l->data;
      gchar *detailed_action_name = g_strdup_printf ("win.switchlanguage(%d)", lang->id);
      g_menu_append (lang_menu, lang->name, detailed_action_name);
      g_free (detailed_action_name);
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
  g_list_free_full (self->lang_list, (GDestroyNotify)lr_database_language_free);
  G_OBJECT_CLASS (lr_main_window_parent_class)->finalize (obj);
}

static void
lr_main_window_init (LrMainWindow *window)
{
  window->active_lang = NULL;
  window->lang_id = -1;

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
lr_main_window_set_database (LrMainWindow *self, lr_database_t *db)
{
  g_return_if_fail (LR_IS_MAIN_WINDOW (self));
  g_return_if_fail (db != NULL);

  self->db = db;

  /* Free the existing list */
  g_list_free_full (self->lang_list, (GDestroyNotify)lr_database_language_free);

  self->lang_list = lr_database_get_languages (db);

  populate_language_menu (self);

  /* Alert the subviews */
  lr_text_selector_set_database (LR_TEXT_SELECTOR (self->text_selector), self->db);

  /* Select a language if we have any */
  if (self->lang_list != NULL)
    {
      if (self->lang_id == -1)
        {
          /* There was no language selected. Pick the first one */
          lr_language_t *first = (lr_language_t *)self->lang_list->data;
          switch_to_language (self, first->id);
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

lr_database_t *
lr_main_window_get_database (LrMainWindow *self)
{
  g_return_val_if_fail (LR_IS_MAIN_WINDOW (self), NULL);

  return self->db;
}
