#ifndef _lr_dictionary_h
#define _lr_dictionary_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LR_TYPE_DICTIONARY (lr_dictionary_get_type ())
G_DECLARE_FINAL_TYPE (LrDictionary, lr_dictionary, LR, DICTIONARY, GtkBox)

GtkWidget *lr_dictionary_new ();

G_END_DECLS

#endif /* _lr_dictionary_h */
