int roman(char *s, size_t l, unsigned int n, int classic);
int arabic(const char *s);

/* Roman nerals only cover [0...3999] range */
#define ROMAN_MAX 3999
#define ROMAN_BUF 19 
