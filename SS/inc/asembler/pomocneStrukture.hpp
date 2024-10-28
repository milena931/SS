#ifndef _POMOCNESTRUKTURE_HPP_
#define _POMOCNESTRUKTURE_HPP_


#include <cstdint>
#include <vector>


enum VrstaSimbola{
  SIMBOL_VRSTA,
  LITERAL_VRSTA
};

struct Simbol{
  VrstaSimbola vrsta;
  char* simbol; //ako je vrsta simbola simbol
  int literal;  //ako je u pitanju literal
};


struct Simboli{
  Simbol* simboli;
  int duzina;
};

enum Vrste_Direktiva{
  DIREKTIVA_GLOBAL,
  DIREKTIVA_EXTERN,
  DIREKTIVA_SECTION,
  DIREKTIVA_SKIP,
  DIREKTIVA_WORD,
  DIREKTIVA_ASCII,
  DIREKTIVA_END
};

struct Direktiva{
  Vrste_Direktiva vrsta;
  int broj; //ako je skip
  Simboli simboli;
  char* simbol;
  char* ascii_string; //ako je ASCII direktiva u pitanju
};

enum VrsteAdresiranja{
  IMMED,
  MEMDIR,
  REGDIR,
  REGIND,
  REGINDPOM
};

struct Operand{
  VrsteAdresiranja adresiranje;
  Simbol simbol;
  int broj;
  int reg;
};

enum VrsteInstrukcija{
  INSTRUKCIJA_HALT,
  INSTRUKCIJA_INT,
  INSTRUKCIJA_IRET,
  INSTRUKCIJA_CALL,
  INSTRUKCIJA_RET,
  INSTRUKCIJA_JMP,
  INSTRUKCIJA_BEQ,
  INSTRUKCIJA_BNE,
  INSTRUKCIJA_BGT,
  INSTRUKCIJA_PUSH,
  INSTRUKCIJA_POP,
  INSTRUKCIJA_XCHG,
  INSTRUKCIJA_ADD,
  INSTRUKCIJA_SUB,
  INSTRUKCIJA_MUL,
  INSTRUKCIJA_DIV,
  INSTRUKCIJA_NOT,
  INSTRUKCIJA_AND,
  INSTRUKCIJA_OR,
  INSTRUKCIJA_XOR,
  INSTRUKCIJA_SHL,
  INSTRUKCIJA_SHR,
  INSTRUKCIJA_LD,
  INSTRUKCIJA_ST,
  INSTRUKCIJA_CSRRD,
  INSTRUKCIJA_CSRWR
};

struct Instrukcija{
  VrsteInstrukcija instrukcija;  //treba da se ovo podudara sa nekim op. kodom
  VrsteAdresiranja adresiranje;  //treba negde i ove op. kodove izdvojiti
  int regA;
  int regB;
  int regC;
  int disp2;
  int disp1;
  int disp0;
  Operand operand;
  char* simbol;
};

enum VrsteLinija{
  PRAZNA,
  DIREKTIVA,
  INSTRUKCIJA
};


struct Linija{
  VrsteLinija vrsta;
  char* labela;
  Direktiva direktiva; //ako je u pitanju direktiva u liniji
  Instrukcija instrukcija;  //ako je instrukcija u liniji
  bool lab;
};


#endif
