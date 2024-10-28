#include "../../inc/asembler/backpatching.hpp"
#include <iostream>
#include <fstream>
using namespace std;


void backpatching(Kontekst* kontekst){
  //u ovoj metodi treba da prodjemo kroz listu lokacija koje treba da se razrese bekpecingom i da izmenimo kontekst tamo gde treba
  BackpatchingElem* curr = kontekst->tabela_obracanja_unapred;

  while(curr){
    uint32_t lc = curr->lokacija;
    int sekcija = curr->sekcija;
    uint16_t velicinaSekcije = kontekst->velicinaSekcije(sekcija);
    if(curr->simbol){  //znaci da treba da se razresi neki simbol
      //treba naci instrukciju koja pocinje na lc i promeniti njen disp ako je simbol u medjuvremenu definisan ako nije -> greska
      char* simbol = curr->sim;
      if(kontekst->simbolUTabeli(simbol)){
        int s = kontekst->sekcijaSimbola(simbol);
        if(s == sekcija){  //provera dodatno da je zapis nesto sto moze da se razresi u bekpecingu
          KontekstElem* pom = kontekst->kontekst_ostatak;
          uint32_t br = 0;
          while(pom){
            if(br == lc){  // ovde ce da iskoci kada smo nasli gde treba da se postavi disp
              break;
            }

            if(pom->ascii && pom->sekcija == s){
              br += pom->ascii_bajtovi;
            }else if(pom->jump && pom->sekcija == s){
              br += 4;
            }else if(pom->skip && pom->sekcija == s){
              br += pom->skip_bajtovi;
            }else if(pom->word && pom->sekcija == s){
              br += 4;
            }else if(pom->sekcija == s){
              br += 4;
            }

            pom = pom->sledeci;
          }
          uint32_t vr = kontekst->vrednostSimbola(simbol);
          int disp = vr - (lc + 4);  //treba da se preskoci trenutna instrukcija -> pc dok se instrukcija izvrsava ce da pokazuje na pocetak sledec instrukcije
          if (disp < 0) {
            disp = (disp + 0x1000) & 0x0fff;
          } else {
            disp = disp & 0x0fff;
          }
          pom->disp = disp;  //postavljanje pomeraja
        }else{
         //treba da se doda relokacioni zapis
         RelokacioniZapisElem* novi = new RelokacioniZapisElem();
         novi->backpatching = false;
         novi->offset = curr->lokacija;  //dovoljno je za relokacioni zapis da pamtimo instrukciju kojoj treba da se menja disp
         UlazTabeleSimbola* simb = kontekst->ulazTabeleSimbola(simbol);
         novi->sekcija = sekcija;  //sekcija u kojoj treba da se uradi zapis
         novi->sledeci = nullptr;
         if(simb->global){
          novi->simbol = simb->rbr;
          novi->addend = 0;
         }else{
          char* nazivSekcije = kontekst->nazivSekcije(simb->sekcija);
          UlazTabeleSimbola* sekc = kontekst->ulazTabeleSimbola(nazivSekcije);
          novi->simbol = sekc->rbr;
          novi->addend = simb->vrednost;
          //u ovom slucaju treba da prodjemo kroz kontekst i da umetnemo jos jednu ld instrukciju ako je u pitanju relokcaioni zapis nad ld instrukcijom
          int cnt = 0;
          KontekstElem* k = kontekst->kontekst_ostatak;
          KontekstElem* prev = nullptr;
          while(k){
            if(k->sekcija == sekcija){
              if(cnt == novi->offset){
                if(k->oc_instrukcije == OC_LD){
                  KontekstElem* novaInstr = new KontekstElem();
                  novaInstr->oc_instrukcije = OC_LD;
                  novaInstr->mod = LD_3;
                  novaInstr->ascii = false;
                  novaInstr->jump = false;
                  novaInstr->skip = false;
                  novaInstr->sekcija = k->sekcija;
                  novaInstr->regA = k->regA;
                  novaInstr->regB = k->regA;
                  novaInstr->regC = 0;
                  novaInstr->disp = 0;
                  novaInstr->sledeci = k->sledeci;
                  k->sledeci = novaInstr;

                  //nakon dodavanja treba da se promeni velicina sekciji
                  Sekcija* sek = kontekst->sekcije;
                  while(sek){
                    if(sek->rbr == k->sekcija){
                      sek->velicina+=4;
                    }
                    sek = sek->sledeca;
                  }
                  //nakon toga treba isto sve relokacijone zapise te sekcije koji su nakon mesta ovog upisa za 4
                  RelokacioniZapisElem* r = kontekst->relokacioni_zapisi;
                  while(r){
                    if(r->sekcija == sekcija && r->offset > cnt){
                      r->offset += 4;
                    }
                    r = r->sledeci;
                  }
                  //treba takodje pomeriti mesto upisa u bekpecing zapisima koji su nakon ovog
                  BackpatchingElem* b = kontekst->tabela_obracanja_unapred;
                  while(b){
                    if(b->sekcija == sekcija && b->lokacija > cnt){
                      b->lokacija += 4;
                    }
                    b = b->sledeci;
                  }
                }
              }
              cnt+=4;
            }
            prev = k;
            k = k->sledeci;
          }
         }
         RelokacioniZapisElem* rel = kontekst->relokacioni_zapisi;
         if(rel != nullptr){
          while(rel->sledeci){
            rel = rel->sledeci;
          }
          rel->sledeci = novi;
         }else{
          kontekst->relokacioni_zapisi = novi;
         }
        }
      }else{
        RelokacioniZapisElem* novi = new RelokacioniZapisElem();
        novi->backpatching = false;
        novi->offset = curr->lokacija;  //dovoljno je za relokacioni zapis da pamtimo instrukciju kojoj treba da se menja disp
        UlazTabeleSimbola* simb = kontekst->ulazTabeleSimbola(simbol);
        novi->sekcija = sekcija;  //sekcija u kojoj treba da se uradi zapis
        novi->sledeci = nullptr;
        if(simb->uvezen){
          novi->simbol = simb->rbr;
          novi->addend = 0;
        }else{
          cerr<<" Greska asembler"<<endl;
          exit(1);
        }
        RelokacioniZapisElem* rel = kontekst->relokacioni_zapisi;
        if(rel != nullptr){
          while(rel->sledeci){
            rel = rel->sledeci;
          }
          rel->sledeci = novi;
        }else{
          kontekst->relokacioni_zapisi = novi;
        }
      }

    }
    curr = curr->sledeci;
  }

  //posto u obradi bekpecinga simbola moze da se promeni velicina sekcije onda se literali rade nakon toga
  curr = kontekst->tabela_obracanja_unapred;
  while(curr){
    //treba nakon simbola da seobrade literali
    uint32_t lc = curr->lokacija;
    int sekcija = curr->sekcija;
    uint16_t velicinaSekcije = kontekst->velicinaSekcije(sekcija);
    if(curr->literal){  //literal iz bazena literala
      if(velicinaSekcije > 4096){  //maksimalna velicina sekcije
        cerr << endl << "Sekcija premasuje maksimalnu velicinu" << endl;
        exit(1);
      }else{
        BazeniLiterala* pom = kontekst->bazen_literala;
        BazeniLiterala* trenutni_bazen = nullptr;   //bazen sekcije koji posmatramo
        while(pom){
          if(pom->sekcija == sekcija){
            trenutni_bazen = pom;
          }
          pom = pom->sledeci;
        }

        uint16_t cnt = 0;  //cnt nam je na kraju offset do trazenog literala
        KontekstElem* k = kontekst->kontekst_ostatak;
        while(k){
          if(k->sekcija == sekcija){
            cnt += 4;
          }
          k = k->sledeci;
        }
        //u ovom trenutku cnt pokazuje na pocetak bazena literala
        for(int i = 0; i < trenutni_bazen->bazen.size(); i++){
          if(trenutni_bazen->bazen[i] == curr->lit){
            break;
          }
          cnt = cnt + 4;
        }
        KontekstElem* pom1 = kontekst->kontekst_ostatak;
        uint32_t br = 0;
        while(pom1){
          if(br == lc){  // ovde ce da iskoci kada smo nasli gde treba da se postavi disp
            break;
          }
          if(pom1->ascii && pom1->sekcija == sekcija){
            br += pom1->ascii_bajtovi;
          }else if(pom1->jump && pom1->sekcija == sekcija){
            br += 4;
          }else if(pom1->skip && pom1->sekcija == sekcija){
            br += pom1->skip_bajtovi;
          }else if(pom1->word && pom1->sekcija == sekcija){
            br += 4;
          }else if(pom1->sekcija == sekcija){
            br += 4;
          }
          
          pom1 = pom1->sledeci;
        }
        if(pom1 == nullptr){
          cerr << endl << "Backpatching greska" << endl;
          exit(1);
        }
        br = br;  //br je pozicija instrukcije u sekciji
        pom1->disp = cnt - (br + 4);  //jer pc u tom trenutku na kraju instrukcije pokazuje na kraj instrukcije tj na pocetak sledece
      }
    }
    curr = curr->sledeci;
  }

  //takodje u backpathing-u treba da se razrese mesta u relokacionim zapisima za simbole koji nisu uvezeni i nisu definisiani u vreme pravljenja relokacionog zapisa
  RelokacioniZapisElem* pom = kontekst->relokacioni_zapisi;
  while(pom){
    if(pom->backpatching){
      //bice dva slucaja -> lokalan simbol i globalan simbol
      int simb = pom->simbol;
      UlazTabeleSimbola* ulaz = kontekst->tabela_simbola;
      while(ulaz){
        if(ulaz->rbr == simb){
          break;
        }
        ulaz = ulaz->sledeci;
      }
      if(ulaz->global){
        pom->addend = 0;
        pom->backpatching = false;
        pom->simbol = ulaz->rbr;
      }else{
        pom->addend = ulaz->vrednost;
        pom->backpatching = false;
        if(ulaz->uvezen){

        }else{  //lokalan simbol
          char* nazivSekcije = kontekst->nazivSekcije(ulaz->sekcija);
          UlazTabeleSimbola* sekc = kontekst->ulazTabeleSimbola(nazivSekcije);
          pom->simbol = sekc->rbr;
        }
      }
    }
    pom = pom->sledeci;
  }
  //ako nakon ovog ima relokacionih zapisa koji imaju sve nule za offset simbol i addend treba ih ukloniti
  pom = kontekst->relokacioni_zapisi;
  RelokacioniZapisElem* prev = nullptr;
  while(pom){
    bool flag = true;
    if(pom->offset == 0 && pom->addend == 0 && pom->simbol == 0){
      //ulaz pom treba da se ukloni
      if(prev == nullptr){
        //znaci da se uklanja prvi
        flag = false;
        kontekst->relokacioni_zapisi = kontekst->relokacioni_zapisi->sledeci;
      }else{
        prev->sledeci = pom->sledeci;
      }
    }
    if(flag){
      prev = pom;
    }
    pom = pom->sledeci;
  }
}

