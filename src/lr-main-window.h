#ifndef _lr_window_h
#define _lr_window_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_MAIN_WINDOW (lr_main_window_get_type ())
G_DECLARE_FINAL_TYPE (LrMainWindow, lr_main_window, LR, MAIN_WINDOW, GtkApplicationWindow)

GtkWidget *lr_main_window_new (GtkApplication *application);

void lr_main_window_set_database (LrMainWindow *self, lr_database_t *db);
lr_database_t *lr_main_window_get_database (LrMainWindow *self);

G_END_DECLS

#endif /* _lr_window_h */
