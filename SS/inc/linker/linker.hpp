#ifndef _LINKER_HPP_
#define _LINKER_HPP_

#include <iostream>
#include <string.h>

#include "../../inc/linker/pomocneStrukture.hpp"

using namespace std;

class Linker{
  // ova klasa ce da ima metode koje ce da rade sve sto treba da radi linker
  // prva metoda koju pravimo je metoda koja ce da prima fajl koji treba da se prodje
  // ceo postupak treba da bude nesto ovako :
  // 1) učitavanje fajlova i sekcija
  // 2) kreiranje internih sekcija i konkatenacija
  // 3) razresavanje relokacija
  // 4) kreiranje hex fajla (ako je to trazeno, a ako ne onda se samo sve serijalizuje u objektni fajl)
public:
  Linker();

  void procitaj_fajl(char* fajl);


  void dodaj_fajl(char* fajl, FILE* o_file);
  int charToInt(char c);

  void dodajSekciju(Sekcija* sekcija);  
  void dodajSimbol(TabelaSimbola* simbol, int fajl);
  void dodajNedefinisan(TabelaSimbola* simbol, int fajl);
  void dodajPocetakSekcije(uint32_t pocetak, string naziv);
  void namestiPocetkeSekcijama();
  void odrediVrednostiSimbolima();
  void postaviRelocatable();

  void kreiranjeSekcija();
  void sortirajSekcije();
  void razresiRelokacioneZapise();
  void formirajHexFajl(char* fajl);
  void formirajOFajl(char* fajl);

  ~Linker();
private:
  char* trenutniFajl;
  bool prvi_prolaz;
  int brFajlova;
  int brojSimbola;
  int brojSekcija;
  bool hex;
  bool relocatable;

  Sekcija* sekcije;
  TabelaSimbola* tabela_simbola;
  TabelaSimbola* nedefinisani_simboli;

  Place* mesta_pocetka;

  uint32_t max_place;

  Fajlovi* fajlovi;

};

#endif