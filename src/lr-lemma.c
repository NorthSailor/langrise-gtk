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

#include "lr-lemma.h"

struct _LrLemma
{
  GObject parent_instance;

  int id;
  gchar *lemma;
  gchar *translation;
  LrLanguage *language;
};

enum
{
  PROP_0,
  PROP_ID,
  PROP_LEMMA,
  PROP_TRANSLATION,
  PROP_LANGUAGE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrLemma, lr_lemma, G_TYPE_OBJECT)

static void
lr_lemma_init (LrLemma *self)
{
  /* Instance initialization */
}

static void
lr_lemma_finalize (GObject *object)
{
  LrLemma *self = LR_LEMMA (object);

  g_free (self->lemma);
  g_free (self->translation);

  G_OBJECT_CLASS (lr_lemma_parent_class)->finalize (object);
}

static void
lr_lemma_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  LrLemma *self = LR_LEMMA (object);

  switch (property_id)
    {
    case PROP_ID:
      lr_lemma_set_id (self, g_value_get_int (value));
      break;
    case PROP_LEMMA:
      lr_lemma_set_lemma (self, g_value_get_string (value));
      break;
    case PROP_TRANSLATION:
      lr_lemma_set_translation (self, g_value_get_string (value));
      break;
    case PROP_LANGUAGE:
      lr_lemma_set_language (self, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_lemma_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  LrLemma *self = LR_LEMMA (object);

  switch (property_id)
    {
    case PROP_ID:
      g_value_set_int (value, lr_lemma_get_id (self));
      break;
    case PROP_LEMMA:
      g_value_set_string (value, lr_lemma_get_lemma (self));
      break;
    case PROP_TRANSLATION:
      g_value_set_string (value, lr_lemma_get_translation (self));
      break;
    case PROP_LANGUAGE:
      g_value_set_object (value, lr_lemma_get_language (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_lemma_class_init (LrLemmaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_lemma_finalize;
  object_class->set_property = lr_lemma_set_property;
  object_class->get_property = lr_lemma_get_property;

  obj_properties[PROP_ID] = g_param_spec_int (
    "id", "ID", "The ID", -1, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  obj_properties[PROP_LEMMA] = g_param_spec_string (
    "lemma", "Lemma", "The lemma", "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  obj_properties[PROP_TRANSLATION] = g_param_spec_string (
    "translation", "Translation", "The translation of the lemma", "", G_PARAM_READWRITE);
  obj_properties[PROP_LANGUAGE] = g_param_spec_object ("language",
                                                       "Language",
                                                       "The language",
                                                       LR_TYPE_LANGUAGE,
                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

LrLemma *
lr_lemma_new (int id, const gchar *lemma, const gchar *translation, LrLanguage *language)
{
  return g_object_new (LR_TYPE_LEMMA,
                       "id",
                       id,
                       "lemma",
                       lemma,
                       "translation",
                       translation,
                       "language",
                       language,
                       NULL);
}

void
lr_lemma_set_id (LrLemma *self, int id)
{
  self->id = id;
}

int
lr_lemma_get_id (LrLemma *self)
{
  return self->id;
}

const gchar *
lr_lemma_get_lemma (LrLemma *self)
{
  return self->lemma;
}

void
lr_lemma_set_lemma (LrLemma *self, const gchar *lemma)
{
  g_free (self->lemma);
  self->lemma = g_strdup (lemma);
}

void
lr_lemma_set_translation (LrLemma *self, const gchar *translation)
{
  g_free (self->translation);
  self->translation = g_strdup (translation);
}

const gchar *
lr_lemma_get_translation (LrLemma *self)
{
  return self->translation;
}

void
lr_lemma_set_language (LrLemma *self, LrLanguage *language)
{
  g_assert (LR_IS_LANGUAGE (language));
  self->language = language;
}

LrLanguage *
lr_lemma_get_language (LrLemma *self)
{
  return self->language;
}

