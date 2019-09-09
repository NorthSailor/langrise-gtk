#include "lr-db-lemmatizer.h"
#include "lr-splitter.h"
#include "lr-lemma-suggestion.h"
#include <sqlite3.h>

/*
 * NOTE:
 *
 * The database lemmatizer assumes a SQL table in the database with
 * the following schema:
 *
 * CREATE TABLE "Forms" (
 *     Form TEXT COLLATE NOCASE,
 *     Lemma TEXT
 * );
 *
 * For optimal performance, the lemmatizer database should contain
 * a proper index as well.
 */

struct _LrDbLemmatizer
{
  LrLemmatizer parent_instance;

  gchar *path;

  sqlite3 *db;

  sqlite3_stmt *query;
};

enum
{
  PROP_0,
  PROP_PATH,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrDbLemmatizer, lr_db_lemmatizer, LR_TYPE_LEMMATIZER)

static void
lr_db_lemmatizer_init (LrDbLemmatizer *self)
{
}

static void
lr_db_lemmatizer_constructed (GObject *object)
{
  LrDbLemmatizer *self = LR_DB_LEMMATIZER (object);

  self->db = NULL;
  const gchar *const *data_dirs = g_get_system_data_dirs ();
  for (const gchar *const *str = &data_dirs[0]; *str != NULL; str++)
    {
      gchar *filename = g_strdup_printf ("%s.lemma", self->path);
      gchar *path = g_build_filename (*str, "langrise", "lemma_dbs", filename, NULL);
      g_free (filename);

      int rc = sqlite3_open_v2 (path, &self->db, SQLITE_OPEN_READWRITE, NULL);
      g_free (path);
      if (rc != SQLITE_OK)
        {
          sqlite3_close (self->db);
          self->db = NULL; /* This signifies that we didn't open the database */
        }
      else
        {
          break; /* We found and were able to open a database */
        }
    }

  if (!self->db)
    return;

  g_assert (sqlite3_prepare_v2 (
              self->db, "SELECT Lemma FROM Forms WHERE Form = ?;", -1, &self->query, NULL) ==
            SQLITE_OK);
}

static void
lr_db_lemmatizer_finalize (GObject *object)
{
  LrDbLemmatizer *self = LR_DB_LEMMATIZER (object);

  sqlite3_finalize (self->query);
  sqlite3_close (self->db);

  G_OBJECT_CLASS (lr_db_lemmatizer_parent_class)->finalize (object);
}

static void
lr_db_lemmatizer_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  LrDbLemmatizer *self = LR_DB_LEMMATIZER (object);

  switch (property_id)
    {
    case PROP_PATH:
      g_free (self->path);
      self->path = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_db_lemmatizer_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  LrDbLemmatizer *self = LR_DB_LEMMATIZER (object);

  switch (property_id)
    {
    case PROP_PATH:
      g_value_set_string (value, self->path);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static gchar *
db_lemmatizer_populate_suggestions (LrLemmatizer *base,
                                    GListStore *store,
                                    const char *text,
                                    GList *selection)
{
  LrDbLemmatizer *self = LR_DB_LEMMATIZER (base);

  if (self->db == NULL)
    {
      return g_strdup ("No lemma suggestions\nfor this language.");
    }

  int selected_words = g_list_length (selection);
  if (selected_words > 1)
    {
      return g_strdup ("This lemmatizer only works with single words");
    }
  else if (selected_words == 0)
    {
      return g_strdup ("No words selected");
    }

  const lr_range_t *range = selection->data;
  gchar *word = g_strndup (text + range->start, (range->end - range->start));

  sqlite3_reset (self->query);
  sqlite3_bind_text (self->query, 1, word, -1, NULL);

  while (sqlite3_step (self->query) == SQLITE_ROW)
    {
      const gchar *lexeme = (const gchar *)sqlite3_column_text (self->query, 0);

      LrLemmaSuggestion *suggestion = lr_lemma_suggestion_new (lexeme, "No Notes");
      g_list_store_append (store, suggestion);
    }

  g_free (word);
  int n_items = g_list_model_get_n_items (G_LIST_MODEL (store));
  return g_strdup_printf ("%d possible %s", n_items, n_items == 1 ? "lemma" : "lemmas");
}

static void
lr_db_lemmatizer_class_init (LrDbLemmatizerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_db_lemmatizer_finalize;
  object_class->constructed = lr_db_lemmatizer_constructed;
  object_class->set_property = lr_db_lemmatizer_set_property;
  object_class->get_property = lr_db_lemmatizer_get_property;

  /* Add additional properties to obj_properties here */
  obj_properties[PROP_PATH] = g_param_spec_string (
    "path", "Path", "The path to the SQLite file.", "", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

  LrLemmatizerClass *lemmatizer_class = LR_LEMMATIZER_CLASS (klass);
  lemmatizer_class->populate_suggestions = db_lemmatizer_populate_suggestions;
}

LrLemmatizer *
lr_db_lemmatizer_new (const gchar *path)
{
  return g_object_new (LR_TYPE_DB_LEMMATIZER, "path", path, NULL);
}
