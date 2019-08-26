#include "lr-database.h"
#include "lr-lemma-instance.h"
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

  /* Get instances by text ID */
  sqlite3_stmt *instances_by_text_id;

  /* Get lemma by instance ID */
  sqlite3_stmt *lemma_by_instance_id;

  /* Update lemma by ID */
  sqlite3_stmt *update_lemma_by_id;

  /* Update instance by ID */
  sqlite3_stmt *update_instance_by_id;

  /* Insert (or ignore) lemma */
  sqlite3_stmt *insert_lemma;

  /* Get a lemma by its lemma and language ID */
  sqlite3_stmt *lemma_by_lemma_language;

  /* Insert a lemma instance */
  sqlite3_stmt *insert_instance;

  /* Remove instance by instance id */
  sqlite3_stmt *delete_instance_by_id;

  /* Remove orphaned lemma by ID */
  sqlite3_stmt *delete_orphaned_lemma_by_id;
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

  g_assert (sqlite3_prepare_v2 (db->db,
                                "SELECT ID, LemmaID, Words, Note FROM Instances WHERE TextID = ?;",
                                -1,
                                &db->instances_by_text_id,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "SELECT ID, Lemma, Translation FROM Lemmas WHERE ID = (SELECT "
                                "LemmaID FROM Instances WHERE ID = ?);",
                                -1,
                                &db->lemma_by_instance_id,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "UPDATE Lemmas SET Translation = ? WHERE ID = ?;",
                                -1,
                                &db->update_lemma_by_id,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "UPDATE Instances SET Note = ? WHERE ID = ?;",
                                -1,
                                &db->update_instance_by_id,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "INSERT OR IGNORE INTO Lemmas (Lemma, LanguageID, Translation) "
                                "VALUES (?1, ?2, \"No translation\");",
                                -1,
                                &db->insert_lemma,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "SELECT ID FROM Lemmas WHERE Lemma = ?1 AND LanguageID = ?2;",
                                -1,
                                &db->lemma_by_lemma_language,
                                NULL) == SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (
              db->db,
              "INSERT INTO Instances (LemmaID, TextID, Words, Note) VALUES (?1, ?2, ?3, \"\");",
              -1,
              &db->insert_instance,
              NULL) == SQLITE_OK);

  g_assert (
    sqlite3_prepare_v2 (
      db->db, "DELETE FROM Instances WHERE ID = ?1;", -1, &db->delete_instance_by_id, NULL) ==
    SQLITE_OK);

  g_assert (sqlite3_prepare_v2 (db->db,
                                "DELETE FROM Lemmas WHERE ID = ?1 AND (SELECT COUNT(ID) FROM "
                                "Instances WHERE LemmaID = ?1) == 0;",
                                -1,
                                &db->delete_orphaned_lemma_by_id,
                                NULL) == SQLITE_OK);
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
  sqlite3_finalize (db->instances_by_text_id);
  sqlite3_finalize (db->lemma_by_instance_id);
  sqlite3_finalize (db->update_lemma_by_id);
  sqlite3_finalize (db->update_instance_by_id);
  sqlite3_finalize (db->insert_lemma);
  sqlite3_finalize (db->lemma_by_lemma_language);
  sqlite3_finalize (db->insert_instance);
  sqlite3_finalize (db->delete_instance_by_id);
  sqlite3_finalize (db->delete_orphaned_lemma_by_id);
}

static void
enable_foreign_keys (LrDatabase *self)
{
  /* By running the following PRAGMA instruction, we enable foreign key
   * supports which means that our cascade deletes work. */
  gchar *error_message;
  g_assert (sqlite3_exec (self->db, "PRAGMA foreign_keys=ON", NULL, NULL, &error_message) ==
            SQLITE_OK);
  if (error_message)
    {
      g_message ("SQLite says: '%s'", error_message);
      sqlite3_free (error_message);
    }
}

static void
delete_orphaned_lemmas (LrDatabase *self)
{
  gchar *error_message;
  g_assert (
    sqlite3_exec (
      self->db,
      "DELETE FROM Lemmas WHERE (SELECT COUNT(ID) FROM Instances WHERE LemmaID == Lemmas.ID) == 0;",
      NULL,
      NULL,
      &error_message) == SQLITE_OK);
  if (error_message)
    {
      g_message ("Error while deleting orphaned lemmas; SQLite says: '%s'", error_message);
      sqlite3_free (error_message);
    }
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

  enable_foreign_keys (self);

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
lr_database_populate_lemma_instances (LrDatabase *self, GListStore *instance_store, LrText *text)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_TEXT (text));
  g_assert (G_IS_LIST_STORE (instance_store));

  g_assert (g_list_model_get_item_type (G_LIST_MODEL (instance_store)) == LR_TYPE_LEMMA_INSTANCE);

  g_list_store_remove_all (instance_store);

  sqlite3_stmt *stmt = self->instances_by_text_id;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lr_text_get_id (text));

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      int id = sqlite3_column_int (stmt, 0);
      int lemma_id = sqlite3_column_int (stmt, 1);
      const gchar *words = (const gchar *)sqlite3_column_text (stmt, 2);
      const gchar *note = (const gchar *)sqlite3_column_text (stmt, 3);

      LrLemmaInstance *instance = lr_lemma_instance_new (id, lemma_id, text, words, note);

      g_list_store_append (instance_store, instance);
      g_clear_object (&instance);
    }
}

