#ifndef _EMULATOR_HPP_
#define _EMULATOR_HPP_

#include <iostream>
#include <map>
#include <termios.h>

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

using namespace std;

class Emulator{
  public:
    Emulator(char* program);

    void procitajFaj();
    void izvrsi();

    void izvrsiAr(int regA, int regB, int regC, int mod);
    void izvrsiLog(int regA, int regB, int regC, int mod);
    void izvrsiPomeracku(int regA, int regB, int regC, int mod);
    void izvrsiXchg(int regA, int regB, int regC, int mod);
    void izvrsiSt(int regA, int regB, int regC, int mod, uint32_t disp);
    void izvrsiLd(int regA, int regB, int regC, int mod, uint32_t disp);
    void izvrsiSkok(int regA, int regB, int regC, int mod, uint32_t disp);
    void izvrsiCall(int regA, int regB, int regC, int mod, uint32_t disp);
    void izvrsiInt();

    void postaviMemoriju(uint32_t addr, uint32_t podatak);
    uint32_t dohvatiIzMemorije(uint32_t addr);
    void nespostojecaInstrukcija();

    int tasterPritisnut();

    void ispisi();

    ~Emulator();

  private: 
    char* program;
    map<uint32_t, uint8_t> kontekst;
    uint32_t registri[16];
    uint32_t handler;
    uint32_t status;
    uint32_t cause;

    uint32_t term_in;
    uint32_t term_out;
    termios term;

};

#endif