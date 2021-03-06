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

#ifndef _lr_window_h
#define _lr_window_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_MAIN_WINDOW (lr_main_window_get_type ())
G_DECLARE_FINAL_TYPE (LrMainWindow, lr_main_window, LR, MAIN_WINDOW, GtkApplicationWindow)

GtkWidget *lr_main_window_new (GtkApplication *application, LrDatabase *db);

void lr_main_window_set_database (LrMainWindow *self, LrDatabase *db);
LrDatabase *lr_main_window_get_database (LrMainWindow *self);

G_END_DECLS

#endif /* _lr_window_h */
