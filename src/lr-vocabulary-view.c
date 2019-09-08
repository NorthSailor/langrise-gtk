#include "lr-vocabulary-view.h"
#include "list-row-creators.h"
#include "export-text.h"

struct _LrVocabularyView
{
  GtkBox parent_instance;

  LrDatabase *db;
  LrLanguage *language;

  GtkWidget *exporter_box;

  GtkWidget *list_box;
  GListStore *text_store;

  GList *selected_rows;
};

enum
{
  PROP_0,
  PROP_DATABASE,
  PROP_LANGUAGE,
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
export_csv (GtkWidget *toplevel, LrDatabase *db, GList *items)
{
  lr_export_text (toplevel, //
                  db,
                  items,
                  "",
                  "\n",
                  ",",
                  "Sentence,Answer,Lemma,Translation,Note\n",
                  "",
                  "CSV Files",
                  "*.csv");
}

static void
export_tsv (GtkWidget *toplevel, LrDatabase *db, GList *items)
{
  lr_export_text (toplevel, //
                  db,
                  items,
                  "",
                  "\n",
                  "\t",
                  "\n",
                  "",
                  "TSV files",
                  "*.tsv");
}

static void
export_latex (GtkWidget *toplevel, LrDatabase *db, GList *items)
{
  lr_export_text (toplevel, //
                  db,
                  items,
                  "\\item{",
                  "}\n",
                  "}{",
                  "",
                  "",
                  "LaTeX source files",
                  "*.tex");
}

static void
export_pdf (GtkWidget *toplevel, LrDatabase *db, GList *items)
{
  GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (toplevel),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_CLOSE,
                                              "PDF exporting is not yet supported!");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

typedef struct
{
  const gchar *label;
  void (*export) (GtkWidget *toplevel, LrDatabase *db, GList *items);
} exporter_t;

static exporter_t exporters[] = {
  { "CSV", export_csv },
  { "TSV", export_tsv },
  { "LaTeX", export_latex },
  { "PDF", export_pdf },
};

typedef struct
{
  LrVocabularyView *self;
  exporter_t *exporter;
} callback_argument;

static void
export_cb (GtkButton *sender, callback_argument *args)
{
  LrVocabularyView *self = args->self;
  exporter_t *exporter = args->exporter;

  GList *selected_rows = gtk_list_box_get_selected_rows (GTK_LIST_BOX (self->list_box));
  GList *items = NULL;

  for (GList *l = selected_rows; l != NULL; l = l->next)
    {
      GtkListBoxRow *row = GTK_LIST_BOX_ROW (l->data);
      int index = gtk_list_box_row_get_index (row);
      LrText *text = g_list_model_get_item (G_LIST_MODEL (self->text_store), index);
      GList *text_items = lr_database_get_vocabulary_items_for_text (self->db, text);
      items = g_list_concat (items, text_items);
    }

  exporter->export(gtk_widget_get_toplevel (GTK_WIDGET (self)), self->db, items);

  g_list_free_full (items, (GDestroyNotify)lr_vocabulary_item_free);

  g_list_free (selected_rows);
}

static void
lr_vocabulary_view_constructed (GObject *obj)
{
  LrVocabularyView *self = LR_VOCABULARY_VIEW (obj);
  g_assert (LR_IS_DATABASE (self->db));

  int num_exporters = sizeof (exporters) / sizeof (exporter_t);
  for (int i = 0; i < num_exporters; i++)
    {
      GtkWidget *button = gtk_button_new_with_label (exporters[i].label);
      gtk_container_add (GTK_CONTAINER (self->exporter_box), button);

      callback_argument *arg = g_malloc (sizeof (callback_argument));
      arg->self = self;
      arg->exporter = &exporters[i];

      g_signal_connect_data (
        button, "clicked", (GCallback)export_cb, arg, (GClosureNotify)g_free, 0);
    }

  self->text_store = g_list_store_new (LR_TYPE_TEXT);
  gtk_list_box_bind_model (GTK_LIST_BOX (self->list_box),
                           G_LIST_MODEL (self->text_store),
                           (GtkListBoxCreateWidgetFunc)create_widget_for_text,
                           NULL,
                           NULL);
}

static void
lr_vocabulary_view_finalize (GObject *object)
{
  LrVocabularyView *self = LR_VOCABULARY_VIEW (object);

  g_list_free (self->selected_rows);
  g_clear_object (&self->text_store);

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
    case PROP_DATABASE:
      self->db = g_value_get_object (value);
      break;
    case PROP_LANGUAGE:
      lr_vocabulary_view_set_language (self, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
selection_changed_cb (GtkListBox *box, LrVocabularyView *self)
{
  g_list_free (self->selected_rows);

  self->selected_rows = gtk_list_box_get_selected_rows (box);

  gtk_widget_set_sensitive (self->exporter_box, self->selected_rows != NULL);
}

static void
lr_vocabulary_view_class_init (LrVocabularyViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->constructed = lr_vocabulary_view_constructed;
  object_class->finalize = lr_vocabulary_view_finalize;
  object_class->set_property = lr_vocabulary_view_set_property;

  obj_properties[PROP_DATABASE] = g_param_spec_object ("database",
                                                       "Database",
                                                       "The database",
                                                       LR_TYPE_DATABASE,
                                                       G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  obj_properties[PROP_LANGUAGE] = g_param_spec_object (
    "language", "language", "The language", LR_TYPE_LANGUAGE, G_PARAM_WRITABLE);


  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/langrise/Langrise/lr-vocabulary-view.ui");

  gtk_widget_class_bind_template_child (widget_class, LrVocabularyView, list_box);
  gtk_widget_class_bind_template_child (widget_class, LrVocabularyView, exporter_box);

  gtk_widget_class_bind_template_callback (widget_class, selection_changed_cb);
}

GtkWidget *
lr_vocabulary_view_new (LrDatabase *db)
{
  return g_object_new (LR_TYPE_VOCABULARY_VIEW, "database", db, NULL);
}

void
lr_vocabulary_view_set_language (LrVocabularyView *self, LrLanguage *language)
{
  self->language = language;

  lr_database_populate_texts (self->db, self->text_store, self->language);
  selection_changed_cb (GTK_LIST_BOX (self->list_box), self);
}

LrLanguage *
lr_vocabulary_view_get_language (LrVocabularyView *self)
{
  return self->language;
}

