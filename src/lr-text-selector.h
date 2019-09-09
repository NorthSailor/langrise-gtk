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

#ifndef _lr_text_selector_h
#define _lr_text_selector_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT_SELECTOR (lr_text_selector_get_type ())
G_DECLARE_FINAL_TYPE (LrTextSelector, lr_text_selector, LR, TEXT_SELECTOR, GtkBox)

GtkWidget *lr_text_selector_new ();

void lr_text_selector_set_database (LrTextSelector *self, LrDatabase *db);
void lr_text_selector_set_language (LrTextSelector *self, LrLanguage *new_language);

G_END_DECLS

#endif /* _lr_text_selector_h */