void formiranjeObjektnogFajla( Kontekst* kontekst, char* objFajl){      //ova metoda formira objektni fajl
  ofstream output;

  output.open(objFajl);

  if(!output){
    cerr<<endl<<"Greška pri formiranju izlaznog ojektnog fajla"<<endl;
    exit(1);
  }
  
  //redosled upisivanja
  //treba prvo tabela sekcija
  //sto se tabele sekcija tice odvojicu 2B
  //jedan ulaz treba da sadrzi identifikator sekcije(2B) i njenu velicinu(2B) i naziv sekcije (1 karakter 1 bajt -> 12B - maksimalno 12 karaktera)
  //output<<"#tabela sekcija"<<endl;
  output<<hex<<setw(4)<<setfill('0')<<kontekst->brojSekcija;  //broj ulaza u tabelu
  
  Sekcija* pom = kontekst->sekcije;
  while(pom){
    output<<hex<<setw(4)<<setfill('0')<<pom->rbr<<hex<<setw(4)<<setfill('0')<<pom->velicina;  //identifikator i velicina
    //naziv
    int indeks = 0;
    int br = 0;
    char* bajtovi = new char[2 * strlen(pom->naziv)+2];
    while(pom->naziv[br]!='\0'){    //ovde se pretvara u hex
      sprintf(&bajtovi[indeks], "%02x", static_cast<unsigned char>(pom->naziv[br]));
      indeks += 2;
      br++;
    }
    sprintf(&bajtovi[indeks], "%02x", static_cast<unsigned char>('\0'));
    output<<bajtovi;
    
    pom = pom->sledeca;
  }

  //tabela simbola
  //output<<endl<<"#tabela simbola"<<endl;
  //tabela simbola treba da ima prvo broj ulaza na 4B
  output<<hex<<setw(8)<<setfill('0')<<kontekst->brojSimbolUTabeli;
  UlazTabeleSimbola* simb = kontekst->tabela_simbola;
  //jedan ulaz tabele simbola mora da ima redni broj simbola na 4B, zatim vrednost na 4B, 1B koji moze da ima vrednost 0 ili 1 (1 - ako je globalan), 1B koji moze da ima vrednost 0 ili 1 (1 - ako je sekcija), 
  //1B koji moze da ima vrednost 0 ili 1 (1 - ako je uvezen), sekciju u kojoj se nalazi(4B) ako je extern bice 0, naziv koji se zavrsava sa '\0'
  while(simb){
    output<<hex<<setw(8)<<setfill('0')<<simb->rbr<<hex<<setw(8)<<setfill('0')<<simb->vrednost;
    if(simb->global){
      output<<1;
    }else{
      output<<0;
    }
    if(simb->sekcija_bool){
      output<<1;
    }else{
      output<<0;
    }
    if(simb->uvezen){
      output<<1;
    }else{
      output<<0;
    }
    output<<hex<<setw(8)<<setfill('0')<<simb->sekcija;
    int indeks = 0;
    int br = 0;
    char* bajtovi = new char[2 * strlen(simb->naziv)+2];
    while(simb->naziv[br]!='\0'){    //ovde se pretvara u hex
      sprintf(&bajtovi[indeks], "%02x", static_cast<unsigned char>(simb->naziv[br]));
      indeks += 2;
      br++;
    }
    sprintf(&bajtovi[indeks], "%02x", static_cast<unsigned char>('\0'));
    output<<bajtovi;
    simb = simb->sledeci;
  }

  //output<<endl<<"#sekcije"<<endl;


  //sekcije -> svaka sekcija treba da sadrzi relokacione zapise za tu sekciju i onda sadrzaj sekcije
  Sekcija* sekc = kontekst->sekcije;
  while(sekc){
    int brRelZapisa = 0;
    RelokacioniZapisElem* rel = kontekst->relokacioni_zapisi;
    while(rel){
      if(rel->sekcija == sekc->rbr){
        brRelZapisa++;
      }
      rel = rel->sledeci;
    }

    //prva stvar koja mora da se ispise je broj relokacionih zapisa na 4B
     output<<hex<<setw(8)<<setfill('0')<<brRelZapisa;

    //zatim ide ispis tih relokacionih zapisa
    //ispisuje se prvo offset od pocetka sekcije (4B), zatim simbol(rbr -> 4B), zatim addend(ako je simbol lokalan to ce biti pomeraj u odnosu na pocetak sekcije u kojoj se simbol nalazi)
    rel = kontekst->relokacioni_zapisi;
    while(rel){
      if(rel->sekcija == sekc->rbr){
        output<<hex<<setw(8)<<setfill('0')<<rel->offset<<hex<<setw(8)<<setfill('0')<<rel->simbol<<hex<<setw(8)<<setfill('0')<<rel->addend;
      }
      rel = rel->sledeci;
    }
    //nakon ovoga ispisujemo sadrzaj sekcije iz konteksta -> ovo su sada bajtovi koje sam unosila prilikom obade u asembleru
    //instr: od(4b) mod(4b) regA(4b) regB(4b) regC(4b) disp(12b)
    KontekstElem* kont = kontekst->kontekst_ostatak;
    while(kont){
      if(kont->sekcija == sekc->rbr){
        if(kont->ascii){
          output<<kont->ascii_hex;
        }else if(kont->word){
          //cout<<"Word: "<<hex<<setw(8)<<setfill('0')<<kont->word_bajtovi<<endl;
          output<<hex<<setw(8)<<setfill('0')<<kont->word_bajtovi;
        }else if(kont->skip){
          for(int i = 0; i < kont->skip_bajtovi; i++){
            output<<0<<0;
          }
        }else{
          //cout<<"Instrukcija: "<<kont->oc_instrukcije<<hex<<kont->mod<<hex<<kont->regA<<hex<<kont->regB<<hex<<kont->regC<<hex<<setw(3)<<setfill('0')<<kont->disp<<endl;
          output<<hex<<kont->oc_instrukcije<<hex<<kont->mod<<hex<<kont->regA<<hex<<kont->regB<<hex<<kont->regC<<hex<<setw(3)<<setfill('0')<<(kont->disp & 0x0fff);
        }
      }
      kont = kont->sledeci;
    }
    //bazen literala te sekcije
    BazeniLiterala* bazeni = kontekst->bazen_literala;
    while(bazeni){
      if(bazeni->sekcija == sekc->rbr){
        for(uint32_t el: bazeni->bazen){
          output<<hex<<setw(8)<<setfill('0')<<el;
        }
      }
      bazeni = bazeni->sledeci;
    }

    sekc = sekc->sledeca;
  }


  output.close();
}

