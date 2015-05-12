/*
** ASM6502.C                                      A 6502 processor assembler.
**
**
** Date     Programmer     History
** -------- -------------- --------------------------------------------------
** 880915   P. Eriksson    Initial coding.
** 150512   P. Eriksson    Modernized the code a bit...
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <malloc.h>
#include <string.h>

#include "admodes.h"
#include "opcodes.h"
#include "evald.h"
#include "symbols.h"


#define LSIZE        32

#define CHECK        0
#define NOCHECK      1

#define NOSPCS       0
#define ONESPC       1

#define NEXT   0
#define END    1
#define ERROR  2

#define NOT_SET      -1



typedef struct FE
{
   struct FE * Next;
   char        Expression[129];
   int         Address;
   int         Storage;
   int         Line;
} ForwardEntry;


ForwardEntry *    FLtop = NULL,
             **   FLptr = &FLtop;

int      current_address = 0;
int      origin_address  = NOT_SET;
char *   memory_image    = NULL;
int      memory_length   = 0;


jmp_buf  error_env;
jmp_buf  undecld_symbol;

int      errcount          = 0;
int      lineno            = 0;
char     current_line[256];
int      linepos;
int      insource          = 0;

int      uc_labels         = 0;
int      hbitchar          = 0;

FILE *   infile;
FILE *   outfile;
FILE *   listfile;
FILE *   entryfile;
FILE *   externfile;


void
display_source(void)
{
   if (listfile && insource)
   {
      while (linepos < 20)
      {
         fputc(' ', listfile);
         linepos++;
      }

      fprintf(listfile, "%5d   %s", lineno, current_line);
   }
}

void
error(char *errmsg)
{
   display_source();
   printf("*** Error #%d, at line #%d is: %s\n", ++errcount, lineno, errmsg);
   if (listfile != NULL && listfile != stdout)
      fprintf(listfile, "*** Error #%d: %s\n", errcount, errmsg);
   longjmp(error_env, 1);
}


void
strupr(char *str)
{
  while (*str)
  {
    *str = toupper(*str);
    str++;
  }
}

void
strtrim(char *str, int spaces)
{
   char local[256], *ptr;

   strcpy(local, str);
   ptr = local;

   while (*ptr)
   {
      if (isspace(*ptr))
      {
         while (isspace(*ptr))
            ptr++;
         if (*ptr && *ptr != ';' && spaces)
            *str++ = ' ';
      }

      if (*ptr == ';')
         break;

      if (*ptr == '"')
      {
         *str++ = *ptr++;
         while (*ptr && (*str++ = *ptr++) != '"')
            ;
      }

      while (*ptr && !isspace(*ptr))
         *str++ = toupper(*ptr++);
   }

   *str = '\0';
}




void
everr_handler(int ecode)
{
   switch (ecode)
   {
      case SYNTAX_ERROR:
         error("syntax error in expression");

      case UNBAL_PARENT:
         error("unbalanced parenthesis in expression");

      case NO_EXPRE_PRE:
         error("the expression was missing");

      case UNDECLD_VARI:
         longjmp(undecld_symbol, 1);

      default:
         error("expression evaluation internal error");
   }
}


int
get_value(char *expression, int current_address, int *value)
{
   *value = -1;

   if (setjmp(undecld_symbol))
      return -1;

   set_symbol("@", current_address);

   evald(expression, value, everr_handler);

   return 0;
}



int
issymname(char *label)
{
   if (!isalpha(*label) && *label != '_')
      return 0;
   while (*++label)
      if (!isalnum(*label) &&  *label != '_')
         return 0;

   return 1;
}

void
dcl_entry(char *name, int address)
{
   if (entryfile != NULL)
      fprintf(entryfile, "%s = $%04X\n", name, address);
}



int
set_label(char *name, int value, int checkflag)
{
   int length, temp;


   if ((length = strlen(name)) == 0)
      error("internal error - null symbol name in set_label()");

   if (name[length - 1] == ':')
   {
      name[length - 1] = '\0';

      if (checkflag)
      {
         /*
         ** Test for multiple symbol definitions
         */
         if (get_symbol(name, &temp) != -1)
            error("symbol already defined");
      }

      dcl_entry(name, value);
   }

   if (checkflag)
   {
      /*
      ** Test for multiple symbol definitions
      */
      if (get_symbol(name, &temp) != -1)
         error("symbol already defined");
   }

   return set_symbol(name, value);
}




