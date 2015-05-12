/*
** This file contains the symbol handling routines.
**
** Date     Programmer     History
** -------- -------------- --------------------------------------------------
** 871220   P. Eriksson    Initial coding.
** 880914   P. Eriksson    Modified to fit ASM/6502
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbols.h"

#define FALSE  0
#define TRUE   1



static struct SYMBOL *symbol_list = NULL;  /* The list of symbols */



/*
 * This function adds one variable to the end of the list.
 * It will return a TRUE value if it could add the variable to the list.
 */
static int
add_symbol(char *name, ADDRESS value)
{
   struct SYMBOL  *tvar;
   struct SYMBOL **avar;

   tvar = symbol_list;
   avar = &symbol_list;



   /* Find end of list */
   while (tvar != NULL)
   {
     avar = &tvar->next;
     tvar = tvar->next;
   }

   /* Add symbol header to the list */
   if ( (tvar = (struct SYMBOL *) malloc(sizeof(struct SYMBOL))) == NULL)
      return FALSE;

   /* Initialize the header */
   tvar->next = NULL;
   if ((tvar->name = malloc(strlen(name)+1)) == NULL)
      return FALSE;
   tvar->value = value;

   /* Copy the symbol name into it */
   strcpy(tvar->name,name);

   /* Set the previous "next" field to point at this header */
   *avar = tvar;
   return TRUE;
}



/*
 * This function deletes a named variable.
 */
int
del_symbol(name)
     char *name;
{
   struct SYMBOL  *tvar;
   struct SYMBOL **avar;


   tvar = symbol_list;
   avar = &symbol_list;

   /* Locate the symbol (if it exists!) */
   while (tvar != NULL && strcmp(tvar->name,name) != NULL )
   {
      avar = &tvar->next;
      tvar = tvar->next;
   }

   if (tvar == NULL)
      return FALSE;

   /* Remove it from the list */
   *avar = tvar->next;
   free(tvar->name);
   free(tvar);

   return TRUE;
}



/*
 * This function returns the value of a named variable.
 */
int
get_symbol(char *name, ADDRESS *value)
{
   struct SYMBOL *tvar;


   tvar = symbol_list;

   /* Locate the symbol */
   while (tvar != NULL && strcmp(tvar->name,name) != NULL)
      tvar = tvar->next;

   if (tvar == NULL)
      return -1;
   else
   {
      *value = tvar->value;
      return 0;
   }
}



/*
 * This function set a symbol to a certain value.
 */
int
set_symbol(char *name, ADDRESS value)
{
   struct SYMBOL *tvar;


   tvar = symbol_list;

   /* Locate it */
   while (tvar != NULL && strcmp(tvar->name,name) != NULL)
      tvar = tvar->next;

   if (tvar == NULL)
      return add_symbol(name,value);
   else
   {
      tvar->value = value;
      return TRUE;
   }
}


/*
 * This function clears the complete list of symbols.
 */
int
clear_symbol()
{
   struct SYMBOL *tvar, *list;

   list = symbol_list;

   while (list != NULL)
   {
      tvar = list->next;
      free(list->name);
      free(list);
      list = tvar;
   }

   symbol_list = NULL;

   return TRUE;
}


