#ifndef _lr_text_h
#define _lr_text_h

#include <glib-object.h>
#include "lr-language.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT (lr_text_get_type ())
G_DECLARE_FINAL_TYPE (LrText, lr_text, LR, TEXT, GObject)

LrText *lr_text_new (int id, LrLanguage *language, const gchar *title, const gchar *tags);

int lr_text_get_id (LrText *self);

LrLanguage *lr_text_get_language (LrText *self);

const gchar *lr_text_get_title (LrText *self);
void lr_text_set_title (LrText *self, const gchar *title);

const gchar *lr_text_get_tags (LrText *self);
void lr_text_set_tags (LrText *self, const gchar *title);

const gchar *lr_text_get_text (LrText *self);
void lr_text_set_text (LrText *self, const gchar *text);

G_END_DECLS

#endif /* _lr_text_h */
