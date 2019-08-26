#ifndef _lr_language_manager_dialog_h
#define _lr_language_manager_dialog_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE_MANAGER_DIALOG (lr_language_manager_dialog_get_type ())
G_DECLARE_FINAL_TYPE (
  LrLanguageManagerDialog, lr_language_manager_dialog, LR, LANGUAGE_MANAGER_DIALOG, GtkDialog)

GtkWidget *lr_language_manager_dialog_new (LrDatabase *database);

G_END_DECLS

#endif /* _lr_language_manager_dialog_h */
