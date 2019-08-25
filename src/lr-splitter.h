#ifndef _lr_splitter_h
#define _lr_splitter_h

#include <glib.h>
#include <glib-object.h>
#include "lr-text.h"

G_BEGIN_DECLS

typedef struct
{
  int start, end;
} lr_range_t;

#define LR_TYPE_SPLITTER (lr_splitter_get_type ())
G_DECLARE_FINAL_TYPE (LrSplitter, lr_splitter, LR, SPLITTER, GObject)

LrSplitter *lr_splitter_new (LrText *text);

const GArray *lr_splitter_get_words (LrSplitter *self);

const lr_range_t *lr_splitter_get_word_at_index (LrSplitter *self, int index);

GList *lr_splitter_ranges_from_string (LrSplitter *self, const gchar *range);

G_END_DECLS

#endif /* lr_splitter */
