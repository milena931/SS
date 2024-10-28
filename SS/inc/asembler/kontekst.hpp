#ifndef _KONTEKST_HPP_
#define _KONTEKST_HPP_

#include "pomocneStrukture.hpp"
#include <iostream>
#include <cstdint>
#include <iomanip>
#include "string.h"

using namespace std;


//mesto za operacione kodove instrukcija

const int OC_HALT = 0;
const int OC_INT = 1; 
const int OC_CALL = 2;
const int CALL_1 = 0;
const int CALL_2 = 1;
const int OC_JMP = 3; //oc za sve skokove sada vrste imaju drugi bajt razlicit -->modovi
const int JMP_1 = 0;
const int JMP_2 = 1;
const int JMP_3 = 2; 
const int JMP_4 = 3;
const int JMP_5 = 4;
const int JMP_6 = 5;
const int JMP_7 = 6;
const int JMP_8 = 7;
const int OC_XCHG = 4;
const int OC_AR = 5; // oc za instrukcije aritmetickih operacija
const int AR_ADD = 0;
const int AR_SUB = 1;
const int AR_MUL = 2;
const int AR_DIV = 3;
const int OC_LOG = 6; // oc za intrukcije logickih operacija
const int LOG_NOT = 0;
const int LOG_AND = 1;
const int LOG_OR = 2;
const int LOG_XOR = 3;
const int OC_SH = 7;  //oc za pomeracke instrukcije
const int SH_L = 0;
const int SH_R = 1;
const int OC_ST = 8; //oc za st instrukciju - ima 3 moda
const int ST_1 = 0;
const int ST_2 = 1;
const int ST_3 = 2;
const int OC_LD = 9;  //oc za ld - ima 8 modova
const int LD_1 = 0;
const int LD_2 = 1;
const int LD_3 = 2;
const int LD_4 = 3;
const int LD_5 = 4;
const int LD_6 = 5;
const int LD_7 = 6;
const int LD_8 = 7;

enum TipSimbola{
  SECT,
  NOTYP
};


struct UlazTabeleSimbola{
  int rbr;   // redni broj
  uint32_t vrednost;   // vrednost simbola (adresa)
  int velicina;   // velicina simbola
  TipSimbola tip;    // tip simbola
  bool global;  //da li je simbol lokalan ili globalan
  int sekcija;
  char* naziv;
  bool definisan;
  bool uvezen;
  bool sekcija_bool;
  UlazTabeleSimbola* sledeci;
};



struct Sekcija{  //zgodno je da imam zabelezeno u nekom nizu koje sve sekcije imam i da pratim njhovu velicinu
  char* naziv;
  int rbr;
  int velicina;
  Sekcija* sledeca;
};


struct KontekstElem{
  int oc_instrukcije;
  int mod;
  int regA;
  int regB;
  int regC;
  int disp;
  bool jump;
  bool skip;
  int skip_bajtovi;
  bool ascii;
  int ascii_bajtovi;
  char* ascii_hex;
  bool word;
  uint32_t word_bajtovi;
  int sekcija;
  KontekstElem* sledeci;
};


struct TabelaLiterala{
  uint32_t vrednost;
  uint32_t velicina;
  uint32_t lokacija;    //ovo se popunjava samo u bekpecingu
  uint8_t sekcija;
  TabelaLiterala* sledeci;
};

struct BackpatchingElem{  //prilikom backpatching-a moze da se desi da u disp treba da se upise ili vrednost simbola ili nekog literala iz bazena literala
  BackpatchingElem* sledeci;
  uint32_t lokacija;
  bool simbol;
  bool literal;
  uint32_t lit;
  char* sim;  //pa na osnovu toga trazimo vrednost simbola
  int sekcija;
};

struct RelokacioniZapisElem{  //ovde posto imamo samo jedan tip relokacije ne treba nam nista vise
  uint32_t offset;  // offset u sekciji u kojoj treba da se prepravi nesto
  int simbol; //ako je lokalan bice sekcija
  uint32_t addend;  //addend sluzi ako je simbol lokalan da bi se pronasao u sekciji
  RelokacioniZapisElem* sledeci;
  int sekcija;  //relokaciona tablea je vezana za sekciju
  bool backpatching;
};

struct BazeniLiterala{
  vector<uint32_t> bazen;
  int sekcija;
  BazeniLiterala* sledeci;
};

class Kontekst{  //ova klasa ce da sadrzi strukture koje se formiraju u glavnoj petlji - tabela simbola, relokacioni zapisi i mesta koja treba da se obrade u backpatching-u

  public:
    Kontekst();

    UlazTabeleSimbola* tabela_simbola;


    bool kraj; 
    int trenutnaSekcija;
    int brojSimbolUTabeli;
    int brojSekcija;
    int brojRelokacionihZapisa;

    uint32_t locationCounter;

    KontekstElem* kontekst_ostatak;
    Sekcija* sekcije;
    BazeniLiterala* bazen_literala;
    TabelaLiterala* tabela_literala;
    BackpatchingElem* tabela_obracanja_unapred;
    RelokacioniZapisElem* relokacioni_zapisi;

    void dodajSimbol(char* simbol, uint32_t adresa, bool global, bool definisan, bool uvezen, bool sekcija);
    void dodajSkipUKontekst(int broj); //prosledjuje se samo broj bajtova koje zauzima
    void dodajSekciju(char* naziv);
    void dodajGlobal(Simboli& simboli);
    void dodajExtern(Simboli& simboli);
    void dodajWord(Simboli& simboli);
    void dodajAscii(char* niz);

    bool simbolUTabeli(char* simbol);
    uint32_t vrednostSimbola(char* simbol);
    int sekcijaSimbola(char* simbol);
    void dodajUTabeluLiterala(uint32_t literal);
    void dodajUTabeluObracanjaUnapred(uint32_t lc, bool literal, bool simbol, uint32_t lit, char* sim);
    void dodajRelokacioniZapis(uint32_t lc, char* simbol);
    UlazTabeleSimbola* ulazTabeleSimbola(char* simbol);
    uint16_t velicinaSekcije(int id);
    void postaviVelicinuPoslednjeSekcije();

    char* nazivSekcije(int rbr);

    void dodajAddSubMulDiv(int id, int reg1, int reg2);   // ako se prosledi 0 - add, 1 - sub, 2 - mul, 3 - div 
    void dodajShlShr(int id, int reg1, int reg2);   //0 - shl, 2 - shr
    void dodajNotAndOrXor(int id, int reg1, int reg2);  //0 - not, 1 - and, 2 - or, 3 - xor
    void dodajXchg(int reg1, int reg2);
    void dodajSkok(int id, Instrukcija instrukcija);
    void dodajHalt();
    void dodajInt();
    void dodajCall(Instrukcija& instrukcija);
    void dodajCsrwr(Instrukcija& instrukcija);
    void dodajCsrrd(Instrukcija& instrukcija);
    void dodajLd(Instrukcija& instrukcija);
    void dodajPop(Instrukcija& instrukcija);
    void dodajSt(Instrukcija& instrukcija);
    void dodajPush(Instrukcija& instrukcija);
    void dodajRet();
    void dodajIret();

    void proveraSimbola();
    void napraviBazeneLiterala();
    void poravnajSekcije();
};

#endif