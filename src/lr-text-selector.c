#include "lr-text-selector.h"
#include "lr-text.h"

struct _LrTextSelector
{
  GtkBox parent_instance;

  LrDatabase *db;

  /* ID of the current language, used for filtering queries */
  guint lang_id; /* -1 if none (edge case, only during initialization) */

  GtkWidget *title_label;
  GtkWidget *read_button;
  GtkWidget *edit_button;
  GtkWidget *delete_button;
  GtkWidget *list_box;

  GList *text_list;
};

G_DEFINE_TYPE (LrTextSelector, lr_text_selector, GTK_TYPE_BOX)

static void
new_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("New text");
}

static void
read_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("Read text");
}

static void
edit_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("Edit text");
}

static void
delete_text_cb (LrTextSelector *self, GtkWidget *button)
{
  g_message ("Delete text");
}

static void
populate_text_list (LrTextSelector *self)
{
  /* Free the previous list */
  g_list_free_full (self->text_list, (GDestroyNotify)lr_database_text_free);

  /* TODO Fill the list box with database query results */
  self->text_list = lr_database_get_texts (self->db, self->lang_id);


  for (GList *l = self->text_list; l != NULL; l = l->next)
    {
      lr_text_t *text = (lr_text_t *)l->data;
      g_message ("Found text (ID %d) '%s' with tags '%s'", text->id, text->title, text->tags);
    }
}

static void
lr_text_selector_init (LrTextSelector *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  self->db = NULL;
  self->lang_id = -1;

  gtk_list_box_set_placeholder (GTK_LIST_BOX (self->list_box), gtk_label_new ("No texts found"));
}

static void
lr_text_selector_finalize (GObject *obj)
{
  g_assert (LR_IS_TEXT_SELECTOR (obj));

  LrTextSelector *self = LR_TEXT_SELECTOR (obj);
  g_list_free_full (self->text_list, (GDestroyNotify)lr_database_text_free);
}

static void
lr_text_selector_class_init (LrTextSelectorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = lr_text_selector_finalize;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-text-selector.ui");

  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, title_label);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, read_button);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, edit_button);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, delete_button);
  gtk_widget_class_bind_template_child (widget_class, LrTextSelector, list_box);

  gtk_widget_class_bind_template_callback (widget_class, new_text_cb);
  gtk_widget_class_bind_template_callback (widget_class, read_text_cb);
  gtk_widget_class_bind_template_callback (widget_class, edit_text_cb);
  gtk_widget_class_bind_template_callback (widget_class, delete_text_cb);
}

GtkWidget *
lr_text_selector_new ()
{
  return g_object_new (LR_TYPE_TEXT_SELECTOR, NULL);
}

void
lr_text_selector_set_database (LrTextSelector *self, LrDatabase *db)
{
  g_assert (LR_IS_TEXT_SELECTOR (self));

  self->db = db;
}

void
lr_text_selector_set_language (LrTextSelector *self, lr_language_t *next_language)
{
  g_assert (LR_IS_TEXT_SELECTOR (self));
  g_assert (self->db != NULL); /* We need a database connection first */

  gchar *title = g_strdup_printf ("%s texts", next_language->name);
  gtk_label_set_text (GTK_LABEL (self->title_label), title);
  g_free (title);

  self->lang_id = next_language->id;

  populate_text_list (self);
}

