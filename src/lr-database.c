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

  /* Get text for a text by ID */
  sqlite3_stmt *text_text_by_id;

  /* Insert a text with title, tags, and text */
  sqlite3_stmt *insert_text;

  /* Update a text by its ID */
  sqlite3_stmt *update_text_by_id;

  /* Delete text by ID */
  sqlite3_stmt *delete_text_by_id;
};

enum
{
  PROP_PATH = 1,
};

G_DEFINE_TYPE (LrDatabase, lr_database, G_TYPE_OBJECT)

static void
prepare_sql_statements (LrDatabase *db)
{
  g_assert (
    sqlite3_prepare_v2 (
      db->db, "SELECT ID, Code, Name, WordRegex FROM Languages;", -1, &db->lang_stmt, NULL) ==
    SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (
              db->db,
              "SELECT ID, Title, Tags FROM Texts WHERE LanguageID = ? ORDER BY Title ASC;",
              -1,
              &db->text_by_lang_stmt,
              NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (
              db->db, "SELECT Text FROM Texts WHERE ID = ?;", -1, &db->text_text_by_id, NULL) ==
            SQLITE_OK);

  g_assert (
    sqlite3_prepare_v2 (db->db,
                        "INSERT INTO Texts (LanguageID, Title, Tags, Text) VALUES (?, ?, ?, ?)",
                        -1,
                        &db->insert_text,
                        NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "UPDATE Texts SET Title = ?, Tags = ?, Text = ? WHERE ID = ?;",
                                -1,
                                &db->update_text_by_id,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (
              db->db, "DELETE FROM Texts WHERE ID = ?;", -1, &db->delete_text_by_id, NULL) ==
            SQLITE_OK);
}

static void
free_sql_statements (LrDatabase *db)
{
  sqlite3_finalize (db->lang_stmt);
  sqlite3_finalize (db->text_by_lang_stmt);
  sqlite3_finalize (db->text_text_by_id);
  sqlite3_finalize (db->insert_text);
  sqlite3_finalize (db->update_text_by_id);
  sqlite3_finalize (db->delete_text_by_id);
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

void
lr_database_populate_languages (LrDatabase *self, GListStore *store)
{
  g_assert (g_list_model_get_item_type (G_LIST_MODEL (store)) == LR_TYPE_LANGUAGE);

  /* Free all the previous languages */
  g_list_store_remove_all (store);

  sqlite3_reset (self->lang_stmt);

  while (sqlite3_step (self->lang_stmt) == SQLITE_ROW)
    {
      int id = sqlite3_column_int (self->lang_stmt, 0);
      const gchar *code = (const gchar *)sqlite3_column_text (self->lang_stmt, 1);
      const gchar *name = (const gchar *)sqlite3_column_text (self->lang_stmt, 2);
      const gchar *word_regex = (const gchar *)sqlite3_column_text (self->lang_stmt, 3);

      LrLanguage *lang = lr_language_new (id, code, name, word_regex);

      g_list_store_append (store, lang);

      g_object_unref (lang);
    }
}

void
lr_database_populate_texts (LrDatabase *self, GListStore *store, LrLanguage *language)
{
  g_assert (g_list_model_get_item_type (G_LIST_MODEL (store)) == LR_TYPE_TEXT);

  /* Free all previous texts */
  g_list_store_remove_all (store);

  sqlite3_stmt *stmt = self->text_by_lang_stmt;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lr_language_get_id (language));

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      int id = sqlite3_column_int (stmt, 0);
      const gchar *title = (const gchar *)sqlite3_column_text (stmt, 1);
      const gchar *tags = (const gchar *)sqlite3_column_text (stmt, 2);

      LrText *text = lr_text_new (id, language, title, tags);

      g_list_store_append (store, text);
      g_object_unref (text);
    }
}

void
lr_database_load_text (LrDatabase *self, LrText *text)

{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_TEXT (text));

  sqlite3_stmt *stmt = self->text_text_by_id;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lr_text_get_id (text));

  g_assert (sqlite3_step (stmt) == SQLITE_ROW);

  const gchar *text_string = (const gchar *)sqlite3_column_text (stmt, 0);
  lr_text_set_text (text, text_string);
}

void
lr_database_insert_text (LrDatabase *self, LrText *text)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_TEXT (text));

  sqlite3_stmt *stmt = self->insert_text;

  sqlite3_reset (stmt);

  LrLanguage *language = lr_text_get_language (text);

  sqlite3_bind_int (stmt, 1, lr_language_get_id (language));
  sqlite3_bind_text (stmt, 2, lr_text_get_title (text), -1, NULL);
  sqlite3_bind_text (stmt, 3, lr_text_get_tags (text), -1, NULL);
  sqlite3_bind_text (stmt, 4, lr_text_get_text (text), -1, NULL);

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);
}

void
lr_database_update_text (LrDatabase *self, LrText *text)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_TEXT (text));

  /* Make sure the text has been loaded first */
  g_assert (lr_text_get_text (text) != NULL);

  sqlite3_stmt *stmt = self->update_text_by_id;
  sqlite3_reset (stmt);

  sqlite3_bind_text (stmt, 1, lr_text_get_title (text), -1, NULL);
  sqlite3_bind_text (stmt, 2, lr_text_get_tags (text), -1, NULL);
  sqlite3_bind_text (stmt, 3, lr_text_get_text (text), -1, NULL);

  sqlite3_bind_int (stmt, 4, lr_text_get_id (text));

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);
}

void
lr_database_delete_text (LrDatabase *self, LrText *text)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_TEXT (text));

  sqlite3_stmt *stmt = self->delete_text_by_id;
  sqlite3_reset (stmt);
  sqlite3_bind_int (stmt, 1, lr_text_get_id (text));

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);
}
