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

#ifndef _lr_lemma_instance_h
#define _lr_lemma_instance_h

#include <glib.h>
#include <glib-object.h>

#include "lr-lemma.h"
#include "lr-text.h"

G_BEGIN_DECLS

#define LR_TYPE_LEMMA_INSTANCE (lr_lemma_instance_get_type ())
G_DECLARE_FINAL_TYPE (LrLemmaInstance, lr_lemma_instance, LR, LEMMA_INSTANCE, GObject)

/*
 * NOTE
 *
 * Unlike other database wrapper objects, you will notice that this instance stores the lemma
 * id instead of a pointer to the object. This is done since the LrLemma can be shared by multiple
 * objects and is loaded on demand only, so that any modification written to the database will
 * propagate to all instances.
 */

LrLemmaInstance *
lr_lemma_instance_new (int id, int lemma_id, LrText *text, const gchar *words, const gchar *note);

void lr_lemma_instance_set_id (LrLemmaInstance *self, int id);
int lr_lemma_instance_get_id (LrLemmaInstance *self);

void lr_lemma_instance_set_lemma_id (LrLemmaInstance *self, int lemma_id);
int lr_lemma_instance_get_lemma_id (LrLemmaInstance *self);

void lr_lemma_instance_set_text (LrLemmaInstance *self, LrText *text);
LrText *lr_lemma_instance_get_text (LrLemmaInstance *self);

void lr_lemma_instance_set_words (LrLemmaInstance *self, const gchar *words);
const gchar *lr_lemma_instance_get_words (LrLemmaInstance *self);

void lr_lemma_instance_set_note (LrLemmaInstance *self, const gchar *note);
const gchar *lr_lemma_instance_get_note (LrLemmaInstance *self);

G_END_DECLS

#endif /* _lr_lemma_instance_h */
