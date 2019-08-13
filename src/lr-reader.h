#ifndef _lr_reader_h
#define _lr_reader_h

#include <gtk/gtk.h>
#include "lr-text.h"

G_BEGIN_DECLS

#define LR_TYPE_READER (lr_reader_get_type ())
G_DECLARE_FINAL_TYPE (LrReader, lr_reader, LR, READER, GtkBox)

GtkWidget *lr_reader_new (void);

void lr_reader_set_text (LrReader *reader, LrText *text);

G_END_DECLS

#endif /* _lr_reader_h */

