#ifndef _lr_database_h
#define _lr_database_h

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "lr-language.h"
#include "lr-text.h"
#include "lr-lemma-instance.h"

G_BEGIN_DECLS

#define LR_TYPE_DATABASE (lr_database_get_type ())
G_DECLARE_FINAL_TYPE (LrDatabase, lr_database, LR, DATABASE, GObject)

LrDatabase *lr_database_new (gchar *path);
void lr_database_close (LrDatabase *self);

void lr_database_populate_languages (LrDatabase *self, GListStore *store);
void lr_database_populate_texts (LrDatabase *self, GListStore *store, LrLanguage *language);
void
lr_database_populate_lemma_instances (LrDatabase *self, GListStore *instance_store, LrText *text);

LrLemma *lr_database_load_lemma_from_instance (LrDatabase *self, LrLemmaInstance *instance);

void lr_database_load_text (LrDatabase *self, LrText *text);
void lr_database_insert_text (LrDatabase *self, LrText *text);
void lr_database_update_text (LrDatabase *self, LrText *text);
void lr_database_delete_text (LrDatabase *self, LrText *text);

G_END_DECLS

#endif /* _lr_database_h */
