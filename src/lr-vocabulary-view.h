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

#ifndef _lr_vocabulary_view_h
#define _lr_vocabulary_view_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_VOCABULARY_VIEW (lr_vocabulary_view_get_type ())
G_DECLARE_FINAL_TYPE (LrVocabularyView, lr_vocabulary_view, LR, VOCABULARY_VIEW, GtkBox)

GtkWidget *lr_vocabulary_view_new (LrDatabase *db);

void lr_vocabulary_view_set_language (LrVocabularyView *self, LrLanguage *language);
LrLanguage *lr_vocabulary_view_get_language (LrVocabularyView *self);

G_END_DECLS

#endif /* _lr_vocabulary_view_h */
