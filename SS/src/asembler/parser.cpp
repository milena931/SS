#include "../../inc/asembler/parser.hpp"

bool Parser::dodaj = false;

Parser::Parser(char* outputFile, char* debugFile){
  this->outputFile = outputFile;
  this->debugFile = debugFile;
  int ret = yyparse(kontekst);

  if(ret >= 1){
    cerr << "Greska prilikom parsiranja.";
    return;
  }else{ //ako je parsiranje uspesno dalja obrada.... kontekst bi trebalo da se sada spremi za backpatching

    if(!kontekst.empty()){  //uspesno parsiranje mora da ima kontekst koji nije prazan

      //obrada nakon parsiranja
      /*
      cout << endl;

      cout << "VRSTA: " << kontekst[11].vrsta << endl;
      cout << "Adresiranje: " << kontekst[11].instrukcija.adresiranje << endl;
      cout << "Instrukcija: " << kontekst[11].instrukcija.instrukcija <<endl;
      cout << "Registar: " << kontekst[11].instrukcija.regA << endl;
      cout << "Adresiranje: " << kontekst[11].instrukcija.operand.adresiranje << endl;
      cout << "Vrsta simbola: " << kontekst[11].instrukcija.operand.simbol.vrsta << endl;
      cout << "Simbol: " << kontekst[11].instrukcija.operand.simbol.simbol << endl;
      cout<< endl;

      cout << "Direktiva vrsta: " << kontekst[6].vrsta << endl;
      cout << "Direktiva: " << kontekst[6].direktiva.vrsta <<endl;
      cout << "Simbol: " << kontekst[6].direktiva.simboli.duzina << endl;
      */
      //cout << endl << "Kontekst: " << kontekst.size() << endl;
      //cout << endl << "Linije: " << linije << endl;

      linije = kontekst.size();

      glavnaPetlja(this->kontekst, linije, this->outputFile, this->debugFile);  //u ovoj metodi se na kraju zove backpatching

      //fajl je formiran pa se kontekst sada prazni i ponovo se puni u sledecem fajlu
      kontekst.clear();
    
    }else{
      cerr << "Greska prilikom parsiranja";
    }
    
  }
}
