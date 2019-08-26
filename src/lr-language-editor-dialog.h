#ifndef _lr_language_editor_dialog_h
#define _lr_language_editor_dialog_h

#include <gtk/gtk.h>
#include "lr-language.h"

G_BEGIN_DECLS

#define LR_TYPE_LANGUAGE_EDITOR_DIALOG (lr_language_editor_dialog_get_type ())
G_DECLARE_FINAL_TYPE (
  LrLanguageEditorDialog, lr_language_editor_dialog, LR, LANGUAGE_EDITOR_DIALOG, GtkDialog)

GtkWidget *lr_language_editor_dialog_new (LrLanguage *language, gboolean is_editing);

LrLanguage *lr_language_editor_dialog_get_language (LrLanguageEditorDialog *self);

G_END_DECLS

#endif /* _lr_language_editor_dialog_h */
