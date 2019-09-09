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

#include "lr-splitter.h"

struct _LrSplitter
{
  GObject parent_instance;

  LrText *text;

  GRegex *word_regex;
  GRegex *separator_regex;

  GArray *words;
  GArray *separators;
};

enum
{
  PROP_0,
  PROP_TEXT,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrSplitter, lr_splitter, G_TYPE_OBJECT)

static void
lr_splitter_init (LrSplitter *self)
{
}

static void
lr_splitter_constructed (GObject *obj)
{
  LrSplitter *self = LR_SPLITTER (obj);
  g_assert (LR_IS_TEXT (self->text));

  LrText *text = self->text;
  const gchar *word_regex_string = lr_language_get_word_regex (lr_text_get_language (text));
  const gchar *separator_regex_string =
    lr_language_get_separator_regex (lr_text_get_language (text));

  self->word_regex = g_regex_new (word_regex_string, 0, 0, NULL);
  g_assert (self->word_regex != NULL);

  self->separator_regex = g_regex_new (separator_regex_string, 0, 0, NULL);
  g_assert (self->separator_regex != NULL);

  self->words = g_array_new (FALSE, FALSE, sizeof (lr_range_t));
  self->separators = g_array_new (FALSE, FALSE, sizeof (lr_range_t));

  /* Split the text */
  GMatchInfo *match_info;
  g_regex_match (self->word_regex, lr_text_get_text (text), 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      lr_range_t range;
      g_match_info_fetch_pos (match_info, 0, &range.start, &range.end);
      g_array_append_val (self->words, range);
      g_match_info_next (match_info, NULL);
    }

  /* Find the separators as well */
  g_regex_match (self->separator_regex, lr_text_get_text (text), 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      lr_range_t range;
      g_match_info_fetch_pos (match_info, 0, &range.start, &range.end);
      g_array_append_val (self->separators, range);
      g_match_info_next (match_info, NULL);
    }

  g_match_info_free (match_info);
}

static void
lr_splitter_finalize (GObject *object)
{
  LrSplitter *self = LR_SPLITTER (object);

  g_array_free (self->words, TRUE);
  g_regex_unref (self->word_regex);
  g_regex_unref (self->separator_regex);

  G_OBJECT_CLASS (lr_splitter_parent_class)->finalize (object);
}

