#include "lr-database.h"
#include <stdio.h>
#include <sqlite3.h>

typedef struct _lr_database_t
{
  sqlite3 *db;

  /* Get all languages */
  sqlite3_stmt *lang_stmt;

  /* Get all texts in a language */
  sqlite3_stmt *text_by_lang_stmt;

} lr_database_t;

static void
prepare_sql_statements (lr_database_t *db)
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
free_sql_statements (lr_database_t *db)
{
  sqlite3_finalize (db->lang_stmt);
  sqlite3_finalize (db->text_by_lang_stmt);
}

lr_database_t *
lr_database_open (const gchar *path)
{
  lr_database_t *db = g_malloc (sizeof (lr_database_t));

  int rc = sqlite3_open_v2 (path, &db->db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
    {
      g_critical (
        "Failed to open the database at '%s'. SQLite Error: '%s'", path, sqlite3_errmsg (db->db));
      sqlite3_close (db->db);
    }

  prepare_sql_statements (db);

  return db;
}

void
lr_database_close (lr_database_t *db)
{
  g_return_if_fail (db != NULL);
  g_return_if_fail (db->db != NULL);

  free_sql_statements (db);

  sqlite3_close (db->db);

  g_free (db);
}

GList *
lr_database_get_languages (lr_database_t *db)
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
lr_database_get_texts (lr_database_t *db, int lang_id)
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

