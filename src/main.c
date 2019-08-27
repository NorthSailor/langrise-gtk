#include <glib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include "lr-database.h"
#include "lr-main-window.h"

static void
init_css ()
{
  GdkScreen *screen = gdk_screen_get_default ();
  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (provider, "/com/langrise/Langrise/style.css");
  gtk_style_context_add_provider_for_screen (
    screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
activate_cb (GtkApplication *app, LrDatabase *db)
{
  GtkWidget *window = lr_main_window_new (app);
  lr_main_window_set_database (LR_MAIN_WINDOW (window), db);

  gtk_widget_show_all (window);
}

static void
create_database (const gchar *database_path)
{
  GBytes *schema_bytes = g_resources_lookup_data (
    "/com/langrise/Langrise/schema.sql", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
  const gchar *schema = g_bytes_get_data (schema_bytes, NULL);

  sqlite3 *db;

  g_assert (sqlite3_open_v2 (
              database_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) == SQLITE_OK);

  g_assert (sqlite3_exec (db, schema, NULL, NULL, NULL) == SQLITE_OK);

  sqlite3_close_v2 (db);

  g_bytes_unref (schema_bytes);
}

static gchar *
get_database_path ()
{
  /* Get the expected path for the database */
  gchar *langrise_config_dir =
    g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), "langrise", NULL);

  /* Create the config directory if it doesn't exist */
  g_mkdir_with_parents (langrise_config_dir, 0770);

  gchar *database_path = g_build_filename (langrise_config_dir, "langrise.db", NULL);
  g_free (langrise_config_dir);

  /* Does the database exist? */
  if (!g_file_test (database_path, G_FILE_TEST_EXISTS))
    {
      /* If it doesn't create a brand new one */
      create_database (database_path);
    }

  return database_path;
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  init_css ();

  gchar *db_path = get_database_path ();
  LrDatabase *db = lr_database_new (db_path);
  g_free (db_path);

  GtkApplication *application =
    gtk_application_new ("com.langrise.Langrise", G_APPLICATION_FLAGS_NONE);

  g_signal_connect (application, "activate", (GCallback)activate_cb, db);

  int status = g_application_run (G_APPLICATION (application), argc, argv);

  g_object_unref (application);

  g_clear_object (&db);

  return status;
}
