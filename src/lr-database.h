/* 
 * Langrise, expanding L2 vocabulary in context.
 * Copyright (C) 2019 Iason Barmparesos
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

void lr_database_insert_language (LrDatabase *self, LrLanguage *language);
void lr_database_update_language (LrDatabase *self, LrLanguage *language);
void lr_database_delete_language (LrDatabase *self, LrLanguage *language);

void lr_database_populate_languages (LrDatabase *self, GListStore *store);
void lr_database_populate_texts (LrDatabase *self, GListStore *store, LrLanguage *language);
void
lr_database_populate_lemma_instances (LrDatabase *self, GListStore *instance_store, LrText *text);

LrLemma *lr_database_load_lemma_from_instance (LrDatabase *self, LrLemmaInstance *instance);

void lr_database_load_text (LrDatabase *self, LrText *text);
void lr_database_insert_text (LrDatabase *self, LrText *text);
void lr_database_update_text (LrDatabase *self, LrText *text);
void lr_database_delete_text (LrDatabase *self, LrText *text);

void lr_database_update_lemma (LrDatabase *self, LrLemma *lemma);
void lr_database_update_instance (LrDatabase *self, LrLemmaInstance *instance);

void lr_database_load_or_create_lemma (LrDatabase *self, LrLemma *lemma);
void lr_database_insert_instance (LrDatabase *self, LrLemmaInstance *instance);

void lr_database_delete_instance (LrDatabase *self, LrLemmaInstance *instance);

/* Vocabulary export functions */
typedef struct
{
  LrText *text;
  char *words;
  char *lemma;
  char *translation;
  char *note;
} lr_vocabulary_item_t;

GList *lr_database_get_vocabulary_items_for_text (LrDatabase *self, LrText *text);

void lr_vocabulary_item_free (lr_vocabulary_item_t *self);

G_END_DECLS

#endif /* _lr_database_h */
