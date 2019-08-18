#ifndef _lr_lemma_suggestion_h
#define _lr_lemma_suggestion_h

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define LR_TYPE_LEMMA_SUGGESTION (lr_lemma_suggestion_get_type ())
G_DECLARE_FINAL_TYPE (LrLemmaSuggestion, lr_lemma_suggestion, LR, LEMMA_SUGGESTION, GObject)

LrLemmaSuggestion *lr_lemma_suggestion_new (const gchar *lemma, const gchar *note);

const gchar *lr_lemma_suggestion_get_lemma (LrLemmaSuggestion *self);
const gchar *lr_lemma_suggestion_get_note (LrLemmaSuggestion *self);

G_END_DECLS

#endif /* _lr_lemma_suggestion_h */