void
output_byte(int byte)
{
   if (listfile)
   {
      if (byte < 0)
         fprintf(listfile, "?? ");
      else
         fprintf(listfile, "%02X ", byte & 0xFF);
      linepos += 3;
   }

   if (origin_address == NOT_SET)
      origin_address = 0;

   if (current_address >= origin_address + memory_length)
   {
      if (memory_image == NULL)
      {
         if ((memory_image = (char *) malloc(memory_length = 200)) == NULL)
         {
            perror("error allocating memory");
            exit(1);
         }
      }
      else
      {
         if ((memory_image =
            (char *) realloc(memory_image, memory_length += 200)) == NULL)
         {
            perror("error allocating memory");
            exit(1);
         }
      }
   }

   memory_image[current_address++ - origin_address] = byte;
   if (current_address > 0xFFFF)
      error("object code storage int overflow");
}



void
forward_dcl(char *expression, int current_address, int storage)
{
   if (FLtop == NULL)
      FLptr = &FLtop;
   else
      FLptr = & ((*FLptr)->Next);

   if ((*FLptr = (ForwardEntry *) malloc(sizeof(ForwardEntry))) == NULL)
   {
      perror("error allocating memory");
      exit(1);
   }

   strcpy( (*FLptr)->Expression, expression );
   (*FLptr)->Address  = current_address;
   (*FLptr)->Storage = storage;
   (*FLptr)->Line = lineno;
   (*FLptr)->Next = NULL;
}


void
define_bytes(char *bytes)
{
   char *ptr, byteexpression[128];
   int value, length;


   while (*bytes)
      if (*bytes == '"')
      {
         while (*++bytes && *bytes != '"')
	     output_byte(*bytes & ((hbitchar?0xFF:0x7F) | (hbitchar?0x80:0x00)));
         if (*bytes == '"')
         {
            if (*++bytes != '\0' && *bytes != ',')
               error("missing comma (',') character");
            if (*bytes == ',')
               ++bytes;
         }
      }
      else
      {
         for (ptr = bytes; *ptr && *ptr != ','; ptr++)
            ;

         if ((length = ptr - bytes) == 0)
            output_byte(0);
         else
         {
            strncpy(byteexpression, bytes, length);
            byteexpression[length] = '\0';
            if (get_value(byteexpression, current_address, &value) == -1)
               forward_dcl(byteexpression, current_address, AM_ZP);
            output_byte(value);
         }
         if (*ptr)
            ptr++;
         bytes = ptr;
      }
}

void
define_words(char *words)
{
   char *ptr, wordexpression[128];
   int value, length;


   while (*words)
   {
      for (ptr = words; *ptr && *ptr != ','; ptr++)
         ;

      if ((length = ptr - words) == 0)
      {
         output_byte(0);
         output_byte(0);
      }
      else
      {
         strncpy(wordexpression, words, length);
         wordexpression[length] = '\0';
         if (get_value(wordexpression, current_address, &value) == -1)
            forward_dcl(wordexpression, current_address, AM_AB);

         output_byte(value);
         output_byte(value < 0 ? -1 : (value >> 8));
      }
      if (*ptr)
         ptr++;
      words = ptr;
   }
}


int
find_match(char *test, char *oper)
{
   int loop;
   char nlc;


   strcat(test, "\n");
   nlc = 0;
   for (loop = 0; amms[loop].match; loop++)
   {
      sscanf(test, amms[loop].match, oper, &nlc);
      if (nlc != '\n')
         continue;
      else
         return amms[loop].admode;
   }

   return -1;
}




int
find_admode(char *mode, int *operand, instruction *OP)
{
   char operstring[128];
   int length, admode;


   if (!(length = strlen(mode)))
      return AM_IMP;

   if (strcmp(mode, "A") == 0)
      return AM_ACC;


   switch (admode = find_match(mode, operstring))
   {
      case AM_IMM:
         if (get_value(operstring, current_address, operand) == -1)
            forward_dcl(operstring, current_address+1, AM_ZP);

         return AM_IMM;

      case AM_RAZ:
         if (get_value(operstring, current_address, operand) == -1)
         {
            forward_dcl(operstring, current_address+1,
               ((OP->admodes & B(AM_REL)) ? AM_REL : AM_AB));

            if ((OP->admodes & B(AM_REL)))
               return AM_REL;
            else
               return AM_AB;
         }
         if (OP->admodes & B(AM_REL))
         {
            *operand = *operand - current_address - 2;
            if (*operand < -128 || *operand > 127)
               error("relative branch out of range");

            *operand &= 0xFF;
            return AM_REL;
         }

         if (*operand > 255 || *operand < 0)
            return AM_AB;
         else
            return AM_ZP;


      case AM_AZX:
         if (get_value(operstring, current_address, operand) == -1)
            forward_dcl(operstring, current_address+1, AM_AB);

         if (*operand > 255 || *operand < 0)
            return AM_ABX;
         else
            return AM_ZPX;


      case AM_AZY:
         if (get_value(operstring, current_address, operand) == -1)
            forward_dcl(operstring, current_address+1, AM_AB);

         if (*operand > 255 || *operand < 0)
            return AM_ABY;
         else
            return AM_ZPY;

      case -1:
         error("unknown addressing mode");

      default:
         if (get_value(operstring, current_address, operand) == -1)
            forward_dcl(operstring, current_address+1, AM_AB);

         return admode;
   }

   return -1;
}


