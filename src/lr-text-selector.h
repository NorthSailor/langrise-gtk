#ifndef _lr_text_selector_h
#define _lr_text_selector_h

#include <gtk/gtk.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT_SELECTOR (lr_text_selector_get_type ())
G_DECLARE_FINAL_TYPE (LrTextSelector, lr_text_selector, LR, TEXT_SELECTOR, GtkBox)

GtkWidget *lr_text_selector_new ();

void lr_text_selector_set_database (LrTextSelector *self, lr_database_t *db);
void lr_text_selector_set_language (LrTextSelector *self, lr_language_t *new_language);

G_END_DECLS

#endif /* _lr_text_selector_h */
