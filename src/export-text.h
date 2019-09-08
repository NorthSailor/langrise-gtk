#ifndef _lr_export_text_h
#define _lr_export_text_h

#include <gtk/gtk.h>
#include "lr-database.h"

void lr_export_text (GtkWidget *toplevel,
                     LrDatabase *db,
                     GList *items,
                     const gchar *prefix,
                     const gchar *postfix,
                     const gchar *field_sep,
                     const gchar *preamble,
                     const gchar *postamble,
                     const gchar *filter_name,
                     const gchar *filter);

#endif /* _lr_export_text_h */
