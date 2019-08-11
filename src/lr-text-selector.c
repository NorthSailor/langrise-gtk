#include "lr-text-selector.h"

struct _LrTextSelector
{
  GtkBox parent_instance;
  /* */
};

G_DEFINE_TYPE (LrTextSelector, lr_text_selector, GTK_TYPE_BOX)

static void
lr_text_selector_init (LrTextSelector *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
lr_text_selector_class_init (LrTextSelectorClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-text-selector.ui");
}

GtkWidget *
lr_text_selector_new ()
{
  return g_object_new (LR_TYPE_TEXT_SELECTOR, NULL);
}

