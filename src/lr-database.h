#ifndef _lr_database_h
#define _lr_database_h

#include <glib.h>

G_BEGIN_DECLS

typedef struct _lr_database_t lr_database_t;

typedef struct
{
  int id;
  gchar *code;
  gchar *name;
} lr_language_t;

typedef struct
{
  int id;
  int lang_id;
  gchar *title;
  gchar *tags;

  /*
   * Note: Since the text can be very long and this struct will also
   * be used to show the list of all texts, the actual text is not
   * loaded until specifically requested. By default 'text' should be NULL.
   */
  gchar *text;
} lr_text_t;

lr_database_t *lr_database_open (const gchar *path);
void lr_database_close (lr_database_t *db);

GList *lr_database_get_languages (lr_database_t *db);

void lr_database_language_free (lr_language_t *lang);

GList *lr_database_get_texts (lr_database_t *db, int lang_id);
void lr_database_text_free (lr_text_t *text);

G_END_DECLS

#endif /* _lr_database_h */
