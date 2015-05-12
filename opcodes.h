typedef struct
{
   char *name;
   int admodes;
   char opcode[13];
} instruction;


instruction idt[] =
{
   {
      "ADC",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0x69, 0x65, 0x75, 0, 0x6D, 0x7D, 0x79, 0, 0x61, 0x71 }
   },
   {
      "AND",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0x29, 0x25, 0x35, 0, 0x2D, 0x3D, 0x39, 0, 0x21, 0x31 }
   },
   {
      "CMP",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0xC9, 0xC5, 0xD5, 0, 0xCD, 0xDD, 0xD9, 0, 0xC1, 0xD1 }
   },
   {
      "EOR",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0x49, 0x45, 0x55, 0, 0x4D, 0x5D, 0x59, 0, 0x41, 0x51 }
   },
   {
      "LDA",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0xA9, 0xA5, 0xB5, 0, 0xAD, 0xBD, 0xB9, 0, 0xA1, 0xB1 }
   },
   {
      "ORA",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0x09, 0x05, 0x15, 0, 0x0D, 0x1D, 0x19, 0, 0x01, 0x11 }
   },
   {
      "SBC",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0xE9, 0xE5, 0xF5, 0, 0xED, 0xFD, 0xF9, 0, 0xE1, 0xF1 }
   },
   {
      "STA",
      B(AM_ZP)+B(AM_ZPX)+
      B(AM_AB)+B(AM_ABX)+B(AM_ABY)+B(AM_INX)+B(AM_INY),
      { 0, 0, 0, 0x85, 0x95, 0, 0x8D, 0x9D, 0x99, 0, 0x81, 0x91 }
   },
   {
      "ASL",
      B(AM_ACC)+B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0x0A, 0, 0x06, 0x16, 0, 0x0E, 0x1E }
   },
   {
      "LSR",
      B(AM_ACC)+B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0x4A, 0, 0x46, 0x56, 0, 0x4E, 0x5E }
   },
   {
      "ROL",
      B(AM_ACC)+B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0x2A, 0, 0x26, 0x36, 0, 0x2E, 0x3E }
   },
   {
      "ROR",
      B(AM_ACC)+B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0x6A, 0, 0x66, 0x76, 0, 0x6E, 0x7E }
   },
   {
      "BCC",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x90 }
   },
   {
      "BCS",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xB0 }
   },
   {
      "BEQ",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xF0 }
   },
   {
      "BMI",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30 }
   },
   {
      "BNE",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xD0 }
   },
   {
      "BPL",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10 }
   },
   {
      "BVC",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x50 }
   },
   {
      "BVS",
      B(AM_REL),
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x70 }
   },
   {
      "BIT",
      B(AM_ZP)+B(AM_AB),
      { 0, 0, 0, 0x24, 0, 0, 0x2C }
   },
   {
      "BRK",
      B(AM_IMP),
      { 0x00 }
   },
   {
      "CLC",
      B(AM_IMP),
      { 0x18 }
   },
   {
      "CLD",
      B(AM_IMP),
      { 0xD8 }
   },
   {
      "CLI",
      B(AM_IMP),
      { 0x58 }
   },
   {
      "CLV",
      B(AM_IMP),
      { 0xB8 }
   },
   {
      "DEX",
      B(AM_IMP),
      { 0xCA }
   },
   {
      "DEY",
      B(AM_IMP),
      { 0x88 }
   },
   {
      "INX",
      B(AM_IMP),
      { 0xE8 }
   },
   {
      "INY",
      B(AM_IMP),
      { 0xC8 }
   },
   {
      "NOP",
      B(AM_IMP),
      { 0xEA }
   },
   {
      "PHA",
      B(AM_IMP),
      { 0x48 }
   },
   {
      "PHP",
      B(AM_IMP),
      { 0x08 }
   },
   {
      "PLA",
      B(AM_IMP),
      { 0x68 }
   },
   {
      "PLP",
      B(AM_IMP),
      { 0x28 }
   },
   {
      "RTI",
      B(AM_IMP),
      { 0x40 }
   },
   {
      "RTS",
      B(AM_IMP),
      { 0x60 }
   },
   {
      "SEC",
      B(AM_IMP),
      { 0x38 }
   },
   {
      "SED",
      B(AM_IMP),
      { 0xF8 }
   },
   {
      "SEI",
      B(AM_IMP),
      { 0x78 }
   },
   {
      "TAX",
      B(AM_IMP),
      { 0xAA }
   },
   {
      "TAY",
      B(AM_IMP),
      { 0xA8 }
   },
   {
      "TSX",
      B(AM_IMP),
      { 0xBA }
   },
   {
      "TXA",
      B(AM_IMP),
      { 0x8A }
   },
   {
      "TXS",
      B(AM_IMP),
      { 0x9A }
   },
   {
      "TYA",
      B(AM_IMP),
      { 0x98 }
   },
   {
      "CPX",
      B(AM_IMM)+B(AM_ZP)+B(AM_AB),
      { 0, 0, 0xE0, 0xE4, 0, 0, 0xEC }
   },
   {
      "CPY",
      B(AM_IMM)+B(AM_ZP)+B(AM_AB),
      { 0, 0, 0xC0, 0xC4, 0, 0, 0xCC }
   },
   {
      "DEC",
      B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0, 0, 0xC6, 0xD6, 0, 0xCE, 0xDE }
   },
   {
      "INC",
      B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0, 0, 0xE6, 0xF6, 0, 0xEE, 0xFE }
   },
   {
      "JMP",
      B(AM_AB)+B(AM_IN),
      { 0, 0, 0, 0, 0, 0, 0x4C, 0, 0, 0x6C }
   },
   {
      "JSR",
      B(AM_AB),
      { 0, 0, 0, 0, 0, 0, 0x20 }
   },
   {
      "LDX",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPY)+B(AM_AB)+B(AM_ABY),
      { 0, 0, 0xA2, 0xA6, 0, 0xB6, 0xAE, 0, 0xBE }
   },
   {
      "LDY",
      B(AM_IMM)+B(AM_ZP)+B(AM_ZPX)+B(AM_AB)+B(AM_ABX),
      { 0, 0, 0xA0, 0xA4, 0xB4, 0, 0xAC, 0xBC }
   },
   {
      "STX",
      B(AM_ZP)+B(AM_ZPY)+B(AM_AB),
      { 0, 0, 0, 0x86, 0, 0x96, 0x8E }
   },
   {
      "STY",
      B(AM_ZP)+B(AM_ZPX)+B(AM_AB),
      { 0, 0, 0, 0x84, 0x94, 0, 0x8C }
   },
   {
      NULL,
      0
   }
};

