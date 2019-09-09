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

#ifndef _lr_language_editor_dialog_h
#define _lr_language_editor_dialog_h

#include <gtk/gtk.h>
#include "lr-language.h"

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE_EDITOR_DIALOG (lr_language_editor_dialog_get_type ())
G_DECLARE_FINAL_TYPE (
  LrLanguageEditorDialog, lr_language_editor_dialog, LR, LANGUAGE_EDITOR_DIALOG, GtkDialog)

GtkWidget *lr_language_editor_dialog_new (LrLanguage *language, gboolean is_editing);

LrLanguage *lr_language_editor_dialog_get_language (LrLanguageEditorDialog *self);

G_END_DECLS

#endif /* _lr_language_editor_dialog_h */
