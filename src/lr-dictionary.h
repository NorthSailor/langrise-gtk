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

#ifndef _lr_dictionary_h
#define _lr_dictionary_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LR_TYPE_DICTIONARY (lr_dictionary_get_type ())
G_DECLARE_FINAL_TYPE (LrDictionary, lr_dictionary, LR, DICTIONARY, GtkBox)

GtkWidget *lr_dictionary_new ();
void lr_dictionary_lookup (LrDictionary *self, const gchar *term);

G_END_DECLS

#endif /* _lr_dictionary_h */