instruction *
find_instruction(char *name)
{
   int loop;

   for (loop = 0; idt[loop].name && strcmp(idt[loop].name, name); loop++)
      ;
   if (idt[loop].name)
      return &idt[loop];

   error("unknown instruction mnemonic");
   return NULL;
}

int
decode_instruction(char *name, char *mode)
{
   instruction *OP;
   int admode, operand;


   /*
   ** Test for "ORG" statement
   */
   if (strcmp(name, "ORG") == 0)
   {
      if (origin_address != -1)
         error("can't handle multiple ORG statements");

      if (get_value(mode, current_address, &origin_address) == -1)
         error("undefined symbol in ORG statement");

      current_address = origin_address;

      return NEXT;
   }


   /*
   ** Test for "END" statement
   */
   if (strcmp(name, "END") == 0)
      return END;


   /*
   ** Test for "DFB" and "DB" statements
   */
   if (strcmp(name, "DFB") == 0 || strcmp(name, "DB") == 0)
   {
      define_bytes(mode);
      return NEXT;
   }


   /*
   ** Test for "DFW" and "DW" statements
   */
   if (strcmp(name, "DFW") == 0 || strcmp(name, "DW") == 0)
   {
      define_words(mode);
      return NEXT;
   }


   /*
   ** Test for a normal CPU opcode
   */
   if ((OP = find_instruction(name)) == NULL)
      return ERROR;

   if ((admode = find_admode(mode, &operand, OP)) == -1)
      return ERROR;

   if (admode == AM_ZP && (OP->admodes & B(AM_ZP)) == 0)
      admode = AM_AB;

   if ((OP->admodes & B(admode)) == 0)
      error("invalid addressing mode for this instruction");

   output_byte(OP->opcode[admode]);

   switch (admode_length[admode])
   {
      case 1:
         output_byte(operand);
         break;

      case 2:
         output_byte(operand);
         output_byte(operand < 0 ? -1 : (operand >> 8));
         break;
   }

   return NEXT;
}




void
solve_forward(void)
{
   ForwardEntry *FE;
   int Value;


   for (FE = FLtop; FE; FE = FE->Next)
   {
      if (get_value(FE->Expression, FE->Address, &Value) == -1)
      {
         fprintf(stderr, "undefined symbol at line %d:%s\n",
            FE->Line,
            FE->Expression);
      }
      else
      {
         switch (FE->Storage)
         {
            case AM_REL:
               if ((Value -= FE->Address + 1) < -128 || Value > 127)
		   fprintf(stderr, "relative branch out of range at line %d\n",
			   FE->Line);
	       
               memory_image[FE->Address-origin_address] = Value & 0xFF;
               break;

            case AM_ZP:
               memory_image[FE->Address-origin_address] = Value & 255;
               break;

            case AM_AB:
               memory_image[FE->Address+0-origin_address] = Value & 255;
               memory_image[FE->Address+1-origin_address] = (Value >> 8) & 0xFF;
         }
      }
   }
}





void
write_outfile(FILE *outfile)
{
   int counter;
   int loop, length, checksum, chr;

   for (counter = origin_address; counter < current_address; counter += LSIZE)
   {
      fprintf(outfile, ":%02X%04X00",
         length = (counter + LSIZE < current_address ?
            LSIZE : current_address - counter),
         counter);
      checksum = length + (current_address & 255) + (current_address >> 8);
      for (loop = 0; loop < length; loop++)
      {
         fprintf(outfile, "%02X",
            chr = memory_image[counter - origin_address + loop]);
         checksum += chr;
      }
      fprintf(outfile, "%02X\n", (256 - checksum)&255);
   }

   fprintf(outfile, ":00000001FF\n");
}










