#ifndef _POMOCNE_STRUKTURE_HPP_
#define _POMOCNE_STRUKTURE_HPP_

#include <iostream>
#include <vector>

using namespace std;

struct TabelaSekcija{
  string naziv;
  int rbr;
  int velicina;
  TabelaSekcija* sledeca;
};

struct TabelaSimbola{
  int rbr;   // redni broj
  uint32_t vrednost;   // vrednost simbola (adresa)
  bool global;  //da li je simbol lokalan ili globalan
  int sekcija;
  string naziv;
  bool definisan;
  bool uvezen;
  bool sekcija_bool;
  string naziv_sekcije;
  int fajl;
  TabelaSimbola* sledeci;
};

struct RelokacioniZapis{
  uint32_t offset;  // offset u sekciji u kojoj treba da se prepravi nesto
  int simbol; //ako je lokalan bice sekcija
  int fajl;
  uint32_t addend;  //addend sluzi ako je simbol lokalan da bi se pronasao u sekciji
  RelokacioniZapis* sledeci;
};

struct Sekcija{
  string naziv;
  RelokacioniZapis* relokacioni_zapisi;
  std::vector<uint32_t> sadrzaj;
  uint32_t start;
  int velicina;
  Sekcija* sledeca;
};

struct Fajlovi{
  char* nazivFajla;
  TabelaSekcija* tabela_sekcija;
  TabelaSimbola* tabela_simbola;
  Sekcija* sekcije;   //ulancane sekcije sa sadrzajem
  Fajlovi* sledeci;
};

struct Place{
  string naziv;
  uint32_t start;
  Place* sledeca;
};


#endif