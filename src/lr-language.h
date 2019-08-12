#ifndef _lr_language_h
#define _lr_language_h

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE (lr_language_get_type ())
G_DECLARE_FINAL_TYPE (LrLanguage, lr_language, LR, LANGUAGE, GObject)

LrLanguage *lr_language_new (int id, const gchar *code, const gchar *name);

int lr_language_get_id (LrLanguage *self);
gchar *lr_language_get_code (LrLanguage *self);
gchar *lr_language_get_name (LrLanguage *self);

G_END_DECLS

#endif /* _lr_language_h */
