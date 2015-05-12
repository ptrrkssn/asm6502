#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>

#include "evald.h"
#include "symbols.h"


/* Advanced C
 * Chapter 9 Expression Parsing and Evaluation
 *
 * Anders Furuhed adopted it for Prime/Primos
 * Floating point version by Anders Furuhed
 *
 *
 * 871119  Added function support
 * 880916  Removed floating point (not needed in ASM6502...) : Peter Eriksson
 */

/* Simple recursive descent parser for integer expressions

Operators

<, >, <>, =, <=, >=, &, |, ~, +, -, *, /, %, ^, <<, >>, <-, ->,
<- and -> (rotates) are not implemented yet

*/

#define DELIMETER     1
#define VARIABLE      2
#define NUMBER        3
#define FUNCTION      4

#define ROL ( ((x>>15) | (x<<1)) & 65535)
#define ROR ( ((x<<15) | (x>>1)) & 65535)


static char   *evaexpress;    /* Holds expression to be analyzed */
static char   token[80];
static char   tok_type;
static jmp_buf errenv;
static void   (*error)(int e);

char   errmess[100];        /* Global error message variable */

static void
level6(number *result);


static int
iswhite(int c)
{
  /* Look for spaces and tabs */
  if( c == ' ' || c == '\t')
    return 1;
  return 0;
}

static int
is_in( int ch, char *s)
{
  while( *s)
    if( *s++ == ch)
      return 1;
  return 0;
}

static int
isdelim(int c)
{
  if( is_in( c, " +-/*%^<>=[]~&|") || c == '\t' || c == '\n' || c == 0)
    return 1;
  return 0;
}


static int
issymbol(int chr)
{
   return isalpha(chr) || chr == '@' || chr == '_';
}


static int
get_int(char *str)
{
   int value;

   switch (str[0])
   {
      case '$':
         sscanf(str+1, "%x", &value);
         return value;

      case '\'':
         return str[1];

      case '0':
         switch (str[1])
         {
            case 'x':
            case 'X':
               sscanf(str+2, "%x", &value);
               return value;

            case 'o':
            case 'O':
               sscanf(str+2, "%o", &value);
               return value;
         }

      default:
         sscanf(str, "%d", &value);
   }

   return value;
}

static number
find_var(char *s)
{
  number temp;


  if( !issymbol(*s) )
    (*error)( UNBAL_PARENT);

  if (get_symbol(s, &temp) != -1)
     return temp;

  strcpy( errmess, token);
  (*error)(UNDECLD_VARI);
  return -1;
}


static void
get_token(void)
{
  register char *temp,
                c;

  tok_type= 0;
  temp= token;

  while( iswhite( *evaexpress))
    ++evaexpress;    /* Skip over white space */

  if( is_in( *evaexpress, "+-*/[]=&|<>%^~!"))
  {
    tok_type= DELIMETER;
    switch (*evaexpress)
    {
      case '-':
               if( *(evaexpress+1) == '>')
                 *temp++ = *evaexpress++;
               break;
      case '<':
               if( (c = *(evaexpress+1)) == '>' || c == '<' || c == '=' || c == '-')
                 *temp++ = *evaexpress++;
               break;
      case '>':
               if( (c = *(evaexpress+1)) == '>' || c == '=')
                 *temp++ = *evaexpress++;
               break;
    }
    /* Advance to next position */
    *temp++ = *evaexpress++;
  }
  else if( issymbol(*evaexpress))
  {
    while( !isdelim( *evaexpress))
      *temp++ = *evaexpress++;
/*
**    if( *evaexpress == '[')
**    {
**      evaexpress++;
**
**      tok_type= FUNCTION;
**      level10( tempval);
**      do_func();
*/
      tok_type= VARIABLE;
  }
  else if( isdigit( *evaexpress) || *evaexpress == '$')
  {
    while( !isdelim( *evaexpress))
      *temp++ = *evaexpress++;
    tok_type= NUMBER;
  }
  else if (evaexpress[0] == '\'' && evaexpress[2] == '\'')
  {
    *temp++ = *evaexpress++;
    *temp++ = *evaexpress++;
    evaexpress++;
    tok_type= NUMBER;
  }
  *temp= 0;
}


static void
primitive(number *result)
{

  switch (tok_type)
  {
    case VARIABLE:
                *result = find_var( token);
                return get_token();
    case NUMBER:
                *result = get_int( token);
                return get_token();
    default:
                (*error)(SYNTAX_ERROR);
  }
}

