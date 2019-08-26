#ifndef _language_presets_h
#define _language_presets_h

typedef struct
{
  const char *name;
  const char *code;
  const char *regex;
} lr_language_preset_t;

/* A useful online tool to convert the UTF-8 regexes to escaped C:
 * https://www.freeformatter.com/java-dotnet-escape.html#ad-output
 */

// clang-format off
static lr_language_preset_t presets[] = {
  { "Dutch", "nl", "[a-zA-Z\u00E0\u00E2\u00E4\u00F4\u00F3\u00E9\u00E8\u00EB\u00EA\u00EF\u00EE\u00F9\u00FB\u00FC\u00FF\u00C0\u00C2\u00C4\u00D4\u00D3\u00C9\u00C8\u00CB\u00CA\u00CF\u00CE\u0178\u00D9\u00DB\u00DC]+" },
  { "English", "en", "[a-zA-Z]+" },
  { "French", "fr", "[a-zA-Z\u00E0\u00E2\u00E4\u00F4\u00E9\u00E8\u00EB\u00EA\u00EF\u00EE\u00E7\u00F9\u00FB\u00FC\u00FF\u00E6\u0153\u00C0\u00C2\u00C4\u00D4\u00C9\u00C8\u00CB\u00CA\u00CF\u00CE\u0178\u00C7\u00D9\u00DB\u00DC\u00C6\u0152]+" },
  { "German", "de", "[a-zA-Z\u00E4\u00F6\u00FC\u00DF\u00C4\u00D6\u00DC\u1E9E]+" },
  { "Italian", "it", "[a-zA-Z\u00E0\u00E8\u00E9\u00EC\u00ED\u00EE\u00F2\u00F3\u00F9\u00FA\u00C0\u00C8\u00C9\u00CC\u00CD\u00CE\u00D2\u00D3\u00D9\u00DA]+" },
  { "Norwegian", "no", "[a-zA-Z\u00E6\u00F8\u00E5\u00C6\u00D8\u00C5]+" },
  { "Polish", "pl", "[a-zA-Z\u0105\u0107\u0119\u0142\u0144\u00F3\u015B\u017A\u017C\u0104\u0106\u0118\u0141\u0143\u00D3\u015A\u0179\u017B]+" },
  { "Spanish", "es", "[a-zA-Z\u00E1\u00E9\u00ED\u00F1\u00F3\u00FA\u00FC\u00C1\u00C9\u00CD\u00D1\u00D3\u00DA\u00DC]+"},
};

#endif
