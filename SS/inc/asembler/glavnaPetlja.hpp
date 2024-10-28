#ifndef GLAVNA_PETLJA_HPP_
#define GLAVNA_PETLJA_HPP_

#include <iostream>
#include "pomocneStrukture.hpp"
#include "kontekst.hpp"
#include "backpatching.hpp"

using namespace std;

void direktiva_handler(Linija& linija, Kontekst* kontekst);

void instrukcija_handler(Linija& linija, Kontekst* kontekst);

void glavnaPetlja(vector<Linija>& kontekst, int broj_linija, char* objFajl, char* debugFajl);

#endif