static void
arith(char *o, number *r, number *h)
{
  number t, ex;

  if( !strcmp( o, "-"))
    *r = *r - *h;
  else if( !strcmp( o, "+"))
    *r = *r + *h;
  else if( !strcmp( o, "*"))
    *r = *r * *h;
  else if( !strcmp( o, "/"))
    *r = (*r) / (*h);
  else if( !strcmp( o, "%"))
    *r = (int) *r % (int) *h;
  else if( !strcmp( o, "<>"))
    *r = (*r != *h);
  else if( !strcmp( o, "&"))
    *r = (int) *r & (int) *h;
  else if( !strcmp( o, "|"))
    *r = (int) *r | (int) *h;
  else if( !strcmp( o, "~"))
    *r = (int) *r ^ (int) *h;
  else if( !strcmp( o, "="))
    *r = (*r == *h);
  else if( !strcmp( o, "<"))
    *r = (*r < *h);
  else if( !strcmp( o, ">"))
    *r = (*r < *h);
  else if( !strcmp( o, "<="))
    *r = (*r <= *h);
  else if( !strcmp( o, ">="))
    *r = (*r >= *h);
  else if( !strcmp( o, "<<"))
    *r = (int) *r << (int) *h;
  else if( !strcmp( o, ">>"))
    *r = (int) *r >> (int) *h;
  else if( !strcmp( o, "^"))
  {
    ex = *r;
    if( *h == 0)
      *r = 1;
    else
    for( t= *h-1; t>0; --t)
      *r = (*r) * ex;
  }
}

static void
unary(char o, number *r)
{
  switch (o)
  {
    case '-':
              *r = -(*r);
              break;
    case '!':
              *r = ! (int) (*r);
              break;
  }

}


static void level10(number *result)
{
  if( (*token == '[') && (tok_type == DELIMETER))
  {
    get_token();
    level6( result);
    if( *token != ']')
      (*error)(UNBAL_PARENT);
    get_token();
  }
  else
    primitive( result);
}

static void
level9(number *result)
{
  register char op;

  op = 0;
  if( (tok_type == DELIMETER) && (*token == '+' || *token == '-' ||
      *token == '!'))
  {
    op = *token;
    get_token();
  }
  level10( result);
  if( op)
    unary( op, result);
}


static void
level8(number *result)
{
  number hold;

  level9( result);
  if( *token == '^')
  {
    get_token();
    level9( &hold);
    arith("^", result, &hold);
  }
}



static void
level7(number *result)
{
  char ope[3];
  number hold;

  level8( result);
  while( *token == '*' ||
         *token == '%' ||
         *token == '/' ||
         !strcmp( token, "->") ||
         !strcmp( token, "<-") ||
         !strcmp( token, ">>") ||
         !strcmp( token, "<<"))
  {
    strcpy( ope, token);
    get_token();
    level8(&hold);
    arith( ope, result, &hold);
  }
}


static void
level6(number *result)
{
  char op[3];
  number hold;

  level7( result);
  while( *token == '+' || *token == '-')
  {
    strcpy( op, token);
    get_token();
    level7( &hold);
    arith( op, result, &hold);
  }
}

static void
level5(number *result)
{
  char op[3];
  number hold;

  level6( result);
  while( !strcmp( token, "&") ||
         !strcmp( token, "|") ||
         !strcmp( token, "~"))
  {
    strcpy( op, token);
    get_token();
    level6( &hold);
    arith( op, result, &hold);
  }
}

static void
level4(number *result)
{
  char op[3];
  number hold;

  level5( result);
  while( !strcmp( token, "<") ||
         !strcmp( token, ">") ||
         !strcmp( token, "=") ||
         !strcmp( token, "<=") ||
         !strcmp( token, ">=") ||
         !strcmp( token, "<>"))
  {
    strcpy( op, token);
    get_token();
    level5( &hold);
    arith( op, result, &hold);
  }
}


static int
get_exp(number *result)
{
  get_token();
  if( !*token)
  {
    (*error)(NO_EXPRE_PRE);
    return -1;
  }
  level4( result);
  return *result;
}

#if 0
static void
putback(void)    /* Return a token to its resting place */
{
  char *t;

  t= token;
  for(; *t; t++)
    evaexpress--;
}
#endif

static void
serror(int error)
{
  static char *e[]=
  {
    "Syntax Error",
    "Unbalanced Parentheses",
    "No Expression Present",
    "Variable undeclared"
  };

  printf("\n*** %s (EVALD)\n", e[error]);
  longjmp(errenv,1);
}




/*
 *  <expr> is the expression to be parsed
 *  The result is placed in <res>
 *  The error messages is sent to <errfunc>. If it's NULL, serror is used
 */
int
evald(char *expr, number *res, void (*errfunc)(int error))
{
   evaexpress = expr;

   /* Set an error-function */
   if( errfunc == NULL)
     error = &serror;
   else
     error = errfunc;

   if (setjmp(errenv))
      return -1;
   get_exp(res);
   return 0;
}
