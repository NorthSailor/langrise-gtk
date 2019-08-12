#ifndef _lr_database_h
#define _lr_database_h

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "lr-language.h"

G_BEGIN_DECLS

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

#define LR_TYPE_DATABASE (lr_database_get_type ())
G_DECLARE_FINAL_TYPE (LrDatabase, lr_database, LR, DATABASE, GObject)

LrDatabase *lr_database_new (gchar *path);
void lr_database_close (LrDatabase *self);

void lr_database_populate_languages (LrDatabase *self, GListStore *store);

GList *lr_database_get_texts (LrDatabase *self, int lang_id);
void lr_database_text_free (lr_text_t *text);

G_END_DECLS

#endif /* _lr_database_h */
