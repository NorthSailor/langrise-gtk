#include "lr-database.h"
#include <stdio.h>
#include <sqlite3.h>

struct _LrDatabase
{
  GObject parent_instance;
  sqlite3 *db;

  gchar *db_path;

  /* Get all languages */
  sqlite3_stmt *lang_stmt;

  /* Get all texts in a language */
  sqlite3_stmt *text_by_lang_stmt;
};

enum
{
  PROP_PATH = 1,
};

G_DEFINE_TYPE (LrDatabase, lr_database, G_TYPE_OBJECT)

static void
prepare_sql_statements (LrDatabase *db)
{
  g_assert (sqlite3_prepare_v2 (
              db->db, "SELECT ID, Code, Name FROM Languages;", -1, &db->lang_stmt, NULL) ==
            SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "SELECT ID, Title, Tags FROM Texts WHERE LanguageID = ?;",
                                -1,
                                &db->text_by_lang_stmt,
                                NULL) == SQLITE_OK);
}

static void
free_sql_statements (LrDatabase *db)
{
  sqlite3_finalize (db->lang_stmt);
  sqlite3_finalize (db->text_by_lang_stmt);
}

static void
open_database (LrDatabase *self)
{
  int rc = sqlite3_open_v2 (self->db_path, &self->db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
    {
      g_critical ("Failed to open the database at '%s'. SQLite Error: '%s'",
                  self->db_path,
                  sqlite3_errmsg (self->db));
      sqlite3_close (self->db);
    }

  prepare_sql_statements (self);
}

static void
lr_database_init (LrDatabase *self)
{
  self->db = NULL;
  self->db_path = NULL;
}

static void
lr_database_finalize (GObject *obj)
{
  LrDatabase *self = LR_DATABASE (obj);

  free_sql_statements (self);
  sqlite3_close (self->db);

  g_free (self->db_path);

  G_OBJECT_CLASS (lr_database_parent_class)->finalize (obj);
}

static void
lr_database_set_property (GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  LrDatabase *self = LR_DATABASE (obj);
  switch (prop_id)
    {
    case PROP_PATH:
      self->db_path = g_value_dup_string (value);
      open_database (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
lr_database_class_init (LrDatabaseClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_database_finalize;
  object_class->set_property = lr_database_set_property;

  g_object_class_install_property (object_class,
                                   PROP_PATH,
                                   g_param_spec_string ("path",
                                                        "Path",
                                                        "The path to the SQLite database.",
                                                        "",
                                                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));
}

LrDatabase *
lr_database_new (gchar *path)
{
  return g_object_new (LR_TYPE_DATABASE, "path", path, NULL);
}

GList *
lr_database_get_languages (LrDatabase *db)
{
  g_return_val_if_fail (db != NULL, NULL);

  GList *lang_list = NULL;

  sqlite3_reset (db->lang_stmt);

  while (sqlite3_step (db->lang_stmt) == SQLITE_ROW)
    {
      lr_language_t *lang = malloc (sizeof (lr_language_t));
      lang->id = sqlite3_column_int (db->lang_stmt, 0);

      lang->code = g_strdup ((gchar *)sqlite3_column_text (db->lang_stmt, 1));
      lang->name = g_strdup ((gchar *)sqlite3_column_text (db->lang_stmt, 2));

      lang_list = g_list_append (lang_list, lang);
    }

  return lang_list;
}

void
lr_database_language_free (lr_language_t *lang)
{
  g_return_if_fail (lang != NULL);

  g_free (lang->code);
  g_free (lang->name);
}

GList *
lr_database_get_texts (LrDatabase *db, int lang_id)
{
  g_assert (db != NULL);

  GList *text_list = NULL;

  sqlite3_stmt *stmt = db->text_by_lang_stmt;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lang_id);

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      lr_text_t *text = malloc (sizeof (lr_text_t));
      text->id = sqlite3_column_int (stmt, 0);
      text->lang_id = lang_id;

      text->title = g_strdup ((gchar *)sqlite3_column_text (stmt, 1));
      text->tags = g_strdup ((gchar *)sqlite3_column_text (stmt, 2));

      text->text = NULL; /* Until explicitly loaded */

      text_list = g_list_append (text_list, text);
    }

  return text_list;
}

void
lr_database_text_free (lr_text_t *text)
{
  g_assert (text != NULL);

  g_free (text->title);
  g_free (text->tags);

  if (text->text != NULL)
    g_free (text->text);
}
