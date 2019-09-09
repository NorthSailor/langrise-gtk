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

#ifndef _lr_language_h
#define _lr_language_h

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE (lr_language_get_type ())
G_DECLARE_FINAL_TYPE (LrLanguage, lr_language, LR, LANGUAGE, GObject)

LrLanguage *lr_language_new (int id,
                             const gchar *code,
                             const gchar *name,
                             const gchar *word_regex,
                             const gchar *separator_regex);

void lr_language_set_id (LrLanguage *self, int id);
int lr_language_get_id (LrLanguage *self);

void lr_language_set_code (LrLanguage *self, const gchar *code);
const gchar *lr_language_get_code (LrLanguage *self);

void lr_language_set_name (LrLanguage *self, const gchar *name);
const gchar *lr_language_get_name (LrLanguage *self);

void lr_language_set_word_regex (LrLanguage *self, const gchar *word_regex);
const gchar *lr_language_get_word_regex (LrLanguage *self);

void lr_language_set_separator_regex (LrLanguage *self, const gchar *separator_regex);
const gchar *lr_language_get_separator_regex (LrLanguage *self);

G_END_DECLS

#endif /* _lr_language_h */
