#include "lr-database.h"
#include <stdio.h>
#include <sqlite3.h>

typedef struct _lr_database_t
{
  sqlite3 *db;

  /* Get all languages */
  sqlite3_stmt *lang_stmt;
} lr_database_t;

static void
prepare_sql_statements (lr_database_t *db)
{
  g_assert (sqlite3_prepare_v2 (
              db->db, "SELECT ID, Code, Name FROM Languages;", -1, &db->lang_stmt, NULL) ==
            SQLITE_OK);
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
lr_database_free_language (lr_language_t *lang)
{
  g_return_if_fail (lang != NULL);

  g_free (lang->code);
  g_free (lang->name);
}

