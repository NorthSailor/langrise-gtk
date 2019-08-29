#include "lr-lemmatizer.h"

G_DEFINE_TYPE (LrLemmatizer, lr_lemmatizer, G_TYPE_OBJECT)

static void
lr_lemmatizer_init (LrLemmatizer *self)
{
  /* Instance initialization */
}

static gchar *
base_lemmatizer_suggestions (LrLemmatizer *self,
                             GListStore *store,
                             const char *text,
                             GList *selection)
{
  return g_strdup ("No lemma suggestions\navailable for this language.");
  return NULL;
}

static void
lr_lemmatizer_class_init (LrLemmatizerClass *klass)
{
  klass->populate_suggestions = base_lemmatizer_suggestions;
}

gchar *
lr_lemmatizer_populate_suggestions (LrLemmatizer *self,
                                    GListStore *store,
                                    const char *text,
                                    GList *selection)
{
  g_list_store_remove_all (store);

  return LR_LEMMATIZER_GET_CLASS (self)->populate_suggestions (self, store, text, selection);
}

#include "lr-db-lemmatizer.h"

LrLemmatizer *
lr_lemmatizer_new_for_language (const gchar *code)
{
  if (g_strcmp0 (code, "pl") == 0)
    return lr_db_lemmatizer_new ("pl.lemma");
  else
    return g_object_new (LR_TYPE_LEMMATIZER, NULL);
}
