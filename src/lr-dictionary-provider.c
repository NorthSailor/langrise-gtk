#include "lr-dictionary-provider.h"

G_DEFINE_INTERFACE (LrDictionaryProvider, lr_dictionary_provider, G_TYPE_OBJECT)

static void
lr_dictionary_provider_default_init (LrDictionaryProviderInterface *iface)
{
  /* Add signals and properties here */
}

void
lr_dictionary_provider_lookup (LrDictionaryProvider *self,
                               const gchar *word,
                               gboolean editing_finished)
{
  LrDictionaryProviderInterface *iface;

  g_return_if_fail (LR_IS_DICTIONARY_PROVIDER (self));

  iface = LR_DICTIONARY_PROVIDER_GET_IFACE (self);
  g_return_if_fail (iface->lookup != NULL);

  iface->lookup (self, word, editing_finished);
}
