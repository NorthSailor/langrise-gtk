#ifndef _lr_vocabulary_view_h
#define _lr_vocabulary_view_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LR_TYPE_VOCABULARY_VIEW (lr_vocabulary_view_get_type ())
G_DECLARE_FINAL_TYPE (LrVocabularyView, lr_vocabulary_view, LR, VOCABULARY_VIEW, GtkBox)

GtkWidget *lr_vocabulary_view_new ();

G_END_DECLS

#endif /* _lr_vocabulary_view_h */
