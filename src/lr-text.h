#ifndef _lr_text_h
#define _lr_text_h

#include <glib-object.h>
#include "lr-database.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT (lr_text_get_type ())
G_DECLARE_FINAL_TYPE (LrText, lr_text, LR, TEXT, GObject)

LrText *lr_text_new ();

G_END_DECLS

#endif /* _lr_text_h */
