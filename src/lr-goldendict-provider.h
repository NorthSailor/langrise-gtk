#ifndef _lr_goldendict_provider_h
#define _lr_goldendict_provider_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LR_TYPE_GOLDENDICT_PROVIDER (lr_goldendict_provider_get_type ())
G_DECLARE_FINAL_TYPE (LrGoldendictProvider, lr_goldendict_provider, LR, GOLDENDICT_PROVIDER, GtkBox)

GtkWidget *lr_goldendict_provider_new ();

G_END_DECLS

#endif /* _lr_goldendict_provider_h */
