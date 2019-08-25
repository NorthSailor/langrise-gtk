#include "lr-lemma-instance.h"

struct _LrLemmaInstance
{
  GObject parent_instance;

  int id;
  int lemma_id;
  LrText *text;
  gchar *words;
  gchar *note;
};

enum
{
  PROP_0,
  PROP_ID,
  PROP_LEMMA_ID,
  PROP_TEXT,
  PROP_WORDS,
  PROP_NOTE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrLemmaInstance, lr_lemma_instance, G_TYPE_OBJECT)

static void
lr_lemma_instance_init (LrLemmaInstance *self)
{
  /* Instance initialization */
}

static void
lr_lemma_instance_finalize (GObject *object)
{
  LrLemmaInstance *self = LR_LEMMA_INSTANCE (object);

  G_OBJECT_CLASS (lr_lemma_instance_parent_class)->finalize (object);
}

static void
lr_lemma_instance_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  LrLemmaInstance *self = LR_LEMMA_INSTANCE (object);

  switch (property_id)
    {
    case PROP_ID:
      lr_lemma_instance_set_id (self, g_value_get_int (value));
      break;
    case PROP_LEMMA_ID:
      lr_lemma_instance_set_lemma_id (self, g_value_get_int (value));
      break;
    case PROP_TEXT:
      lr_lemma_instance_set_text (self, g_value_get_object (value));
      break;
    case PROP_WORDS:
      lr_lemma_instance_set_words (self, g_value_get_string (value));
      break;
    case PROP_NOTE:
      lr_lemma_instance_set_note (self, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_lemma_instance_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  LrLemmaInstance *self = LR_LEMMA_INSTANCE (object);

  switch (property_id)
    {
    case PROP_ID:
      g_value_set_int (value, lr_lemma_instance_get_id (self));
      break;
    case PROP_LEMMA_ID:
      g_value_set_int (value, lr_lemma_instance_get_lemma_id (self));
      break;
    case PROP_TEXT:
      g_value_set_object (value, lr_lemma_instance_get_text (self));
      break;
    case PROP_WORDS:
      g_value_set_string (value, lr_lemma_instance_get_words (self));
      break;
    case PROP_NOTE:
      g_value_set_string (value, lr_lemma_instance_get_note (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_lemma_instance_class_init (LrLemmaInstanceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_lemma_instance_finalize;
  object_class->set_property = lr_lemma_instance_set_property;
  object_class->get_property = lr_lemma_instance_get_property;

  obj_properties[PROP_ID] =
    g_param_spec_int ("id", "ID", "The ID", -1, G_MAXINT, -1, G_PARAM_READWRITE);
  obj_properties[PROP_LEMMA_ID] =
    g_param_spec_int ("lemma-id", "lemma-id", "The lemma-id", -1, G_MAXINT, -1, G_PARAM_READWRITE);
  obj_properties[PROP_TEXT] =
    g_param_spec_object ("text", "text", "The text", LR_TYPE_TEXT, G_PARAM_READWRITE);
  obj_properties[PROP_WORDS] =
    g_param_spec_string ("words", "words", "The words", "", G_PARAM_READWRITE);
  obj_properties[PROP_NOTE] =
    g_param_spec_string ("note", "note", "The note", "", G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

LrLemmaInstance *
lr_lemma_instance_new (int id, int lemma_id, LrText *text, const gchar *words, const gchar *note)
{
  return g_object_new (LR_TYPE_LEMMA_INSTANCE,
                       "id",
                       id,
                       "lemma-id",
                       lemma_id,
                       "text",
                       text,
                       "words",
                       words,
                       "note",
                       note,
                       NULL);
}

void
lr_lemma_instance_set_id (LrLemmaInstance *self, int id)
{
  self->id = id;
}

int
lr_lemma_instance_get_id (LrLemmaInstance *self)
{
  return self->id;
}

void
lr_lemma_instance_set_lemma_id (LrLemmaInstance *self, int lemma_id)
{
  self->lemma_id = lemma_id;
}

int
lr_lemma_instance_get_lemma_id (LrLemmaInstance *self)
{
  return self->lemma_id;
}

void
lr_lemma_instance_set_text (LrLemmaInstance *self, LrText *text)
{
  self->text = text;
}

LrText *
lr_lemma_instance_get_text (LrLemmaInstance *self)
{
  return self->text;
}

void
lr_lemma_instance_set_words (LrLemmaInstance *self, const gchar *words)
{
  g_free (self->words);
  self->words = g_strdup (words);
}

const gchar *
lr_lemma_instance_get_words (LrLemmaInstance *self)
{
  return self->words;
}

void
lr_lemma_instance_set_note (LrLemmaInstance *self, const gchar *note)
{
  g_free (self->note);
  self->note = g_strdup (note);
}

const gchar *
lr_lemma_instance_get_note (LrLemmaInstance *self)
{
  return self->note;
}

