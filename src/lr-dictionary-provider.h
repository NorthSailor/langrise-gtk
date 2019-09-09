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

#ifndef _lr_dictionary_provider
#define _lr_dictionary_provider

#include <glib-object.h>

G_BEGIN_DECLS

#define LR_TYPE_DICTIONARY_PROVIDER lr_dictionary_provider_get_type ()
G_DECLARE_INTERFACE (LrDictionaryProvider, lr_dictionary_provider, LR, DICTIONARY_PROVIDER, GObject)

struct _LrDictionaryProviderInterface
{
  GTypeInterface parent_iface;

  void (*lookup) (LrDictionaryProvider *self, const gchar *term, gboolean editing_finished);
};

void lr_dictionary_provider_lookup (LrDictionaryProvider *self,
                                    const gchar *term,
                                    gboolean editing_finished);

G_END_DECLS

#endif
