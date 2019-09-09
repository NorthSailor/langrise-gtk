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

#ifndef _lr_text_h
#define _lr_text_h

#include <glib-object.h>
#include "lr-language.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT (lr_text_get_type ())
G_DECLARE_FINAL_TYPE (LrText, lr_text, LR, TEXT, GObject)

LrText *lr_text_new (int id, LrLanguage *language, const gchar *title, const gchar *tags);

int lr_text_get_id (LrText *self);

LrLanguage *lr_text_get_language (LrText *self);

const gchar *lr_text_get_title (LrText *self);
void lr_text_set_title (LrText *self, const gchar *title);

const gchar *lr_text_get_tags (LrText *self);
void lr_text_set_tags (LrText *self, const gchar *title);

const gchar *lr_text_get_text (LrText *self);
void lr_text_set_text (LrText *self, const gchar *text);

G_END_DECLS

#endif /* _lr_text_h */
