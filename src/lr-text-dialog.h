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

#ifndef _lr_text_dialog_h
#define lr_text_dialog_h

#include <gtk/gtk.h>
#include "lr-text.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT_DIALOG (lr_text_dialog_get_type ())
G_DECLARE_FINAL_TYPE (LrTextDialog, lr_text_dialog, LR, TEXT_DIALOG, GtkDialog)

GtkWidget *lr_text_dialog_new (LrText *text);

void lr_text_dialog_set_text (LrTextDialog *self, LrText *text);
LrText *lr_text_dialog_get_text (LrTextDialog *self);

G_END_DECLS

#endif /* lr_text_dialog */
