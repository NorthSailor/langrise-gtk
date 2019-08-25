#ifndef _lr_lemma_h
#define _lr_lemma_h

#include <glib.h>
#include <glib-object.h>

#include "lr-language.h"

G_BEGIN_DECLS

#define LR_TYPE_LEMMA (lr_lemma_get_type ())
G_DECLARE_FINAL_TYPE (LrLemma, lr_lemma, LR, LEMMA, GObject)

LrLemma *lr_lemma_new (int id, const gchar *lemma, const gchar *translation, LrLanguage *language);

void lr_lemma_set_id (LrLemma *self, int id);
int lr_lemma_get_id (LrLemma *self);

const gchar *lr_lemma_get_lemma (LrLemma *self);
void lr_lemma_set_lemma (LrLemma *self, const gchar *lemma);

void lr_lemma_set_translation (LrLemma *self, const gchar *translation);
const gchar *lr_lemma_get_translation (LrLemma *self);

void lr_lemma_set_language (LrLemma *self, LrLanguage *language);
LrLanguage *lr_lemma_get_language (LrLemma *self);

G_END_DECLS

#endif /* _lr_lemma_h */
