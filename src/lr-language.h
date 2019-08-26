#ifndef _lr_language_h
#define _lr_language_h

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE (lr_language_get_type ())
G_DECLARE_FINAL_TYPE (LrLanguage, lr_language, LR, LANGUAGE, GObject)

LrLanguage *lr_language_new (int id, const gchar *code, const gchar *name, const gchar *word_regex);

void lr_language_set_id (LrLanguage *self, int id);
int lr_language_get_id (LrLanguage *self);

void lr_language_set_code (LrLanguage *self, const gchar *code);
const gchar *lr_language_get_code (LrLanguage *self);

void lr_language_set_name (LrLanguage *self, const gchar *name);
const gchar *lr_language_get_name (LrLanguage *self);

void lr_language_set_word_regex (LrLanguage *self, const gchar *word_regex);
const gchar *lr_language_get_word_regex (LrLanguage *self);


G_END_DECLS

#endif /* _lr_language_h */
