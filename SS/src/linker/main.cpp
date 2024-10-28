#include <iostream>
#include <string.h>
#include "../../inc/linker/linker.hpp"

using namespace std;

int main(int argc, char** argv) {

    bool relocatable = false;
    bool hex = false;
    char* izlazna_datoteka = nullptr;

    Linker* linker = new Linker();
    
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-hex")){
            hex = true;
        }else if(!strcmp(argv[i], "-relocatable")){
            relocatable = true;
        }else if(!strcmp(argv[i], "-o") && (i+1) < argc){
            izlazna_datoteka = argv[i+1];
            i++;
        }else if(string(argv[i]).substr(0,6) == "-place"){  //treba da se namesti pocetak sekcije od navedene sdrese
            //treba da iz stringa oblika -place=<imeSekcije>@<adresa> da izvucem sve i pozovem metodu da postavi u linkerov niz pocetak te sekcije

            uint32_t start;
            string naziv;

            string arg = string(argv[i]).substr(7);
			int pos = arg.find("@");
			if(pos == arg.npos){
                cerr<<endl<<"Los argument: "<<argv[i]<<endl;
                exit(1);
            }
			string num = arg.substr(pos + 1);
			if (num[0] == '0' && num[1] == 'x'){
				start = (uint32_t)strtol(num.substr(2).c_str(), NULL, 16);
            }else{
				start = (uint32_t)strtol(num.c_str(), NULL, 10);
            }
			naziv = arg.substr(0, pos);
            linker->dodajPocetakSekcije(start, naziv);
        }else{
            //ako nije ni jedna od ovih ostalih stvari onda su u pitanju obektne datoteke za povezivanje i treba da se redom kako su navedene povezu
            //ideja je da imam metodu koja ce da cita fajl i smesta stvari u privremene strukture i da onda kada se sve procita prebaci u globalnu strukturu linkera
            //nakon sto se prebaci u globalnu strukturu onda treba da se sve privremeno sto se bilo popunilo isprazni da bi se punilo opet za sledeci fajl
            char* fajl = argv[i];
            linker->procitaj_fajl(fajl);
        }
    }

    if(izlazna_datoteka == nullptr){
        cerr<<endl<<"Greška: niste naveli naziv izlaznog fajla"<<endl;
        exit(1);
    }

    if(hex && relocatable){
        cerr<<endl<<"Greška: ne mogu da se navedu i relocatable i hex"<<endl;
        exit(1);
    }

    if(!((hex && !relocatable) || (!hex && relocatable))){
        cerr<<endl<<"Greška: Niste naveli ni jednu opiju kojom linker generiše izlaz. Izaberite -hex ili -relocatable"<<endl;
        exit(1);
    }

    //hex se radi podrazumevano pa za to ne mora da se poziva metoda ali za relocatable mora
    if(relocatable){
        linker->postaviRelocatable();
    }

    linker->kreiranjeSekcija();
    
    if(!relocatable){
        //nakon kreiranja i konkatenacija sekcija treba da se postavi pocetak sekcija koji je dat sa place
        linker->namestiPocetkeSekcijama();
        //nakon sto su svi pocetci postavljeni mogu da se izracunaju svi pomeraju u relokacionim zapisima
        linker->sortirajSekcije();  //ovu metodu pozivam u slucaju da neka sekcija ima namesten start koji nije 0 da bi bile onda redom kako ce se nalaziti postavljene da bi mogao lakse da se izracuna pomeraj kroz sekcije
    }
    linker->odrediVrednostiSimbolima();
    linker->razresiRelokacioneZapise();

    if(hex){
        linker->formirajHexFajl(izlazna_datoteka);
    }else{
        linker->formirajOFajl(izlazna_datoteka);
    }

    //delete linker;
    cout<<endl<<"Uspesno povezivanje!"<<endl;
    return 0;
}
