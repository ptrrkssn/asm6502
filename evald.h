#define EVAL$SHORT   4
#define EVAL$INT     5
#define EVAL$LONG    6
#define EVAL$FLOAT   7
#define EVAL$DOUBLE  8

#define SYNTAX_ERROR 0
#define UNBAL_PARENT 1
#define NO_EXPRE_PRE 2
#define UNDECLD_VARI 3

/* declare extern char errmess; */

typedef int number;


struct evarformat
{
  char *name;
  number equ;
  struct evarformat *next;
};


extern int
evald(char *expr, number *res, void (*errfunc)(int error));

