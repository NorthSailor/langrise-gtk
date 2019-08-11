#include "lr-language-switcher.h"
#include "lr-database.h"

struct _LrLanguageSwitcher
{
  GtkButton parent;

  GtkWidget *label;
  GtkWidget *expander_icon;

  GtkWidget *popover;
  GtkWidget *list_box;
};

G_DEFINE_TYPE (LrLanguageSwitcher, lr_language_switcher, GTK_TYPE_BUTTON)

static void
change_language_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  g_message ("Language changed");
}

static void
lr_language_switcher_init (LrLanguageSwitcher *self)
{
  GtkWidget *expander_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);

  self->label = gtk_label_new ("(None)");
  self->expander_icon =
    gtk_image_new_from_icon_name ("go-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

  gtk_container_add (GTK_CONTAINER (expander_box), self->label);
  gtk_container_add (GTK_CONTAINER (expander_box), self->expander_icon);

  gtk_container_add (GTK_CONTAINER (self), expander_box);

  self->popover = gtk_popover_new (GTK_WIDGET (self));
  /* Install the CSS class "langswitcher" for the popover */

  GtkStyleContext *context = gtk_widget_get_style_context (self->popover);
  gtk_style_context_add_class (context, "langswitcher");

  GtkWidget *lang_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add (GTK_CONTAINER (self->popover), lang_box);

  gtk_container_add (GTK_CONTAINER (lang_box), gtk_button_new_with_label ("Polish"));
  gtk_container_add (GTK_CONTAINER (lang_box), gtk_button_new_with_label ("Dutch"));
  gtk_container_add (GTK_CONTAINER (lang_box), gtk_button_new_with_label ("German"));

  gtk_widget_show_all (lang_box);

  /* Disable the controls until we have added some languages to choose from */
  // gtk_widget_set_sensitive (GTK_WIDGET (self), FALSE);
}

static void
lr_language_switcher_finalize (GObject *obj)
{
  G_OBJECT_CLASS (lr_language_switcher_parent_class)->finalize (obj);
}

static void
lr_language_switcher_clicked (GtkButton *button)
{
  LrLanguageSwitcher *self = LR_LANGUAGE_SWITCHER (button);
  gtk_popover_popup (GTK_POPOVER (self->popover));
}

static void
lr_language_switcher_class_init (LrLanguageSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_language_switcher_finalize;

  GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);
  button_class->clicked = lr_language_switcher_clicked;
}

GtkWidget *
lr_language_switcher_new ()
{
  return g_object_new (LR_TYPE_LANGUAGE_SWITCHER, NULL);
}

void
lr_language_switcher_set_language_list (LrLanguageSwitcher *self, GList *list, int active_id)
{
  g_return_if_fail (LR_IS_LANGUAGE_SWITCHER (self));
  /* TODO */
}

