#ifndef _lr_text_dialog_h
#define lr_text_dialog_h

#include <gtk/gtk.h>
#include "lr-text.h"

G_BEGIN_DECLS

#define LR_TYPE_TEXT_DIALOG (lr_text_dialog_get_type ())
G_DECLARE_FINAL_TYPE (LrTextDialog, lr_text_dialog, LR, TEXT_DIALOG, GtkDialog)

GtkWidget *lr_text_dialog_new (LrText *text);

void lr_text_dialog_set_text (LrTextDialog *self, LrText *text);
LrText *lr_text_dialog_get_text (LrTextDialog *self);

G_END_DECLS

#endif /* lr_text_dialog */
