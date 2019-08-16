#include "lr-splitter.h"

struct _LrSplitter
{
  GObject parent_instance;

  LrText *text;

  GRegex *word_regex;

  GArray *words;
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

  self->word_regex = g_regex_new (word_regex_string, 0, 0, NULL);
  g_assert (self->word_regex != NULL);

  self->words = g_array_new (FALSE, FALSE, sizeof (lr_word_range_t));

  /* Split the text */
  GMatchInfo *match_info;
  g_regex_match (self->word_regex, lr_text_get_text (text), 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      lr_word_range_t range;
      g_match_info_fetch_pos (match_info, 0, &range.start, &range.end);

      g_array_append_val (self->words, range);

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

const lr_word_range_t *
lr_splitter_get_word_at_index (LrSplitter *self, int index)
{
  for (int i = 0; i < self->words->len; i++)
    {
      lr_word_range_t *range = &g_array_index (self->words, lr_word_range_t, i);
      if (index < range->start)
        return NULL;

      if (index <= range->end)
        return range;
    }
  return NULL;
}

