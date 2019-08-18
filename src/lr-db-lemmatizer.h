#ifndef _lr_db_lemmatizer_h
#define _lr_db_lemmatizer_h

#include <glib.h>
#include "lr-lemmatizer.h"

G_BEGIN_DECLS

#define LR_TYPE_DB_LEMMATIZER (lr_db_lemmatizer_get_type ())
G_DECLARE_FINAL_TYPE (LrDbLemmatizer, lr_db_lemmatizer, LR, DB_LEMMATIZER, LrLemmatizer)

LrLemmatizer *lr_db_lemmatizer_new (const gchar *path);

G_END_DECLS

#endif /* _lr_db_lemmatizer_h */
