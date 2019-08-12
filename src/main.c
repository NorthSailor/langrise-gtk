#include <glib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include <webkit2/webkit2.h>
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

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  init_css ();

  /* TODO: Find the database file in a more robust way. */
  LrDatabase *db = lr_database_new ("langrise.db");

  GtkApplication *application =
    gtk_application_new ("com.langrise.Langrise", G_APPLICATION_FLAGS_NONE);

  g_signal_connect (application, "activate", (GCallback)activate_cb, db);

  int status = g_application_run (G_APPLICATION (application), argc, argv);

  g_object_unref (application);

  g_clear_object (&db);

  return status;
}
