/* symbols.h */

typedef int ADDRESS;

struct SYMBOL
{
    struct SYMBOL *next;
    char          *name;
    ADDRESS       value;
};


extern int
set_symbol(char *name, ADDRESS value);
     
extern int
get_symbol(char *name, ADDRESS *value);
