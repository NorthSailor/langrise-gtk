/* 
 * Langrise, expanding L2 vocabulary in context.
 * Copyright (C) 2019 Iason Barmparesos
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _lr_lemmatizer_h
#define _lr_lemmatizer_h

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define LR_TYPE_LEMMATIZER (lr_lemmatizer_get_type ())
G_DECLARE_DERIVABLE_TYPE (LrLemmatizer, lr_lemmatizer, LR, LEMMATIZER, GObject)

struct _LrLemmatizerClass
{
  GObjectClass parent_class;

  gchar *(*populate_suggestions) (LrLemmatizer *self,
                                  GListStore *store,
                                  const char *text,
                                  GList *selection);

  /* No need to add padding since this class is not exposed through an ABI */
};

/* Populate the given GListStore with suggestions, and possible return a
 * message to be displayed to the user.
 */
gchar *lr_lemmatizer_populate_suggestions (LrLemmatizer *self,
                                           GListStore *store,
                                           const char *text,
                                           GList *selection);

LrLemmatizer *lr_lemmatizer_new_for_language (const gchar *code);

G_END_DECLS

#endif /* _lr_lemmatizer_h */