int
main(int argc, char **argv)
{
   char     temp[129];
   char     line[256];
   char     label[32];
   char     name[32];
   char     mode[128];
   char *   ptr;
   int      endflag;
   int      value;
   int      length;


   puts("[ASM/6502 version 1.10]");

   if (argc < 2)
   {
      infile     = stdin;
      listfile   = stdout;
      outfile    = stdout;
      entryfile  = stdout;
      externfile = stdout;
   }
   else
   {
      /*
      ** ASM source file
      */
      if ((infile = fopen(argv[1], "r")) == NULL)
      {
         strcpy(temp, argv[1]);
         strcat(temp, ".asm");
         if ((infile = fopen(temp, "r")) == NULL)
         {
            perror("Error opening source file");
            exit(1);
         }
      }


      /*
      ** HEX output file
      */
      strcpy(temp, argv[1]);
      if ((ptr = strchr(temp, '.')) != NULL)
         strcpy(ptr, ".ith");
      else
         strcat(temp, ".ith");

      if ((outfile = fopen(temp, "w")) == NULL)
      {
         perror("Error opening output file");
         exit(1);
      }


      /*
      ** ENTrypoints file
      */
      strcpy(temp, argv[1]);
      if ((ptr = strchr(temp, '.')) != NULL)
         strcpy(ptr, ".ent");
      else
         strcat(temp, ".ent");

      if ((entryfile = fopen(temp, "w")) == NULL)
      {
         perror("Error opening entrypoint file");
         exit(1);
      }

      /*
      ** EXTernal declarations file.
      */
      strcpy(temp, argv[1]);
      if ((ptr = strchr(temp, '.')) != NULL)
         strcpy(ptr, ".ext");
      else
         strcat(temp, ".ext");

      if ((externfile = fopen(temp, "w")) == NULL)
      {
         perror("Error opening external declarations file");
         exit(1);
      }

      /*
      ** LISTing file
      */
      strcpy(temp, argv[1]);
      if ((ptr = strchr(temp, '.')) != NULL)
         strcpy(ptr, ".list");
      else
         strcat(temp, ".list");

      if ((listfile = fopen(temp, "w")) == NULL)
      {
         perror("Error opening entrypoint file");
         exit(1);
      }
   }



   endflag = 0;
   insource = 1;
   setjmp(error_env);
   if (insource == 0)
      goto END_OF_FILE;

   while (!feof(infile) && !endflag)
   {
      fgets(line, 255, infile);
      ++lineno;
      strcpy(current_line, line);
      strtrim(line, ONESPC);

      if (listfile)
      {
         fprintf(listfile, "%04X: ", current_address);
         linepos = 7;
      }

      if (line[0] == '\0')
         goto NEXT_LINE;


      label[0] = name[0] = mode[0] = '\0';

      if (isspace(line[0]))
      {
         sscanf(line, "%32s %128[^\n]", name, mode);
         strupr(name);
         strtrim(mode, NOSPCS);
      }
      else
      {
         /*
         ** Fetch the <label>, <name> and <mode> strings.
         */
         sscanf(line, "%32s %32s %128[^\n]", label, name, mode);

         /*
         ** Remove the trailing ':' character if used in label statement
         */
         if ((length = strlen(label)) == 0)
            error("internal error - null symbol name in main()");
         if (label[length-1] == ':')
            label[length-1] = '\0';

         /*
         ** If label case sensitivity is off - turn it into uppercase only
         */
         if (uc_labels)
            strupr(label);

         /*
         ** Normal <current_address> assignment ("<label>:")
         */
         if (name[0] == '\0')
         {
            if (set_label(label, current_address, CHECK) == -1)
               error("symbol storage full");
            goto NEXT_LINE;
         }

         /*
         ** Fix the <name> and <mode> string to look nice...
         */
         strupr(name);
         strtrim(mode, NOSPCS);

         /*
         ** Test for the "SET" statement - always set the label
         ** ie. do not test if it has already been defined.
         ** Syntax: <label> SET <expression>
         */
         if (strcmp(name, "SET") == 0)
         {
            if (get_value(mode, current_address, &value) == -1)
               error("undefined symbol in SET expression");

            if (set_label(label, value, NOCHECK) == -1)
               error("symbol storage full");

            goto NEXT_LINE;
         }

         /*
         ** Test for "<label> EQU <expression>" statement
         */
         if (strcmp(name, "EQU") == 0)
         {
            if (get_value(mode, current_address, &value) == -1)
               error("undefined symbol in EQU expression");

            if (set_label(label, value, CHECK) == -1)
               error("symbol storage full");

            goto NEXT_LINE;
         }

         /*
         ** Normal <current_address> assignment
         */
         if (set_label(label, current_address, CHECK) == -1)
            error("symbol storage full");
      }

      /*
      ** Assemble the instruction
      */
      endflag = decode_instruction(name, mode);

NEXT_LINE:
      display_source();
   }

   insource = 0;
   if (endflag != END)
      error("end of file reached without END statement");

END_OF_FILE:
   solve_forward();
   write_outfile(outfile);

   printf("\nAssembly status: %02d errors in %d source lines\n",
      errcount, lineno);

   if (listfile != NULL && listfile != stdout)
   {
      fprintf(listfile,
         "\nAssembly status: %02d errors in %d source lines\n",
         errcount, lineno);
      fclose(listfile);
   }

   if (outfile != stdout)
      fclose(outfile);
   if (infile != stdin)
      fclose(infile);

   return 0;
}
