#include "../../inc/linker/linker.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

Linker::Linker(){
  prvi_prolaz = true;
  this->trenutniFajl = nullptr;
  this->fajlovi = nullptr;
  this->brFajlova  = 0;
  this->sekcije = nullptr;
  this->tabela_simbola = nullptr;
  this->brojSimbola = 0;
  this->nedefinisani_simboli = nullptr;
  this->hex = true;
  this->relocatable = false;
  this->max_place = 0;
}

void Linker::procitaj_fajl(char* fajl){
  this->trenutniFajl = fajl;
  FILE* o_file = fopen(fajl, "r");

  if(!o_file){
    cerr<<endl<<"Greška pri otvaranju fajla"<<endl;
    exit(1);
  }

  this->dodaj_fajl(fajl, o_file);
  this->brFajlova++;

  fclose(o_file);
}

void Linker::postaviRelocatable(){
  this->hex = false;
  this->relocatable = true;
}

int Linker::charToInt(char c){
  switch (c)
  {
  case '0':
    return 0;
    break;
  case '1':
    return 1;
    break;
  case '2':
    return 2;
    break;
  case '3':
    return 3;
    break;
  case '4':
    return 4;
    break;
  case '5':
    return 5;
    break;
  case '6':
    return 6;
    break;
  case '7':
    return 7;
    break;
  case '8':
    return 8;
    break;
  case '9':
    return 9;
    break;
  case 'a':
    return 10;
    break;
  case 'b':
    return 11;
    break;
  case 'c':
    return 12;
    break;
  case 'd':
    return 13;
    break;
  case 'e':
    return 14;
    break;
  case 'f':
    return 15;
    break;
  default:
    break;
  }
  return -1;
}

