#include "lr-vocabulary-view.h"

struct _LrVocabularyView
{
  GtkBox parent_instance;

  /* Instance variables */
};

enum
{
  PROP_0,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE (LrVocabularyView, lr_vocabulary_view, GTK_TYPE_BOX)

static void
lr_vocabulary_view_init (LrVocabularyView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
lr_vocabulary_view_finalize (GObject *object)
{
  LrVocabularyView *self = LR_VOCABULARY_VIEW (object);

  G_OBJECT_CLASS (lr_vocabulary_view_parent_class)->finalize (object);
}

static void
lr_vocabulary_view_set_property (GObject *object,
                                 guint property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  LrVocabularyView *self = LR_VOCABULARY_VIEW (object);

  switch (property_id)
    {
    /* Add additional properties here */
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_vocabulary_view_get_property (GObject *object,
                                 guint property_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  LrVocabularyView *self = LR_VOCABULARY_VIEW (object);

  switch (property_id)
    {
    /* Add additional properties here */
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
lr_vocabulary_view_class_init (LrVocabularyViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_vocabulary_view_finalize;
  object_class->set_property = lr_vocabulary_view_set_property;
  object_class->get_property = lr_vocabulary_view_get_property;

  /* Add additional properties to obj_properties here */

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-vocabulary-view.ui");
}

GtkWidget *
lr_vocabulary_view_new ()
{
  return g_object_new (LR_TYPE_VOCABULARY_VIEW, NULL);
}
