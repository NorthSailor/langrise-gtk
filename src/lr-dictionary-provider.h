#ifndef _lr_dictionary_provider
#define _lr_dictionary_provider

#include <glib-object.h>

G_BEGIN_DECLS

#define LR_TYPE_DICTIONARY_PROVIDER lr_dictionary_provider_get_type ()
G_DECLARE_INTERFACE (LrDictionaryProvider, lr_dictionary_provider, LR, DICTIONARY_PROVIDER, GObject)

struct _LrDictionaryProviderInterface
{
  GTypeInterface parent_iface;

  void (*lookup) (LrDictionaryProvider *self, const gchar *term, gboolean editing_finished);
};

void lr_dictionary_provider_lookup (LrDictionaryProvider *self,
                                    const gchar *term,
                                    gboolean editing_finished);

G_END_DECLS

#endif
