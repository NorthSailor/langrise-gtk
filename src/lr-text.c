#include "lr-text.h"

struct _LrText
{
  GObject parent_instance;

  /* Instance variables */
};

G_DEFINE_TYPE (LrText, lr_text, G_TYPE_OBJECT)

static void
lr_text_init (LrText *self)
{
}

static void
lr_text_finalize (GObject *obj)
{
  LrText *self = LR_TEXT (obj);
  g_message ("Text finalized");
}

static void
lr_text_class_init (LrTextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_text_finalize;
}

LrText *
lr_text_new ()
{
  return g_object_new (LR_TYPE_TEXT, NULL);
}
