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

#include "lr-dictionary-provider.h"

G_DEFINE_INTERFACE (LrDictionaryProvider, lr_dictionary_provider, G_TYPE_OBJECT)

static void
lr_dictionary_provider_default_init (LrDictionaryProviderInterface *iface)
{
  /* Add signals and properties here */
}

void
lr_dictionary_provider_lookup (LrDictionaryProvider *self,
                               const gchar *word,
                               gboolean editing_finished)
{
  LrDictionaryProviderInterface *iface;

  g_return_if_fail (LR_IS_DICTIONARY_PROVIDER (self));

  iface = LR_DICTIONARY_PROVIDER_GET_IFACE (self);
  g_return_if_fail (iface->lookup != NULL);

  iface->lookup (self, word, editing_finished);
}
