#ifndef _lr_text_selector_h
#define _lr_text_selector_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LR_TYPE_TEXT_SELECTOR (lr_text_selector_get_type ())
G_DECLARE_FINAL_TYPE (LrTextSelector, lr_text_selector, LR, TEXT_SELECTOR, GtkBox)

GtkWidget *lr_text_selector_new ();

G_END_DECLS

#endif /* _lr_text_selector_h */
