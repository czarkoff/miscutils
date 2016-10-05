#define BGROUP	01
#define BSKIP	02

#define DEFAULT_SYM "01"

char *binary(const uint8_t *src, size_t srclen, const char *sym, int flags);
