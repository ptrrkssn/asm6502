
#define B(am)           (1<<(am))

#define AM_IMP          0
#define AM_ACC          1
#define AM_IMM          2
#define AM_ZP           3
#define AM_ZPX          4
#define AM_ZPY          5
#define AM_AB           6
#define AM_ABX          7
#define AM_ABY          8
#define AM_IN           9
#define AM_INX         10
#define AM_INY         11
#define AM_REL         12

#define AM_RAZ         20
#define AM_AZX         21
#define AM_AZY         22


int admode_length[] =
{
   0,    /* IMP */
   0,    /* ACC */
   1,    /* IMM */
   1,    /* ZP  */
   1,    /* ZPX */
   1,    /* ZPY */
   2,    /* AB  */
   2,    /* ABX */
   2,    /* ABY */
   2,    /* IN  */
   1,    /* INX */
   1,    /* INY */
   1,    /* REL */
};


struct
{
   char *match;
   int  admode;
} amms[] =
{
   {
      "#%[^,()\n]%c",
      AM_IMM
   },
   {
      "%[^,()\n]%c",
      AM_RAZ
   },
   {
      "%[^,()\n],X%c",
      AM_AZX
   },
   {
      "%[^,()\n],Y%c",
      AM_AZY
   },
   {
      "(%[^,()\n])%c",
      AM_IN
   },
   {
      "(%[^,()\n]),Y%c",
      AM_INY
   },
   {
      "(%[^,()\n],X)%c",
      AM_INX
   },
   {
      NULL,
      -1
   }
};


