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

#ifndef _lr_lemma_h
#define _lr_lemma_h

#include <glib.h>
#include <glib-object.h>

#include "lr-language.h"

G_BEGIN_DECLS

#define LR_TYPE_LEMMA (lr_lemma_get_type ())
G_DECLARE_FINAL_TYPE (LrLemma, lr_lemma, LR, LEMMA, GObject)

LrLemma *lr_lemma_new (int id, const gchar *lemma, const gchar *translation, LrLanguage *language);

void lr_lemma_set_id (LrLemma *self, int id);
int lr_lemma_get_id (LrLemma *self);

const gchar *lr_lemma_get_lemma (LrLemma *self);
void lr_lemma_set_lemma (LrLemma *self, const gchar *lemma);

void lr_lemma_set_translation (LrLemma *self, const gchar *translation);
const gchar *lr_lemma_get_translation (LrLemma *self);

void lr_lemma_set_language (LrLemma *self, LrLanguage *language);
LrLanguage *lr_lemma_get_language (LrLemma *self);

G_END_DECLS

#endif /* _lr_lemma_h */