void Linker::dodaj_fajl(char* fajl, FILE* o_fajl){
  Fajlovi* novi = new Fajlovi();
  novi->nazivFajla = fajl;
  novi->sledeci = nullptr;

  uint8_t* podaci;
  fseek(o_fajl, 0, SEEK_END);
  long size = ftell(o_fajl);
  fseek(o_fajl, 0, SEEK_SET);

  podaci = (uint8_t*)malloc(size + 1);

  fread(podaci, size, 1, o_fajl);  //ocadi su sad niz bajtova koji se citaju

  int i = 0;  //brojac kojim cemo da pristupano nizu

  //treba prvo procitati tabelu sekcija
  TabelaSekcija* tabela_sekcija = nullptr;
  uint16_t brSekcija = (charToInt(podaci[0])<<12) | (charToInt(podaci[1])<<8) | (charToInt(podaci[2])<<4) | charToInt(podaci[3]);
  i += 4;
  
  for(int j = 0; j < brSekcija; j++){
    //treba ucitati redni broj(2B), velicinu(2B), i naziv -> ucitava se iz fajla sve dok se ne naidje na '\0' tj 00 je taj bajt
    TabelaSekcija* nova = new TabelaSekcija();
    uint16_t rbr = (charToInt(podaci[i])<<12) | (charToInt(podaci[i+1])<<8) | (charToInt(podaci[i+2])<<4) | charToInt(podaci[i+3]);
    i+=4;
    uint16_t velicina = (charToInt(podaci[i])<<12) | (charToInt(podaci[i+1])<<8) | (charToInt(podaci[i+2])<<4) | charToInt(podaci[i+3]);
    i+=4;
    nova->velicina = velicina;
    nova->rbr = rbr;
    nova->sledeca = nullptr;

    //citanje naziva sekcije
    string nazivSekcije = "";

    while (true) {
      // Prvo uzimamo dve heksadecimalne cifre (1 bajt)
      uint8_t highNibble = charToInt(podaci[i]);
      uint8_t lowNibble = charToInt(podaci[i + 1]);
      // Sada formiramo bajt od dve nible vrednosti
      uint8_t byteValue = (highNibble << 4) | lowNibble;
      // Ako je bajt '00', završavamo čitanje
      if (byteValue == 0x00) {
        i += 2;
        break;
      }
      // Inače, konvertujemo bajt u char i dodajemo u naziv sekcije
      nazivSekcije += static_cast<char>(byteValue);
      // Pomeramo se za 2 heksadecimalne cifre unapred
      i += 2;
    }
    nova->naziv = nazivSekcije;


    if(tabela_sekcija == nullptr){
      tabela_sekcija = nova;
    }else{
      TabelaSekcija* pom = tabela_sekcija;
      while(pom->sledeca){
        pom = pom->sledeca;
      }
      pom->sledeca = nova;
    }
  }

  novi->tabela_sekcija = tabela_sekcija;

  //nakon tabele sekcija treba procitati  tabelu simbola
  TabelaSimbola* tabela_simbola = nullptr;
  //prvo treba procitati broj ulaza koji je napisan u 4B -> 8 hex cifara
  uint32_t brSimbola = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
  i += 8;
  
  for(int j = 0; j < brSimbola; j++){
    TabelaSimbola* simb = new TabelaSimbola();
    simb->sledeci = nullptr;
    simb->fajl = this->brFajlova;

    //citamo prvo redni broj na 4B
    uint32_t rbr = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
    i+=8;
    simb->rbr = rbr;
    //citamo vrednost na 4B
    uint32_t vrednost = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
    simb->vrednost = vrednost;
    i+=8;
    //zatim global kao jedna hex cifra
    int global = charToInt(podaci[i]);
    i++;
    if(global == 1){
      simb->global = true;
    }else{
      simb->global = false;
    }
    //zatim sekcija kao jedna hex cifra
    int sekcija_bool = charToInt(podaci[i]);
    i++;
    if(sekcija_bool == 1){
      simb->sekcija_bool = true;
    }else{
      simb->sekcija_bool = false;
    }
    //zatim uvezen kao jedna hex cifra
    int uvezen = charToInt(podaci[i]);
    i++;
    if(uvezen == 1){
      simb->uvezen = true;
    }else{
      simb->uvezen = false;
    }
    //zatim sekcija
    uint32_t sekcija = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
    simb->sekcija = sekcija;
    i+=8;
    //nakon toga naziv
    string nazivSimbola = "";

    while (true) {
      // Prvo uzimamo dve heksadecimalne cifre (1 bajt)
      uint8_t highNibble = charToInt(podaci[i]);
      uint8_t lowNibble = charToInt(podaci[i + 1]);
      // Sada formiramo bajt od dve nible vrednosti
      uint8_t byteValue = (highNibble << 4) | lowNibble;
      // Ako je bajt '00', završavamo čitanje
      if (byteValue == 0x00) {
        i += 2;
        break;
      }
      nazivSimbola += static_cast<char>(byteValue);
      // Pomeramo se za 2 heksadecimalne cifre unapred
      i += 2;
    }
    simb->naziv = nazivSimbola;

    if(tabela_simbola == nullptr){
      tabela_simbola = simb;
    }else{
      TabelaSimbola* pom = tabela_simbola;
      while(pom->sledeci){
        pom = pom->sledeci;
      }
      pom->sledeci = simb;
    }
  }

  novi->tabela_simbola = tabela_simbola;

  //zatim treba procitati sve sekcije njihov sadrzaj itd
  novi->sekcije = nullptr;
  for (int j = 0; j < brSekcija; j++){
    //citamo broj relokacionih zapisa
    Sekcija* sekcija = new Sekcija();
    sekcija->sledeca = nullptr;
    uint32_t brRel = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
    i += 8;
    //cout<<endl<<"Broj rel zapisa u fajlu: "<<novi->nazivFajla<<" "<<brRel;
    
    sekcija->relokacioni_zapisi = nullptr;
    
    for (int p = 0; p < brRel; p++){
      RelokacioniZapis* rel = new RelokacioniZapis();
      rel->sledeci = nullptr;
      rel->fajl = this->brFajlova;
      uint32_t offset = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
      i += 8;
      rel->offset = offset;
      uint32_t simbol = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
      i += 8;
      rel->simbol = simbol;
      uint32_t addend = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16)| (charToInt(podaci[i+4])<<12)| (charToInt(podaci[i+5])<<8)| (charToInt(podaci[i+6])<<4)| (charToInt(podaci[i+7]));
      i += 8;
      rel->addend = addend;
      if(sekcija->relokacioni_zapisi == nullptr){
        //cout<<endl<<"Dodajem prvi rel: "<<novi->nazivFajla;
        sekcija->relokacioni_zapisi = rel;
      }else{
        RelokacioniZapis* pom = sekcija->relokacioni_zapisi;
        while(pom->sledeci){
          pom = pom->sledeci;
        }
        pom->sledeci = rel;
      }
    }

    //nakon relokacionih zapisa treba procitati sadrzaj sekcije -> instrukcije plus bazen literala
    TabelaSekcija* sekc = novi->tabela_sekcija;
    int velicina = 0;
    string naziv = "";
    while(sekc){
      if(sekc->rbr == j){
        velicina = sekc->velicina;
        naziv = sekc->naziv;
      }
      sekc = sekc->sledeca;
    }
    sekcija->velicina = velicina;
    sekcija->naziv = naziv;
    sekcija->start = 0;

    for(int p = 0; p < velicina; p+=4){
      uint32_t sadrzaj = (charToInt(podaci[i])<<28) | (charToInt(podaci[i+1])<<24) | (charToInt(podaci[i+2])<<20) | (charToInt(podaci[i+3])<<16) | (charToInt(podaci[i+4])<<12) | (charToInt(podaci[i+5])<<8) | (charToInt(podaci[i+6])<<4) | (charToInt(podaci[i+7]));
      i+=8;
      sekcija->sadrzaj.push_back(sadrzaj);
    }

    if(novi->sekcije == nullptr){
      novi->sekcije = sekcija;
    }else{
      Sekcija* pom_sekc = novi->sekcije;
      while(pom_sekc->sledeca){
        pom_sekc = pom_sekc->sledeca;
      }
      pom_sekc->sledeca = sekcija;
    }

  }


  if(this->fajlovi == nullptr){
    this->fajlovi = novi;
  }else{
    Fajlovi* pom = this->fajlovi;
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Linker::dodajSimbol(TabelaSimbola* simbol, int fajl){
  TabelaSimbola* pom = this->tabela_simbola;
  bool vecBio = false;
  while(pom){
    if(simbol->naziv == pom->naziv){
      vecBio = true;
    }
    pom = pom->sledeci;
  }
  if(vecBio == false){
    this->brojSimbola++;
    TabelaSimbola* novi_ulaz = new TabelaSimbola();
    novi_ulaz->sledeci = nullptr;
    novi_ulaz->fajl = fajl;
    novi_ulaz->sekcija = simbol->sekcija;
    
    novi_ulaz->vrednost = simbol->vrednost; 
    novi_ulaz->rbr = this->brojSimbola;
    this->brojSimbola++;
    novi_ulaz->naziv = simbol->naziv;
    Fajlovi* f = this->fajlovi;
    int br = 0;
    while(f){
      if(br==fajl){
        break;
      }
      br++;
      f = f->sledeci;
    }
    TabelaSekcija* s = f->tabela_sekcija;
    while(s){
      if(s->rbr == simbol->sekcija){
        break;
      }
      s = s->sledeca;
    }
    novi_ulaz->naziv_sekcije = s->naziv;
    if(this->tabela_simbola == nullptr){
      this->tabela_simbola = novi_ulaz;
    }else{
      pom = this->tabela_simbola;
      while(pom->sledeci){
        pom = pom->sledeci;
      }
      pom->sledeci = novi_ulaz;
    }
    //nakon sto se doda definicija simbola on treba da se skloni iz nedefinisanih
    TabelaSimbola* nedef = this->nedefinisani_simboli;
    TabelaSimbola* prev = nullptr;
    
    while(nedef){
      if(nedef->naziv == simbol->naziv){
        if(prev != nullptr){
          prev->sledeci = nedef->sledeci;
        }else{
          this->nedefinisani_simboli = this->nedefinisani_simboli->sledeci;
        }
        break;
      }
      prev = nedef;
      nedef = nedef->sledeci;
    }
  }else{
    //znaci da postoji dvostruka definicija simbola i treba da se prekine izvrsavanje
    cerr<<endl<<"Linker greška: dvostruko definisan simbol"<<endl;
    exit(1);
  }
}

void Linker::dodajSekciju(Sekcija* sekcija){
  Sekcija* pom = this->sekcije;
  bool vecTu = false;
  while(pom){
    if(pom->naziv == sekcija->naziv){
      vecTu = true;
      break;
    }
    pom = pom->sledeca;
  }
  
  if(vecTu){
    //ako je sekcija vec tu treba da se odradi kao merge sa postojecom sekcijom -> da se dodaju relokacioni zapisi i da se doda sadrzaj druge sekcije
    if(pom->naziv == sekcija->naziv){
      pom->velicina += sekcija->velicina;
      if(pom->velicina > 4096){  //sekcija ne moze da bude veca od 2 na 12 zbog disp
        cerr<<endl<<"Linker greška: Velicina sekcije je prevelika. Sekcija: "<<pom->naziv<<endl;
        exit(1);
      }
      for(uint32_t el: sekcija->sadrzaj){
        pom->sadrzaj.push_back(el);
      }
      
        
      RelokacioniZapis* rel = pom->relokacioni_zapisi;
      if(rel != nullptr){
        while(rel->sledeci){
          rel = rel->sledeci;
        }
        rel->sledeci = sekcija->relokacioni_zapisi;
      }else{
        pom->relokacioni_zapisi = sekcija->relokacioni_zapisi;
      }
    }
    
  }else{
    //ako nije tu dodajemo je prvi put
    Sekcija* noviUlaz = new Sekcija();
    noviUlaz->sledeca = nullptr;
    noviUlaz->naziv = sekcija->naziv;
    noviUlaz->relokacioni_zapisi = sekcija->relokacioni_zapisi;
    noviUlaz->sadrzaj = sekcija->sadrzaj;
    noviUlaz->start = 0;
    noviUlaz->velicina = sekcija->velicina;
    pom = this->sekcije;
    if(pom == nullptr){
      this->sekcije = noviUlaz;
    }else{
      while(pom->sledeca){
        pom = pom->sledeca;
      }
      pom->sledeca = noviUlaz;
    }
    TabelaSimbola* s = new TabelaSimbola();
    s->definisan =  true;
    s->global = true;
    s->naziv = sekcija->naziv;
    s->naziv_sekcije = sekcija->naziv;
    s->sekcija_bool = true;
    s->rbr = this->brojSimbola++;
    s->vrednost = 0;
    s->uvezen = false;
    s->sledeci = nullptr;
    TabelaSimbola* simb = this->tabela_simbola;
    if(simb==nullptr){
      this->tabela_simbola = s;
    }else{
      while(simb->sledeci){
        simb = simb->sledeci;
      }
      simb->sledeci = s;
    }
    this->brojSekcija++;
  }
}

void Linker::dodajNedefinisan(TabelaSimbola* simbol, int fajl){
  //ova metoda treba da doda simbol ako on stvarno nije definisan
  bool def = false;
  TabelaSimbola* ulaz = this->tabela_simbola;
  while(ulaz){
    if(simbol->naziv == ulaz->naziv){
      def = true;
    }
    ulaz = ulaz->sledeci;
  }
  if(def == false){
    TabelaSimbola* noviUlaz = new TabelaSimbola();
    noviUlaz->fajl = fajl;
    noviUlaz->sledeci = nullptr;
    noviUlaz->naziv = simbol->naziv;
    noviUlaz->rbr = simbol->rbr;
    noviUlaz->sekcija = simbol->sekcija;
    if(this->nedefinisani_simboli == nullptr){
      this->nedefinisani_simboli = noviUlaz;
    }else{
      TabelaSimbola* pom = this->nedefinisani_simboli;
      while(pom->sledeci){
        pom = pom->sledeci;
      }
      pom->sledeci = noviUlaz;
    }
  }
}

void Linker::kreiranjeSekcija(){
  //ova metoda treba da prodje kroz sve sekcije u fajlovima i da napravi internu struktruru gde su sve skcije spojene
  //pre nego sto to uradi treba da prodje kroz tabelu simbola svakog fajla i da napravi svoju tabelu simbola gde ce da prati simbole koji su definisani i koji nisu
  //ako na kraju ove metode postoji simbol koji nije definisan ili se naidje na dvostruku definiciju nekog simbola linker prijavljuje gresku
  Fajlovi* fajlovi = this->fajlovi;
  int fajl = 0;
  while(fajlovi){
    //prvo treba proci kroz tabelu simbola fajla i probati da se doda simbol po simbol ako je globalan ili sekcija ako je sekcija
    TabelaSimbola* simb = fajlovi->tabela_simbola;
    while(simb){

      if(simb->sekcija_bool){  //treba da se doda sekcija ako vec ne postoji u linkerovoj tabeli sekcija
        //ako naidjemo na sekciju slacemo ulaz iz tabele sekcija
        Sekcija* pom = fajlovi->sekcije;
        while(pom){
          if(pom->naziv == simb->naziv){
            break;
          }
          pom = pom->sledeca;
        }
        this->dodajSekciju(pom);
      }else if(simb->global){  //globalan simbol treba da se doda u tabelu simbola
        this->dodajSimbol(simb, fajl);
      }else if(simb->uvezen){  //ovaj simbol nije definisan u tabeli pa ga necemo dodati u tabelu simbola vec u nedefinisane simbole
        this->dodajNedefinisan(simb, fajl);
      }

      simb = simb->sledeci;
    }


    fajlovi = fajlovi->sledeci;
    fajl++;
  }
  //nakon prolaska kroz sve fajlove ne bi trebalo da postoji ni jedan nedefinisan simbol
  if(this->nedefinisani_simboli != nullptr){
    cerr<<endl<<"Linker greška: Nepostojeća definicija simbola."<<endl;
    exit(1);
  }
}

void Linker::dodajPocetakSekcije(uint32_t pocetak, string naziv){
  Place* pom = this->mesta_pocetka;
  bool vecTu = false;
  while(pom){
    if(pom->naziv == naziv){
      vecTu = true;
      break;
    }
    pom = pom->sledeca;
  }
  if(vecTu){
    pom->start = pocetak;
  }else{
    Place* novi = new Place();
    novi->sledeca = nullptr;
    novi->naziv = naziv;
    novi->start = pocetak;
    if(this->mesta_pocetka == nullptr){
      this->mesta_pocetka = novi;
    }else{
      pom = this->mesta_pocetka;
      while(pom->sledeca){
        pom = pom->sledeca;
      }
      pom->sledeca = novi;
    }
  }
  if(this->max_place < pocetak){
    this->max_place = pocetak;
  }
}

void Linker::namestiPocetkeSekcijama(){
  Place* pom = this->mesta_pocetka;
  while(pom){
    Sekcija* sekc = this->sekcije;
    while(sekc){
      if(sekc->naziv == pom->naziv){
        sekc->start = pom->start;
        if(sekc->start == this->max_place){
          this->max_place = sekc->start + sekc->velicina;  //odavde ce da idu ostale sekcije koje nemaju start podesen preko place
          RelokacioniZapis* rz = sekc->relokacioni_zapisi;
          int brZapisa = 0;
          while(rz){
            brZapisa = brZapisa + 1;
            rz = rz->sledeci;
          }
          this->max_place = this->max_place + (brZapisa * 4);
          if(this->max_place % 8){  //zbog poravnanja tj pocetak sekcije treba da bude poravnat sa 8 jer je zgodnije tako u hex fajl da se upise
            this->max_place += 4;
          }
        }
        break;
      }
      sekc = sekc->sledeca;
    }
    pom = pom->sledeca;
  }
}

void Linker::sortirajSekcije(){
  //treba prvo postaviti pocetke sekcijam kojima je start i dalje na nuli
  Sekcija* tr = this->sekcije;
  while(tr && this->hex){
    if(tr->start == 0){
      tr->start = this->max_place;
      this->max_place = tr->start + tr->velicina;
      RelokacioniZapis* rz = tr->relokacioni_zapisi;
      int brZapisa = 0;
      while(rz){
        brZapisa = brZapisa + 1;
        rz = rz->sledeci;
      }
      this->max_place = this->max_place + (brZapisa * 4);
      if(this->max_place % 8){  //zbog poravnanja
        this->max_place += 4;
      }
    }

    tr = tr->sledeca;
  }

  // Prazna ili samo jedna sekcija - vec je sortirano
    if (!this->sekcije || !this->sekcije->sledeca) {
        return;
    }

    // Nova početna tačka za sortirane sekcije
    Sekcija* sortirana = nullptr;

    // Prođi kroz originalnu listu i ubacuj sekcije u sortiranu listu
    Sekcija* trenutna = this->sekcije;
    while (trenutna) {
        // Sledeća sekcija pre nego što ubacimo trenutnu u sortiranu listu
        Sekcija* sledeca = trenutna->sledeca;

        // Ubacivanje trenutne sekcije u sortiranu listu
        if (!sortirana || sortirana->start > trenutna->start) {
            // Ubaci na početak liste
            trenutna->sledeca = sortirana;
            sortirana = trenutna;
        } else {
            // Pronađi mesto za ubacivanje unutar sortirane liste
            Sekcija* current = sortirana;
            while (current->sledeca && current->sledeca->start <= trenutna->start) {
                current = current->sledeca;
            }
            // Ubaci trenutnu sekciju na pravo mesto
            trenutna->sledeca = current->sledeca;
            current->sledeca = trenutna;
        }

        // Pređi na sledeću sekciju u originalnoj listi
        //cout<<endl<<trenutna->naziv<<" "<<std::hex<<trenutna->start<<" "<<std::hex<<trenutna->velicina;
        trenutna = sledeca;
    }

    // Ažuriraj glavu liste
    this->sekcije = sortirana;

    /*trenutna = this->sekcije;
    sortirana = nullptr;
    while(trenutna && this->hex){  //ako se radi relocatable treba sekcije da ostanu sve na nula
      if(trenutna->start == 0){
        if(sortirana != nullptr){
          //treba proci kroz sve relokacione zapise sortirane sekcije i dodati 4B * br na velicinu sekcije da odatle krece trenutna
          RelokacioniZapis* rel = sortirana->relokacioni_zapisi;
          int br = 0;
          while(rel){
            br++;
            rel = rel->sledeci;
          }
          trenutna->start = sortirana->start + sortirana->velicina + (4 * br);
          if(trenutna->start % 8){  //zbog poravnanja
            trenutna->start += 4;
          }
        }
      }
      //cout<<endl<<trenutna->naziv<<" "<<std::hex<<trenutna->start<<" velicina: "<<std::hex<<trenutna->velicina;
      sortirana = trenutna;
      trenutna = trenutna->sledeca;
    }*/

    //ostalo je jos da proverimo preklapanja sekcija -> ako se preklapaju treba da se prijavi greska
    trenutna = this->sekcije;
    while(trenutna){
      uint32_t krajJedne = trenutna->start + trenutna->velicina;
      if(trenutna->sledeca){
        if(krajJedne > trenutna->sledeca->start){
          cerr<<endl<<"Greška: Preklapanje sekcija"<<endl;
          exit(1);
        }
      }
      trenutna = trenutna->sledeca;
    }
}

void Linker::odrediVrednostiSimbolima(){
  TabelaSimbola* simbol = this->tabela_simbola;
  while(simbol){
    string sekcija = simbol->naziv_sekcije;

    uint32_t vrednost = 0;
    Sekcija* s = this->sekcije;
    while(s){
      if(s->naziv == sekcija){
        break;
      }
      s = s->sledeca;
    }
    vrednost = s->start;

    Fajlovi* f = this->fajlovi;
    int br = 0;
    while(f){
      if(br == simbol->fajl){
        break;
      }

      TabelaSekcija* skc = f->tabela_sekcija;
      while(skc){
        if(skc->naziv == sekcija){
          vrednost += skc->velicina;
        }
        skc = skc->sledeca;
      }

      br++;
      f = f->sledeci;
    }
    vrednost += simbol->vrednost;  //na kraju dodajemo vrednost koja je trenutno pomeraj u okviru fajla u kom je definisan
    simbol->vrednost = vrednost;
    //cout<<endl<<"Simbol "<<simbol->naziv<<" vrednost: "<<std::hex<<simbol->vrednost<< " sekcija "<<sekcija<<" start "<<std::hex<<s->start;

    simbol = simbol->sledeci;
  }
}

void Linker::razresiRelokacioneZapise(){
  //ova metoda treba da prodje kroz relokacione zapise svih sekcija i da ih razresi
  //u ovom trenutku sekcije još nisu postavljene na place i idalje se tretira kao da svaka krece od nule
  //u svakom relokacionom zapisu pise iz kog je fajla pa se pomeraj do simbola racuna kao zbir velicina sekcije iz svih fajlova pre tog plus pomeraj do tog simbola u tabeli simbola
  //relokacioni zapis treba nakon razresavanja da se obrise iz liste da bi na kraju za relocatable ostali samo zapisi koji treba da ostanu
  Sekcija* sekcija = this->sekcije;
  while(sekcija){
    //treba u svakoj sekciji razresiti relokacione zapise
    RelokacioniZapis* zapis = sekcija->relokacioni_zapisi;
    RelokacioniZapis* prev = nullptr;
    while(zapis){
      int fajl_br = zapis->fajl;  //fajl u kom se nalazi mesto koje se razresava
      //prvo treba odrediti poziciju mesta
      int br = 0;
      Fajlovi* fajl = this->fajlovi;
      while(fajl){   //nakon petlje fajl ce da bude struktura fajla u kom treba da se uradi relokacija
        if(br == fajl_br){
          break;
        }
        br++;
        fajl = fajl->sledeci;
      }

      uint32_t vrednostSimbola = zapis->addend;  //na pocetku je pomeraj addend  -> tj ovo je destinacija tj trazeni simbol
      string nazivDrugeSekcije = "";
      //sada treba da se prodje kroz tabelu simbola fajla da se nadje naziv simbola i da se onda u tabeli simbola nadje vrednost tog simbola koji se trazi i doda na trenutni pomeraj tj addend
      if(fajl->tabela_simbola != nullptr){
        TabelaSimbola* simboli = fajl->tabela_simbola;
        while(simboli){
          if(simboli->rbr == zapis->simbol){
            break;
          }
          simboli = simboli->sledeci;
        }
        TabelaSimbola* svi_simb = this->tabela_simbola;
        while(svi_simb){
          if(simboli != nullptr){
            if(svi_simb->naziv == simboli->naziv){
              //cout<<endl<<" simbol koji se razresava: "<<svi_simb->naziv<<" vrednost "<<std::hex<<svi_simb->vrednost;
              break;
            }
          }
          svi_simb = svi_simb->sledeci;
        }
        if(svi_simb != nullptr){
          vrednostSimbola = vrednostSimbola + svi_simb->vrednost;  //ovo ce na kraju ove linije da pokazuje na simbol u toj sekciji u kojoj se on nalazi
          nazivDrugeSekcije = svi_simb->naziv_sekcije;
        }
        
      }

      if(nazivDrugeSekcije == sekcija->naziv || (nazivDrugeSekcije != sekcija->naziv && this->hex)){ 
        uint32_t mesto_upisa = zapis->offset;  //zapis je inicijalno offset i pokazuje na kojoj instrukciji treba da se radi upis
        //pomeraj tacan cemo da dobijemo tako sto na mesto upisa dodajemo velicine sekcija koje se zovu kao ova sekcija dok ne dodjemo do fajla u kom se nalazi upis
        Sekcija* sek = this->sekcije;
        mesto_upisa += sekcija->start;
        int cnt = 0;
        fajl = this->fajlovi;
        while(fajl){
          if(cnt==zapis->fajl){
            break;
          }
          TabelaSekcija* s = fajl->tabela_sekcija;
          while(s){
            if(s->naziv == sekcija->naziv){
              mesto_upisa = mesto_upisa + s->velicina;
              break;
            }
            s = s->sledeca;
          }
          cnt++;
          fajl = fajl->sledeci;
        }
        //nakon ovoga je formirano mesto upisa i treba da se prepravi disp u toj instrukciji
        int lc = sekcija->start;
        for(uint32_t& el: sekcija->sadrzaj){
          if(lc == mesto_upisa){ //ovde treba da se upise
            //prvo proveravamo dal je el 0 jer ako jeste onda treba da se upise vrednost simbola samo u element
            if(el == 0){
              el = vrednostSimbola;
              lc += 4;
              continue;  //da se dalje nista ne izvrsi
            }
            //pomeraj do mesta ovog ce da bude
            uint32_t disp = vrednostSimbola - (mesto_upisa + 4);   //pomeraj je isto sto i vrednost simbola - mora +4 jer pc uvek cim se procita instrukcija pokazuje na sledecu instrukciju
            if(disp >= -2048 && disp <= 2047){  //ako je pomeraj do 12bita moze da se upise odmah
              el = (el & 0xFFFFF000) | (disp & 0xFFF);
              //cout<<" dobar pomeraj: "<<std::hex<<disp;
            }else{  //ako ne treba da se doda u bazen literala vrednost tog simbola
              //treba ovoj sekciji i svim sekcijama koje pocinju na njenom kraju povecati velicinu za 4 i svim simbolima iz tih sekcija vrednosti za 4
              
              //cout<<endl<<"el: "<<std::hex<<el;
              
              uint16_t disp1 = sekcija->start + sekcija->velicina - (mesto_upisa + 4);
              if(disp1 >= -2048 && disp1 <= 2047){  //ako je pomeraj do 12bita moze da se upise odmah
                el = (el & 0xFFFFF000) | (disp1 & 0xFFF);
                //cout<<endl<<" lc: "<<std::hex<<mesto_upisa<<" disp "<<std::hex<<disp1<<" el: "<<std::hex<<el;
              }else{
                cerr<<endl<<"Sekcija veca od 2^12"<<endl;
                exit(1);
              }
              sekcija->sadrzaj.push_back(vrednostSimbola);
              //cout<<" Push back simbola"<<" : "<<std::hex<<vrednostSimbola;
              sekcija->velicina += 4;
              
            }
            
            break;
          }
          lc+=4;
        }
        //razresen relokacioni zapis treba da se ukloni
        if(prev != nullptr){
          prev->sledeci = zapis->sledeci;
        }else{
          sekcija->relokacioni_zapisi = zapis->sledeci;
        }
      
      }else{
        //u ovom slucaju se radi relocatable pa onda treba da ostane relokacioni zapis samo treba da se simbol i offset
        //offset se odredjuje isto kao mesto upisa a simbol ce da bude id iz tabele simbola
        uint32_t mesto_upisa = zapis->offset;
        Sekcija* sek = this->sekcije;
        mesto_upisa += sekcija->start;
        int cnt = 0;
        fajl = this->fajlovi;
        while(fajl){
          if(cnt==zapis->fajl){
            break;
          }
          TabelaSekcija* s = fajl->tabela_sekcija;
          while(s){
            if(s->naziv == sekcija->naziv){
              mesto_upisa = mesto_upisa + s->velicina;
              break;
            }
            s = s->sledeca;
          }
          cnt++;
          fajl = fajl->sledeci;
        }
        zapis->offset = mesto_upisa;

        //sada treba naci redni broj simbola iz zapisa u globalnoj tabeli simbola
        fajl = this->fajlovi;
        while(fajl){   //nakon petlje fajl ce da bude struktura fajla u kom treba da se uradi relokacija
          if(br == fajl_br){
            break;
          }
          br++;
          fajl = fajl->sledeci;
        }
        //sada treba da se prodje kroz tabelu simbola fajla da se nadje naziv simbola i da se onda u tabeli simbola nadje vrednost tog simbola koji se trazi i doda na trenutni pomeraj tj addend
        if(fajl->tabela_simbola != nullptr){
          TabelaSimbola* simboli = fajl->tabela_simbola;
          while(simboli){
            if(simboli->rbr == zapis->simbol){
              break;
            }
            simboli = simboli->sledeci;
          }
          TabelaSimbola* svi_simb = this->tabela_simbola;
          while(svi_simb){
            if(simboli != nullptr){
              if(svi_simb->naziv == simboli->naziv){
                break;
              }
            }
            svi_simb = svi_simb->sledeci;
          }
          if(svi_simb != nullptr){
            zapis->simbol = svi_simb->rbr;
          }
        }
      }
      prev = zapis;
      zapis = zapis->sledeci;
    }
    sekcija = sekcija->sledeca;
  }
}

void Linker::formirajHexFajl(char* fajl){
  ofstream izlaz(fajl);

  if(!izlaz){
    cerr<<endl<<"Greška pri formiranju izlaznog hex fajla"<<endl;
    exit(1);
  }

  //format fajla treba da bude 
  // 0000: 00 01 02 03 04 05 06 07
  // 0008: 08 09 0a 0b 0c 0d 0e 0f
  // 0010: 10 11 12 13 14 15 16 17 

  Sekcija* sekcija = this->sekcije;
  bool pocetak = true;
  while(sekcija){
    uint32_t lc = sekcija->start;
    
    for(uint32_t el: sekcija->sadrzaj){
      if((lc % 8) == 0){
        if(pocetak == false){
          izlaz<<endl;
        }else{
          pocetak = false;
        }
        izlaz<<std::hex<<setw(8)<<setfill('0')<<lc<<": ";
      }
      //izlaz<<std::hex<<setw(2)<<setfill('0')<<((el & 0xff000000)>>24)<<" "<<std::hex<<setw(2)<<setfill('0')<<((el & 0x00ff0000)>>16)<<" " <<std::hex<<setw(2)<<setfill('0')<< ((el & 0x0000ff00)>>8)<<" "<<std::hex<<setw(2)<<setfill('0') << (el & 0x000000ff);
      // Ispis u little endian formatu
      izlaz<<std::hex<<setw(2)<<setfill('0')<<(el & 0x000000ff)<<" "
           <<std::hex<<setw(2)<<setfill('0')<<((el & 0x0000ff00) >> 8)<<" "
           <<std::hex<<setw(2)<<setfill('0')<<((el & 0x00ff0000) >> 16)<<" "
           <<std::hex<<setw(2)<<setfill('0')<<((el & 0xff000000) >> 24);
      if((lc % 8) == 0){
        izlaz<<" ";
      }
      lc+=4;

    }

    sekcija = sekcija->sledeca;
  }

  izlaz.close();
}

void Linker::formirajOFajl(char* fajl){
  ofstream output;
  output.open(fajl);

  if(!output){
    cerr<<endl<<"Greška pri formiranju izlaznog ojektnog fajla"<<endl;
    exit(1);
  }

  //redosled upisivanja
  //treba prvo tabela sekcija
  //sto se tabele sekcija tice odvojicu 2B
  //jedan ulaz treba da sadrzi identifikator sekcije(2B) i njenu velicinu(2B) i naziv sekcije (1 karakter 1 bajt -> 12B - maksimalno 12 karaktera)
  //output<<"#tabela sekcija"<<endl;
  output<<std::hex<<setw(4)<<setfill('0')<<this->brojSekcija;  //broj ulaza u tabelu
  Sekcija* pom = this->sekcije;
  int br = 0;
  while(pom){
    output<<std::hex<<setw(4)<<setfill('0')<<br<<std::hex<<setw(4)<<setfill('0')<<pom->velicina;  //identifikator i velicina
    // naziv kao heksadecimalna reprezentacija
    std::stringstream hexStream;
    for (unsigned char ch : pom->naziv) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
    }

    // dodaj nulu za kraj stringa ('\0')
    hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>('\0');

    output << hexStream.str();  // ispiši heksadecimalne bajtove
    
    pom = pom->sledeca;
    br++;
  }

  //tabela simbola
  //output<<endl<<"#tabela simbola"<<endl;
  //tabela simbola treba da ima prvo broj ulaza na 4B
  output<<std::hex<<setw(8)<<setfill('0')<<this->brojSimbola;
  TabelaSimbola* simb = this->tabela_simbola;
  //jedan ulaz tabele simbola mora da ima redni broj simbola na 4B, zatim vrednost na 4B, 1B koji moze da ima vrednost 0 ili 1 (1 - ako je globalan), 1B koji moze da ima vrednost 0 ili 1 (1 - ako je sekcija), 
  //1B koji moze da ima vrednost 0 ili 1 (1 - ako je uvezen), sekciju u kojoj se nalazi(4B) ako je extern bice 0, naziv koji se zavrsava sa '\0'
  while(simb){
    output<<std::hex<<setw(8)<<setfill('0')<<simb->rbr<<std::hex<<setw(8)<<setfill('0')<<simb->vrednost;
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
    std::stringstream hexStream;
    for (unsigned char ch : simb->naziv) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
    }

    // dodaj nulu za kraj stringa ('\0')
    hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>('\0');

    output << hexStream.str();  // ispiši heksadecimalne bajtove
    
    simb = simb->sledeci;
  }

  //ispis sekcija

  Sekcija* s = this->sekcije;
  while(s){
    RelokacioniZapis* rel = s->relokacioni_zapisi;
    br = 0;
    while(rel){
      rel = rel->sledeci;
      br++;
    }
    //prva stvar koja mora da se ispise je broj relokacionih zapisa na 4B
    output<<std::hex<<setw(8)<<setfill('0')<<br;
    //nakon ovog ispisujemo rel zapise
    rel = s->relokacioni_zapisi;
    while(rel){
      output<<std::hex<<setw(8)<<setfill('0')<<rel->offset<<std::hex<<setw(8)<<setfill('0')<<rel->simbol<<std::hex<<setw(8)<<setfill('0')<<rel->addend;
      rel = rel->sledeci;
    }
    //nakon ovoga ide sadrzaj
    for(uint32_t el: s->sadrzaj){
      output<<std::hex<<setw(8)<<setfill('0')<<el;
    }

    s = s->sledeca;
  }

  
}

Linker::~Linker() {
    // Oslobađanje memorije za trenutniFajl, ako je dodeljena
    if (trenutniFajl != nullptr) {
        delete[] trenutniFajl;
    }

    // Oslobađanje memorije za sekcije
      Sekcija* tr = this->sekcije;
      Sekcija* prev = nullptr;
      while(tr){
        prev = tr;
        tr = tr->sledeca;
        delete prev;
      }
    

    // Oslobađanje memorije za tabelu simbola
    TabelaSimbola* t = this->tabela_simbola;
    TabelaSimbola* pret = nullptr;
    while(t){
      pret = t;
      t = t->sledeci;
      delete pret;
    }

    // Oslobađanje memorije za mesta_pocetka
    Place* mesto = this->mesta_pocetka;
    Place* p = nullptr;
    while(mesto){
      p = mesto;
      mesto = mesto->sledeca;
      delete p;
    }

    // Oslobađanje memorije za fajlovi
    Fajlovi* f = this->fajlovi;
    Fajlovi* pf = nullptr;
    while(f){
      pf = f;
      f = f->sledeci;
      delete pf;
    }
}
