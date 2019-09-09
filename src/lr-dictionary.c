/* 
 * Langrise, expanding L2 vocabulary in context.
 * Copyright (C) 2019 Iason Barmparesos
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "lr-dictionary.h"
#include "lr-dictionary-provider.h"
#include "lr-goldendict-provider.h"

struct _LrDictionary
{
  GtkBox parent_instance;

  /* Instance variables */
  GtkWidget *provider_stack;
  GtkWidget *provider_combobox;
  GtkWidget *search_entry;

  /* Providers */
  GtkWidget *goldendict_provider;
};

G_DEFINE_TYPE (LrDictionary, lr_dictionary, GTK_TYPE_BOX)

static void
lr_dictionary_init (LrDictionary *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
provider_changed_cb (LrDictionary *self, GtkWidget *combo_box)
{
  const gchar *id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (combo_box));
  gtk_stack_set_visible_child_name (GTK_STACK (self->provider_stack), id);
}

static void
register_provider (LrDictionary *self, GtkWidget *provider, const gchar *id, const gchar *name)
{
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (self->provider_combobox), id, name);
  gtk_stack_add_named (GTK_STACK (self->provider_stack), provider, id);
}

static void
search_changed_cb (LrDictionary *self, GtkWidget *search_entry)
{
  /* Find the selected provider */
  GtkWidget *selected_provider = gtk_stack_get_visible_child (GTK_STACK (self->provider_stack));
  const gchar *term = gtk_entry_get_text (GTK_ENTRY (search_entry));

  lr_dictionary_provider_lookup (LR_DICTIONARY_PROVIDER (selected_provider), term, TRUE);
}

static void
lr_dictionary_constructed (GObject *object)
{
  LrDictionary *self = LR_DICTIONARY (object);

  self->goldendict_provider = lr_goldendict_provider_new ();

  /* Register dictionary providers */
  register_provider (self, self->goldendict_provider, "goldendict", "GoldenDict");

  /* Activate the first provider by default.
   * Could be saved in a GSettings file in the future
   */
  gtk_combo_box_set_active (GTK_COMBO_BOX (self->provider_combobox), 0);

  G_OBJECT_CLASS (lr_dictionary_parent_class)->constructed (object);
}

static void
lr_dictionary_class_init (LrDictionaryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->constructed = lr_dictionary_constructed;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-dictionary.ui");

  gtk_widget_class_bind_template_child (widget_class, LrDictionary, provider_combobox);
  gtk_widget_class_bind_template_child (widget_class, LrDictionary, provider_stack);
  gtk_widget_class_bind_template_child (widget_class, LrDictionary, search_entry);

  gtk_widget_class_bind_template_callback (widget_class, provider_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
}

GtkWidget *
lr_dictionary_new ()
{
  return g_object_new (LR_TYPE_DICTIONARY, NULL);
}

void
lr_dictionary_lookup (LrDictionary *self, const gchar *term)
{
  gtk_entry_set_text (GTK_ENTRY (self->search_entry), term);
  search_changed_cb (self, self->search_entry);
}
