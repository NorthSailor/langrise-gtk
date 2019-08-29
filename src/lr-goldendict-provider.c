#include "lr-goldendict-provider.h"
#include "lr-dictionary-provider.h"
#include "common.h"

struct _LrGoldendictProvider
{
  GtkBox parent_instance;

  GtkWidget *status_label;
  GtkWidget *file_chooser;

  gchar *goldendict_path;
};

static void lr_goldendict_provider_interface_init (LrDictionaryProviderInterface *iface);

G_DEFINE_TYPE_WITH_CODE (LrGoldendictProvider,
                         lr_goldendict_provider,
                         GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (LR_TYPE_DICTIONARY_PROVIDER,
                                                lr_goldendict_provider_interface_init))


static gchar *
get_config_file_path ()
{
  return g_build_path (
    G_DIR_SEPARATOR_S, g_get_user_data_dir (), CONFIG_DIR_NAME, "goldendict.ini", NULL);
}

static void
file_set_cb (LrGoldendictProvider *self, GtkWidget *button)
{
  gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));
  self->goldendict_path = path;
  gtk_label_set_markup (GTK_LABEL (self->status_label), "Custom GoldenDict path set");

  /* Write the path in the INI file */
  GKeyFile *key_file = g_key_file_new ();

  g_key_file_set_string (key_file, "goldendict", "path", self->goldendict_path);

  gchar *config_path = get_config_file_path ();
  g_key_file_save_to_file (key_file, config_path, NULL);
  g_free (config_path);

  g_key_file_free (key_file);
}

gchar *
path_from_config ()
{
  gchar *path = get_config_file_path ();
  GKeyFile *key_file = g_key_file_new ();

  gchar *goldendict_path = NULL;

  if (g_key_file_load_from_file (key_file, path, G_KEY_FILE_NONE, NULL))
    {
      /* Properly formatted INI file found */
      /* We don't need to do any error checking since it will just return NULL
       * if the key is not found, which is what this function should
       * do anyway.
       */
      goldendict_path = g_key_file_get_string (key_file, "goldendict", "path", NULL);
    }

  g_key_file_free (key_file);
  g_free (path);
  return goldendict_path;
}

static void
set_goldendict_path (LrGoldendictProvider *self)
{
  if (!self->goldendict_path)
    {
      gchar *path = path_from_config ();

      /* If a path was read from the config file, check if the file actually exists */
      if (path && (g_file_test (path, G_FILE_TEST_EXISTS)))
        goto set_path;

      g_free (path);

      /* Couldn't find a valid file from the config file, search in PATH */
      path = g_find_program_in_path ("goldendict");
      if (path == NULL)
        {
          /* Goldendict was not found */
          gtk_label_set_markup (GTK_LABEL (self->status_label),
                                "Failed to find GoldenDict in PATH.\nSelect the executable below");
          return;
        }

    set_path:
      self->goldendict_path = path;
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (self->file_chooser), path);
      gtk_label_set_markup (GTK_LABEL (self->status_label), "Found GoldenDict.\nYou are all set!");
    }
}

static void
goldendict_lookup (LrDictionaryProvider *dictionary_provider,
                   const gchar *term,
                   gboolean finished_editing)
{
  LrGoldendictProvider *self = LR_GOLDENDICT_PROVIDER (dictionary_provider);
  set_goldendict_path (self);

  /* Spawn goldendict asynchronously and forget about it */
  const gchar *const argv[] = { self->goldendict_path, term, NULL };

  if (!g_spawn_async (NULL,
                      (gchar **)argv,
                      NULL,
                      G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_STDOUT_TO_DEV_NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL))
    {
      g_error ("Failed to spawn goldendict!");
    }
}


static void
lr_goldendict_provider_interface_init (LrDictionaryProviderInterface *iface)
{
  iface->lookup = goldendict_lookup;
}

static void
lr_goldendict_provider_init (LrGoldendictProvider *self)
{
  self->goldendict_path = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  set_goldendict_path (self);
}

static void
lr_goldendict_provider_finalize (GObject *object)
{
  LrGoldendictProvider *self = LR_GOLDENDICT_PROVIDER (object);

  g_free (self->goldendict_path);

  G_OBJECT_CLASS (lr_goldendict_provider_parent_class)->finalize (object);
}

static void
lr_goldendict_provider_class_init (LrGoldendictProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_goldendict_provider_finalize;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-goldendict-provider.ui");

  gtk_widget_class_bind_template_child (widget_class, LrGoldendictProvider, status_label);
  gtk_widget_class_bind_template_child (widget_class, LrGoldendictProvider, file_chooser);

  gtk_widget_class_bind_template_callback (widget_class, file_set_cb);
}

GtkWidget *
lr_goldendict_provider_new ()
{
  return g_object_new (LR_TYPE_GOLDENDICT_PROVIDER, NULL);
}
