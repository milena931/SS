#ifndef _PARSER_HPP_
#define _PARSER_HPP_


#include "../../flexbison.tab.hpp"
#include "kontekst.hpp"
#include "glavnaPetlja.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

extern int linije;


class Parser{  //klasa koja prima linije ulaznog fajla i prosledjuje ih bisonu i flex-u tako sto poziva yyparse()
public:

  Parser(char* oputputFIle, char* debugFile);  //parser kad se napravi pokrece yyparse funkciju koja formira kontekst

  vector<Linija> kontekst;  //ovde ce u bisonu da se popinjava ova klasa 
  static bool dodaj;
  char* outputFile;
  char* debugFile;
private:

};

#endif