void formiranjeTekstualnogFajla( Kontekst* kontekst, char* debugFajl){
  ofstream output;

  output.open(debugFajl);

  if(!output){
    cerr<<endl<<"Greška pri formiranju izlaznog debug fajla"<<endl;
    exit(1);
  }

  //redosled upisivanja
  //treba prvo tabela sekcija
  output<<"#tabela sekcija"<<endl;
  output<<"Broj sekcija: "<<kontekst->brojSekcija<<endl;  //broj ulaza u tabelu
  
  Sekcija* pom = kontekst->sekcije;
  while(pom){
    output<<"Redni broj: "<<hex<<setw(4)<<setfill('0')<<pom->rbr<<"  Velicina: "<<hex<<setw(4)<<setfill('0')<<pom->velicina<< " Naziv: "<<pom->naziv<<endl;  //identifikator i velicina
    pom = pom->sledeca;
  }

  //tabela simbola
  output<<endl<<"#tabela simbola"<<endl;
  //tabela simbola treba da ima prvo broj ulaza na 4B
  output<<"Broj simbola u tabeli: "<<hex<<setw(8)<<setfill('0')<<kontekst->brojSimbolUTabeli<<endl;
  UlazTabeleSimbola* simb = kontekst->tabela_simbola;
  //jedan ulaz tabele simbola mora da ima redni broj simbola na 4B, zatim vrednost na 4B, 1B koji moze da ima vrednost 0 ili 1 (1 - ako je globalan), 1B koji moze da ima vrednost 0 ili 1 (1 - ako je sekcija)
  //1B koji moze da ima vrednost 0 ili 1 (1 - ako je uvezen), sekciju u kojoj se nalazi(4B) ako je extern bice 0, naziv koji se zavrsava sa '\0'
  while(simb){
    output<<"Redni broj: "<<hex<<setw(8)<<setfill('0')<<simb->rbr<<"  Vrednost: "<<hex<<setw(8)<<setfill('0')<<simb->vrednost<<" Sekcija: "<<hex<<setw(8)<<setfill('0')<<simb->sekcija;;
    if(simb->global){
      output<<" Global: "<<1;
    }else{
      output<<" Global: "<<0;
    }
    if(simb->sekcija_bool){
      output<<" Sekcija: "<<1;
    }else{
      output<<" Sekcija: "<<0;
    }
    if(simb->uvezen){
      output<<" Uvezen: "<<1;
    }else{
      output<<" Uvezen: "<<0;
    }
    output<<" Naziv: "<<simb->naziv<<endl;
    simb = simb->sledeci;
  }

  //sekcije -> svaka sekcija treba da sadrzi relokacione zapise za tu sekciju i onda sadrzaj sekcije
  output<<endl<<"#sekcije"<<endl;
  Sekcija* sekc = kontekst->sekcije;
  int br = 0;
  while(sekc){
    int brRelZapisa = 0;
    output<<"Sekcija: "<<br<<endl;
    br++;
    RelokacioniZapisElem* rel = kontekst->relokacioni_zapisi;
    while(rel){
      if(rel->sekcija == sekc->rbr){
        brRelZapisa++;
      }
      rel = rel->sledeci;
    }

    //prva stvar koja mora da se ispise je broj relokacionih zapisa na 4B
     output<<"Broj relokacionih zapisa: "<<hex<<setw(8)<<setfill('0')<<brRelZapisa<<endl;

    //zatim ide ispis tih relokacionih zapisa
    //ispisuje se prvo offset od pocetka sekcije (4B), zatim simbol(rbr -> 4B), zatim addend(ako je simbol lokalan to ce biti pomeraj u odnosu na pocetak sekcije u kojoj se simbol nalazi)
    rel = kontekst->relokacioni_zapisi;
    while(rel){
      if(rel->sekcija == sekc->rbr){
        output<<"Offset: "<<hex<<setw(8)<<setfill('0')<<rel->offset<<" Simbol: "<<hex<<setw(8)<<setfill('0')<<rel->simbol<<" Addend: "<<hex<<setw(8)<<setfill('0')<<rel->addend<<endl;
      }
      rel = rel->sledeci;
    }
    //nakon ovoga ispisujemo sadrzaj sekcije iz konteksta -> ovo su sada bajtovi koje sam unosila prilikom obade u asembleru
    //instr: od(4b) mod(4b) regA(4b) regB(4b) regC(4b) disp(12b)
    KontekstElem* kont = kontekst->kontekst_ostatak;
    while(kont){
      if(kont->sekcija == sekc->rbr){
        if(kont->ascii){
          output<<"Ascii: "<<kont->ascii_hex<<endl;
        }else if(kont->word){
          output<<"Word: "<<hex<<setw(8)<<setfill('0')<<kont->word_bajtovi<<endl;
          //output<<hex<<setw(8)<<setfill('0')<<kont->word_bajtovi;
        }else if(kont->skip){
          output<<"Skip: ";
          for(int i = 0; i < kont->skip_bajtovi; i++){
            output<<0<<0;
          }
          output<<endl;
        }else{
          output<<"Instrukcija: "<<kont->oc_instrukcije<<hex<<kont->mod<<hex<<kont->regA<<hex<<kont->regB<<hex<<kont->regC<<hex<<setw(3)<<setfill('0')<<kont->disp<<endl;
          //output<<hex<<kont->oc_instrukcije<<hex<<kont->mod<<hex<<kont->regA<<hex<<kont->regB<<hex<<kont->regC<<hex<<setw(3)<<setfill('0')<<kont->disp;
        }
      }
      
      kont = kont->sledeci;
    }
    //bazen literala te sekcije
    output<<"Bazen literala: "<<endl;
    BazeniLiterala* bazeni = kontekst->bazen_literala;
    while(bazeni){
      if(bazeni->sekcija == sekc->rbr){
        for(uint32_t el: bazeni->bazen){
          output<<hex<<setw(8)<<setfill('0')<<el<<endl;
        }
      }
      bazeni = bazeni->sledeci;
    }

    sekc = sekc->sledeca;
  }

  output.close();
}