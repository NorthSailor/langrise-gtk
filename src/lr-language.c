#include "lr-language.h"

struct _LrLanguage
{
  GObject parent_instance;

  gint id;
  gchar *code;
  gchar *name;
};

G_DEFINE_TYPE (LrLanguage, lr_language, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_ID,
  PROP_CODE,
  PROP_NAME,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

static void
lr_language_init (LrLanguage *self)
{
}

static void
lr_language_finalize (GObject *obj)
{
  LrLanguage *self = LR_LANGUAGE (obj);

  g_free (self->code);
  g_free (self->name);

  G_OBJECT_CLASS (lr_language_parent_class)->finalize (obj);
}

static void
lr_language_set_property (GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)

{
  LrLanguage *self = LR_LANGUAGE (object);
  switch (property_id)
    {
    case PROP_ID:
      self->id = g_value_get_int (value);
      break;
    case PROP_CODE:
      g_free (self->code);
      self->code = g_value_dup_string (value);
      break;
    case PROP_NAME:
      g_free (self->name);
      self->name = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_language_class_init (LrLanguageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_language_finalize;
  object_class->set_property = lr_language_set_property;

  obj_properties[PROP_ID] = g_param_spec_int ("id",
                                              "ID",
                                              "The ID of the language",
                                              0,
                                              G_MAXINT,
                                              0,
                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

  obj_properties[PROP_CODE] = g_param_spec_string (
    "code", "Code", "The language code.", "", G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

  obj_properties[PROP_NAME] = g_param_spec_string (
    "name", "Name", "The name of the language", "", G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);


  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}

LrLanguage *
lr_language_new (int id, const gchar *code, const gchar *name)
{
  return g_object_new (LR_TYPE_LANGUAGE, "id", id, "code", code, "name", name, NULL);
}

int
lr_language_get_id (LrLanguage *self)
{
  return self->id;
}

gchar *
lr_language_get_code (LrLanguage *self)
{
  return self->code;
}

gchar *
lr_language_get_name (LrLanguage *self)
{
  return self->name;
}