LrLemma *
lr_database_load_lemma_from_instance (LrDatabase *self, LrLemmaInstance *instance)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_LEMMA_INSTANCE (instance));

  LrText *text = lr_lemma_instance_get_text (instance);
  LrLanguage *language = lr_text_get_language (text);

  sqlite3_stmt *stmt = self->lemma_by_instance_id;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lr_lemma_instance_get_id (instance));

  /* There should be only one result */
  g_assert (sqlite3_step (stmt) == SQLITE_ROW);

  int id = sqlite3_column_int (stmt, 0);
  const gchar *lemma_text = (const gchar *)sqlite3_column_text (stmt, 1);
  const gchar *translation = (const gchar *)sqlite3_column_text (stmt, 2);

  LrLemma *lemma = lr_lemma_new (id, lemma_text, translation, language);
  return lemma;
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

  delete_orphaned_lemmas (self);
}

void
lr_database_update_lemma (LrDatabase *self, LrLemma *lemma)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_LEMMA (lemma));

  sqlite3_stmt *stmt = self->update_lemma_by_id;
  sqlite3_reset (stmt);

  sqlite3_bind_text (stmt, 1, lr_lemma_get_translation (lemma), -1, NULL);
  sqlite3_bind_int (stmt, 2, lr_lemma_get_id (lemma));

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);
}

void
lr_database_update_instance (LrDatabase *self, LrLemmaInstance *instance)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_LEMMA_INSTANCE (instance));

  sqlite3_stmt *stmt = self->update_instance_by_id;
  sqlite3_reset (stmt);

  sqlite3_bind_text (stmt, 1, lr_lemma_instance_get_note (instance), -1, NULL);
  sqlite3_bind_int (stmt, 2, lr_lemma_instance_get_id (instance));

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);
}

void
lr_database_load_or_create_lemma (LrDatabase *self, LrLemma *lemma)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_LEMMA (lemma));

  sqlite3_stmt *stmt = self->insert_lemma;
  sqlite3_reset (stmt);

  sqlite3_bind_text (stmt, 1, lr_lemma_get_lemma (lemma), -1, NULL);
  sqlite3_bind_int (stmt, 2, lr_language_get_id (lr_lemma_get_language (lemma)));

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);

  stmt = self->lemma_by_lemma_language;
  sqlite3_reset (stmt);

  sqlite3_bind_text (stmt, 1, lr_lemma_get_lemma (lemma), -1, NULL);
  sqlite3_bind_int (stmt, 2, lr_language_get_id (lr_lemma_get_language (lemma)));

  g_assert (sqlite3_step (stmt) == SQLITE_ROW);

  int lemma_id = sqlite3_column_int (stmt, 0);

  lr_lemma_set_id (lemma, lemma_id);
}

void
lr_database_insert_instance (LrDatabase *self, LrLemmaInstance *instance)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_LEMMA_INSTANCE (instance));

  sqlite3_stmt *stmt = self->insert_instance;
  sqlite3_reset (stmt);

  int lemma_id = lr_lemma_instance_get_lemma_id (instance);
  int text_id = lr_text_get_id (lr_lemma_instance_get_text (instance));
  const gchar *words = lr_lemma_instance_get_words (instance);

  sqlite3_bind_int (stmt, 1, lemma_id);
  sqlite3_bind_int (stmt, 2, text_id);
  sqlite3_bind_text (stmt, 3, words, -1, NULL);

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);

  int id = sqlite3_last_insert_rowid (self->db);
  lr_lemma_instance_set_id (instance, id);
}

void
lr_database_delete_instance (LrDatabase *self, LrLemmaInstance *instance)
{
  g_assert (LR_IS_DATABASE (self));
  g_assert (LR_IS_LEMMA_INSTANCE (instance));

  sqlite3_stmt *stmt = self->delete_instance_by_id;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lr_lemma_instance_get_id (instance));

  g_assert (sqlite3_step (stmt) == SQLITE_DONE);

  stmt = self->delete_orphaned_lemma_by_id;
  sqlite3_reset (stmt);

  sqlite3_bind_int (stmt, 1, lr_lemma_instance_get_lemma_id (instance));
  g_assert (sqlite3_step (stmt) == SQLITE_DONE);
}
