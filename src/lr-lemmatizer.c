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

#include "lr-lemmatizer.h"

G_DEFINE_TYPE (LrLemmatizer, lr_lemmatizer, G_TYPE_OBJECT)

static void
lr_lemmatizer_init (LrLemmatizer *self)
{
  /* Instance initialization */
}

static gchar *
base_lemmatizer_suggestions (LrLemmatizer *self,
                             GListStore *store,
                             const char *text,
                             GList *selection)
{
  return g_strdup ("No lemma suggestions\navailable for this language.");
  return NULL;
}

static void
lr_lemmatizer_class_init (LrLemmatizerClass *klass)
{
  klass->populate_suggestions = base_lemmatizer_suggestions;
}

gchar *
lr_lemmatizer_populate_suggestions (LrLemmatizer *self,
                                    GListStore *store,
                                    const char *text,
                                    GList *selection)
{
  g_list_store_remove_all (store);

  return LR_LEMMATIZER_GET_CLASS (self)->populate_suggestions (self, store, text, selection);
}

#include "lr-db-lemmatizer.h"

LrLemmatizer *
lr_lemmatizer_new_for_language (const gchar *code)
{
  if (g_strcmp0 (code, "pl") == 0)
    return lr_db_lemmatizer_new ("pl");
  else
    return g_object_new (LR_TYPE_LEMMATIZER, NULL);
}