static void
lr_splitter_set_property (GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  LrSplitter *self = LR_SPLITTER (object);

  switch (property_id)
    {
    case PROP_TEXT:
      self->text = LR_TEXT (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_splitter_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  LrSplitter *self = LR_SPLITTER (object);

  switch (property_id)
    {
    case PROP_TEXT:
      g_value_take_object (value, self->text);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_splitter_class_init (LrSplitterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_splitter_finalize;
  object_class->constructed = lr_splitter_constructed;
  object_class->set_property = lr_splitter_set_property;
  object_class->get_property = lr_splitter_get_property;

  obj_properties[PROP_TEXT] = g_param_spec_object ("text",
                                                   "Language",
                                                   "The text to be split.",
                                                   LR_TYPE_TEXT,
                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

LrSplitter *
lr_splitter_new (LrText *text)
{
  return g_object_new (LR_TYPE_SPLITTER, "text", text, NULL);
}

const GArray *
lr_splitter_get_words (LrSplitter *self)
{
  g_assert (LR_IS_SPLITTER (self));

  return self->words;
}

const lr_range_t *
lr_splitter_get_word_at_index (LrSplitter *self, int index)
{
  for (int i = 0; i < self->words->len; ++i)
    {
      lr_range_t *range = &g_array_index (self->words, lr_range_t, i);
      if (index < range->start)
        return NULL;

      if (index <= range->end)
        return range;
    }
  return NULL;
}

GList *
lr_splitter_ranges_from_string (LrSplitter *self, const gchar *range)
{
  GList *list = NULL;

  gchar **ranges = g_strsplit (range, ";", -1);

  for (int i = 0; ranges[i] != NULL; ++i)
    {
      const gchar *string = ranges[i];
      int word = g_ascii_strtoll (string, NULL, 10);

      lr_range_t *range = &g_array_index (self->words, lr_range_t, word);
      list = g_list_append (list, range);
    }

  g_strfreev (ranges);

  return list;
}

static int
word_index_from_range (LrSplitter *self, lr_range_t *range)
{
  for (int i = 0; i < self->words->len; ++i)
    {
      lr_range_t *current_range = &g_array_index (self->words, lr_range_t, i);
      if (current_range == range)
        return i;
    }
  return -1;
}

gchar *
lr_splitter_selection_to_text (LrSplitter *self, GList *selection)
{
  int n_words = g_list_length (selection);

  gchar **index_str_array = g_malloc ((n_words + 1) * sizeof (gchar *));
  index_str_array[n_words] = NULL;

  int i = 0;
  for (GList *l = selection; l != NULL; l = l->next)
    {
      lr_range_t *range = (lr_range_t *)l->data;
      int word_index = word_index_from_range (self, range);
      g_assert (word_index > -1);

      index_str_array[i] = g_strdup_printf ("%d", word_index);
      i++;
    }

  gchar *joint = g_strjoinv (";", index_str_array);
  g_strfreev (index_str_array);

  return joint;
}

static gint
compare_ranges (lr_range_t *first, lr_range_t *second)
{
  if (first->start < second->start)
    return -1;
  else if (first->start == second->start)
    return 0;
  else
    return 1;
}

/**
 * Creates a cloze-like question from an instance.
 *
 * Replaces the words in the sentence with placeholder and concantenates
 * all words with semicolons into answer. It sorts the selection_ptr list
 * as a side effect.
 */
void
lr_splitter_context_from_selection (LrSplitter *self,
                                    GList **selection_ptr,
                                    gchar **context,
                                    gchar **answer,
                                    const gchar *placeholder)
{
  g_assert (selection_ptr != NULL);
  g_assert (*selection_ptr != NULL);

  *selection_ptr = g_list_sort (*selection_ptr, (GCompareFunc)compare_ranges);
  GList *selection = *selection_ptr;

  /* Find the first and the last words in the selection */
  lr_range_t *first = (lr_range_t *)selection->data, *last = (lr_range_t *)selection->data;

  for (GList *l = selection; l != NULL; l = l->next)
    {
      lr_range_t *range = (lr_range_t *)l->data;

      if (first->start > range->start)
        first = range;

      if (last->end < range->end)
        last = range;
    }

  const gchar *text = lr_text_get_text (self->text);

  /* Find the first separator before the first word and first after the last word */
  lr_range_t *start_sep = NULL;
  lr_range_t *end_sep = NULL;
  for (int i = 0; i < self->separators->len; i++)
    {
      lr_range_t *sep = &g_array_index (self->separators, lr_range_t, i);

      /* If the separator is before the word, set it as the start separator */
      if (sep->end <= first->start)
        start_sep = sep;

      /* If no end separator has been set, and this one is after the last word,
       * set it as the end separator, and also exit the loop */
      if (!end_sep && (sep->start >= last->end))
        {
          end_sep = sep;
          break;
        }
    }

  lr_range_t sentence_range;
  if (start_sep)
    sentence_range.start = start_sep->end;
  else
    sentence_range.start = 0;

  if (end_sep)
    sentence_range.end = end_sep->end;
  else
    sentence_range.end = strlen (text);

  GString *sentence_str =
    g_string_new_len (text + sentence_range.start, sentence_range.end - sentence_range.start);

  GString *answer_str = g_string_new (NULL);

  /* Substitute the words for the placeholder */
  /* When deleting a word and substituting it with the placeholder, the range offsets will be invalid.
   * Therefore, the following offset is updated as offset += strlen(placeholder) - strlen(word) */
  int placeholder_len = strlen (placeholder);
  int offset = 0;
  for (GList *l = selection; l != NULL; l = l->next)
    {
      lr_range_t *range = (lr_range_t *)l->data;

      g_string_erase (
        sentence_str, offset + range->start - sentence_range.start, range->end - range->start);
      g_string_insert (sentence_str, offset + range->start - sentence_range.start, placeholder);

      offset += placeholder_len - (range->end - range->start);

      if (answer_str->len)
        g_string_append_c (answer_str, ' ');

      g_string_append_len (answer_str, text + range->start, range->end - range->start);
    }

  *context = g_strstrip (g_string_free (sentence_str, FALSE));
  *answer = g_string_free (answer_str, FALSE);
}

