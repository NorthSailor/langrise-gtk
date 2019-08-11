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

lr_database_t *lr_database_open (const gchar *path);
void lr_database_close (lr_database_t *db);

GList *lr_database_get_languages (lr_database_t *db);

void lr_database_free_language (lr_language_t *lang);

G_END_DECLS

#endif /* _lr_database_h */
