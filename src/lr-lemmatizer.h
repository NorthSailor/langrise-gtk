#ifndef _lr_lemmatizer_h
#define _lr_lemmatizer_h

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define LR_TYPE_LEMMATIZER (lr_lemmatizer_get_type ())
G_DECLARE_DERIVABLE_TYPE (LrLemmatizer, lr_lemmatizer, LR, LEMMATIZER, GObject)

struct _LrLemmatizerClass
{
  GObjectClass parent_class;

  gchar *(*populate_suggestions) (LrLemmatizer *self,
                                  GListStore *store,
                                  const char *text,
                                  GList *selection);

  /* No need to add padding since this class is not exposed through an ABI */
};

/* Populate the given GListStore with suggestions, and possible return a
 * message to be displayed to the user.
 */
gchar *lr_lemmatizer_populate_suggestions (LrLemmatizer *self,
                                           GListStore *store,
                                           const char *text,
                                           GList *selection);

LrLemmatizer *lr_lemmatizer_new_for_language (const gchar *code);

G_END_DECLS

#endif /* _lr_lemmatizer_h */
