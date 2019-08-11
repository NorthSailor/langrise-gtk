#ifndef _lr_language_switcher_h
#define _lr_language_switcher_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE_SWITCHER (lr_language_switcher_get_type ())
G_DECLARE_FINAL_TYPE (LrLanguageSwitcher, lr_language_switcher, LR, LANGUAGE_SWITCHER, GtkButton)

GtkWidget *lr_language_switcher_new ();

void lr_language_set_language_list (LrLanguageSwitcher *self, GList *lang_list, int active_id);

G_END_DECLS

#endif /* _lr_language_switcher_h */
