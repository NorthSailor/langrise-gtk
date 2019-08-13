#include "lr-text.h"

struct _LrText
{
  GObject parent_instance;

  /* Instance variables */
  int id;
  LrLanguage *language;
  gchar *title;
  gchar *tags;
  gchar *text;
};

G_DEFINE_TYPE (LrText, lr_text, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_ID,
  PROP_LANGUAGE,
  PROP_TITLE,
  PROP_TAGS,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

static void
lr_text_init (LrText *self)
{
}

static void
lr_text_finalize (GObject *obj)
{
  LrText *self = LR_TEXT (obj);

  g_free (self->title);
  g_free (self->tags);
  g_free (self->text);

  g_clear_object (&self->language);
}

static void
lr_text_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)

{
  LrText *self = LR_TEXT (object);
  switch (property_id)
    {
    case PROP_ID:
      self->id = g_value_get_int (value);
      break;
    case PROP_LANGUAGE:
      g_clear_object (&self->language);
      self->language = LR_LANGUAGE (g_value_get_object (value));
      g_object_ref (self->language);
      break;
    case PROP_TITLE:
      lr_text_set_title (self, g_value_get_string (value));
      break;
    case PROP_TAGS:
      lr_text_set_tags (self, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_text_class_init (LrTextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_text_finalize;
  object_class->set_property = lr_text_set_property;

  obj_properties[PROP_ID] = g_param_spec_int (
    "id", "ID", "The ID of the text", 0, G_MAXINT, 0, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

  obj_properties[PROP_LANGUAGE] = g_param_spec_object ("language",
                                                       "Language",
                                                       "The text's language",
                                                       LR_TYPE_LANGUAGE,
                                                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

  obj_properties[PROP_TITLE] = g_param_spec_string (
    "title", "Title", "The text's title.", "", G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

  obj_properties[PROP_TAGS] = g_param_spec_string (
    "tags", "Tags", "The text's tags", "", G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);


  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

LrText *
lr_text_new (int id, LrLanguage *language, const gchar *title, const gchar *tags)
{
  return g_object_new (
    LR_TYPE_TEXT, "id", id, "language", language, "title", title, "tags", tags, NULL);
}

int
lr_text_get_id (LrText *self)
{
  return self->id;
}

int
lr_text_get_language_id (LrText *self)
{
  return lr_language_get_id (self->language);
}

const gchar *
lr_text_get_title (LrText *self)
{
  return self->title;
}

void
lr_text_set_title (LrText *self, const gchar *title)
{
  g_free (self->title);
  self->title = g_strdup (title);
}

const gchar *
lr_text_get_tags (LrText *self)
{
  return self->tags;
}

void
lr_text_set_tags (LrText *self, const gchar *tags)
{
  g_free (self->tags);
  self->tags = g_strdup (tags);
}

const gchar *
lr_text_get_text (LrText *self)
{
  g_assert (LR_IS_TEXT (self));
  return self->text;
}

void
lr_text_set_text (LrText *self, const gchar *text)
{
  g_assert (LR_IS_TEXT (self));

  g_free (self->text);
  self->text = g_strdup (text);
}

