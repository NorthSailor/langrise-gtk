#include "lr-lemma-suggestion.h"

struct _LrLemmaSuggestion
{
  GObject parent_instance;

  /* Instance variables */
  gchar *lemma;
  gchar *note;
};

enum
{
  PROP_0,
  PROP_LEMMA,
  PROP_NOTE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrLemmaSuggestion, lr_lemma_suggestion, G_TYPE_OBJECT)

static void
lr_lemma_suggestion_init (LrLemmaSuggestion *self)
{
  /* Instance initialization */
}

static void
lr_lemma_suggestion_finalize (GObject *object)
{
  LrLemmaSuggestion *self = LR_LEMMA_SUGGESTION (object);

  g_free (self->lemma);
  g_free (self->note);

  G_OBJECT_CLASS (lr_lemma_suggestion_parent_class)->finalize (object);
}

static void
lr_lemma_suggestion_set_property (GObject *object,
                                  guint property_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
  LrLemmaSuggestion *self = LR_LEMMA_SUGGESTION (object);

  switch (property_id)
    {
    case PROP_LEMMA:
      g_free (self->lemma);
      self->lemma = g_value_dup_string (value);
      break;
    case PROP_NOTE:
      g_free (self->note);
      self->note = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_lemma_suggestion_get_property (GObject *object,
                                  guint property_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
  LrLemmaSuggestion *self = LR_LEMMA_SUGGESTION (object);

  switch (property_id)
    {
    case PROP_LEMMA:
      g_value_set_string (value, self->lemma);
      break;
    case PROP_NOTE:
      g_value_set_string (value, self->note);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_lemma_suggestion_class_init (LrLemmaSuggestionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_lemma_suggestion_finalize;
  object_class->set_property = lr_lemma_suggestion_set_property;
  object_class->get_property = lr_lemma_suggestion_get_property;

  obj_properties[PROP_LEMMA] = g_param_spec_string (
    "lemma", "Lemma", "The lemma", "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  obj_properties[PROP_NOTE] = g_param_spec_string ("note",
                                                   "Note",
                                                   "The note accompanying the suggestion",
                                                   "",
                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

LrLemmaSuggestion *
lr_lemma_suggestion_new (const gchar *lemma, const gchar *note)
{
  return g_object_new (LR_TYPE_LEMMA_SUGGESTION, "lemma", lemma, "note", note, NULL);
}

const gchar *
lr_lemma_suggestion_get_lemma (LrLemmaSuggestion *self)
{
  return self->lemma;
}

const gchar *
lr_lemma_suggestion_get_note (LrLemmaSuggestion *self)
{
  return self->note;
}

