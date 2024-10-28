#include "../../inc/asembler/kontekst.hpp"

Kontekst:: Kontekst(){
  this->kraj = false;
  this->trenutnaSekcija = 0;    // 0 je UND
  this->locationCounter = 0;
  this->brojSimbolUTabeli = 1;
  this->brojSekcija = 1;
  this->brojRelokacionihZapisa = 0;

  this->kontekst_ostatak = nullptr;
  //this->tabela_simbola = nullptr;
  this->tabela_simbola = new UlazTabeleSimbola();
  this->tabela_simbola->definisan = true;
  this->tabela_simbola->global = true;
  this->tabela_simbola->naziv = "UNDF";
  this->tabela_simbola->rbr = 0;
  this->tabela_simbola->sekcija = 0;
  this->tabela_simbola->sekcija_bool = true;
  this->tabela_simbola->sledeci = nullptr;
  this->tabela_simbola->uvezen = false;
  this->tabela_simbola->vrednost = 0;
  this->sekcije = new Sekcija();   //na pocetku postoji nedefinisana sekcija
  this->sekcije->naziv = "UNDF";
  this->sekcije->rbr = 0;
  this->sekcije->sledeca = nullptr;
  this->bazen_literala = nullptr;
  this->tabela_literala = nullptr;
  this->tabela_obracanja_unapred = nullptr;
  this->relokacioni_zapisi = nullptr;
}  

void Kontekst::dodajSimbol(char* simbol, uint32_t adresa, bool global, bool definisan, bool uvezen, bool sekcija){  //adresa je location cout kada dodjem do te 
  //prvo treba proveriti da li je labela mozda sekcija ako jeste znaci da se prelazi sa jedne na drugu sekciju
  //ako ne znaci da je u pitanju nov simbol koji treba da se doda u tabelu simbola ko vec ne postoji u njoj
  //ako postoji u tabeli simbola onda treba da se upise adresa simbola i da se simbolo odredi kao definisan
  UlazTabeleSimbola* pom = this->tabela_simbola;
  if(pom == nullptr){
    UlazTabeleSimbola* novi = new UlazTabeleSimbola();
    novi->definisan = definisan;
    novi->global = global;
    novi->naziv = simbol;
    novi->vrednost = adresa;
    novi->sledeci = nullptr;
    this->tabela_simbola = novi;
    novi->rbr = 0;
    novi->uvezen = uvezen;
    novi->sekcija_bool = sekcija;

    if(novi->definisan){
      novi->sekcija = this->trenutnaSekcija;
    }
    if(novi->uvezen){
      novi->sekcija = 0;
    }
    this->brojSimbolUTabeli++;

    return;
  }

  bool vecBio = false;
  
  while(pom){
    if(!strcmp(pom->naziv, simbol)){
      if(pom->definisan){  //pri dvostrukoj definiciji simbola asembler treba da prijavi gresku
        cerr<<endl<<"Dvostruka definicija simbola"<<endl;
        exit(1);
      }
      pom->definisan = definisan;
      pom->vrednost = adresa;
      pom->sekcija = this->trenutnaSekcija;
      vecBio = true;
    }
    pom = pom->sledeci;
  }

  if(!vecBio){
    UlazTabeleSimbola* novi = new UlazTabeleSimbola();
    novi->definisan = definisan;
    novi->global = global;
    novi->naziv = simbol;
    novi->vrednost = adresa;
    novi->sledeci = nullptr;
    pom = this->tabela_simbola;
    novi->uvezen = uvezen;
    novi->sekcija_bool = sekcija;

    int br = 0;
    while(pom->sledeci){
      br++;
      pom = pom->sledeci;
    }
    novi->rbr = br + 1;
    if(novi->definisan){
      novi->sekcija = this->trenutnaSekcija;
    }
    if(novi->uvezen){
      novi->sekcija = 0;
    }
    pom->sledeci = novi;
    this->brojSimbolUTabeli++;
  }

}

void Kontekst::dodajSekciju(char* naziv){   //ova metoda se poziva kada se naleti na .section direktivu. mora da se proveri prvu da li je sekcija kao takva dodata u tabelu simbola
  //ako sekcija nije dodata u tabelu simbola treba da je dodamo ako jeste onda je sve sto treba da uradimo da promenimo identifikator trenutne sekcije
  //izbacujem sekciju iz tabele simbola
  if(this->sekcije != nullptr){  //treba da napisemo velicinu ove sekcije trenutne
    Sekcija* pom2 = this->sekcije;
    while(pom2){
      if(this->trenutnaSekcija == pom2->rbr){
        pom2->velicina = this->locationCounter;
      }
      pom2 = pom2->sledeca;
    }
  }
  bool vecPostoji = false;
  Sekcija* pom2 = this->sekcije;
  while(pom2){
    if(!strcmp(naziv, pom2->naziv)){
      this->locationCounter = pom2->velicina;  // ako se nastavlja neka sekcija koja je prethodno zapoceta pa je predjeno na drugu
      vecPostoji = true;
      this->trenutnaSekcija = pom2->rbr;
    }
    pom2 = pom2->sledeca;
  }

  if(vecPostoji == false){  //treba da se doda sekcija u tabelu simbola
    
    this->brojSekcija++;

    Sekcija* nova = new Sekcija();
    nova->naziv = naziv;
    nova->sledeca = nullptr;
    nova->velicina = 0;
    this->locationCounter = 0;
    if(this->sekcije != nullptr){
      Sekcija* pom1 = this->sekcije;
      while(!strcmp(pom1->naziv, naziv)){
        pom1 = pom1->sledeca;
      }
      pom1->velicina = this->locationCounter;
    }

    if(this->sekcije == nullptr){ //prva sekcija -> ovde ne bi trebalo da se udje jer tabela sekcija u startu ima nultu nedefinisanu sekciju
      nova->rbr = 0;
      this->sekcije = nova;
    }else{   //ako nije prva sekcija
      nova->rbr = this->brojSekcija - 1;
      Sekcija* pom1 = this->sekcije;
      while(pom1->sledeca){
        pom1 = pom1->sledeca;
      }
      pom1->sledeca = nova;
    }
    this->trenutnaSekcija = nova->rbr;
    this->dodajSimbol(naziv, 0, true, true, false, true);
  }
}

void Kontekst::dodajGlobal(Simboli& sim){
  //ova direktiva treba da u tabelu simbola ubaci sve simbole koji se nalaze u nizu simboli, a ako se simbol vec nalazi u tabeli simbola onda mora da ga oznaci kao globalan
  //isto treba voditi racuna da simboli koji su naznaceni pod global NISU DEFINISANI U OVOM TRENUTKU
  Simbol* simboli = sim.simboli;
  int brojSimbola = sim.duzina;
  for(int i = 0; i < brojSimbola; i++){
    Simbol tr = simboli[i];
    this->dodajSimbol(tr.simbol, 0, true, false, false, false);
  }
}

void Kontekst::dodajExtern(Simboli& sim){
  Simbol* simboli = sim.simboli;
  int brojSimbola = sim.duzina;

  for(int i = 0; i < brojSimbola; i++){
    Simbol tr = simboli[i];
    this->dodajSimbol(tr.simbol, 0, false, false, true, false);
  }
}

bool Kontekst::simbolUTabeli(char* simbol){
  UlazTabeleSimbola* pom = this->tabela_simbola;
  bool ret = false;
  while(pom){
    if(!strcmp(pom->naziv, simbol) && pom->definisan){
      ret = true;
    }
    pom = pom->sledeci;
  }
  return ret;
}

int Kontekst::sekcijaSimbola(char* simbol){
  UlazTabeleSimbola* pom = this->tabela_simbola;
  int ret = 0;
  while(pom){
    if(!strcmp(pom->naziv, simbol)){
      ret = pom->sekcija;
    }
    pom = pom->sledeci;
  }
  return ret;
}

uint32_t Kontekst::vrednostSimbola(char* simbol){
  UlazTabeleSimbola* pom = this->tabela_simbola;
  uint32_t ret = 0;
  while(pom){
    if(!strcmp(pom->naziv, simbol) && pom->definisan){
      ret = pom->vrednost;
    }
    pom = pom->sledeci;
  }
  return ret;
}

void Kontekst::dodajWord(Simboli& sim){
  // ova direktiva treba da ako je simbol napravi relokacioni zapis ili bekpecing zapis ako simbol nije definisan
  // a ako jeste definisan samo upise na 4B vrednost tog simbola
  // a ako je literal samo da upise taj literal na 4B
  // posto ovo utice na masinski kod kao kod skip stavljamo to  4B
  Simbol* simboli = sim.simboli;
  int brojSimbola = sim.duzina;
  for(int i = 0; i < brojSimbola; i++){
    if(simboli[i].vrsta == 0){    //ako je simbol
    //treba svakako da se doda relokacioni zapis jer vrednost konacna simbola ne moze biti poznata dok ne zavrsi sa povezivanjem

      KontekstElem* novi = new KontekstElem();
      novi->word = true;
      novi->word_bajtovi = 0;
      novi->skip = false;
      novi->skip_bajtovi = 0;
      novi->disp = 0;
      novi->oc_instrukcije = 0;
      novi->mod = 0;
      novi->regA = 0;
      novi->regB = 0;
      novi->regC = 0;
      novi->sekcija = this->trenutnaSekcija;
      novi->ascii = false;
      novi->ascii_bajtovi = 0;
      novi->ascii_hex = nullptr;
      novi->jump = false;
      novi->sledeci = nullptr;

      KontekstElem* pom = this->kontekst_ostatak;
      if(this->kontekst_ostatak == nullptr){
        this->kontekst_ostatak = novi;
      }else{
        while(pom->sledeci){
          pom = pom->sledeci;
        }

        pom->sledeci = novi;
      }
      this->dodajRelokacioniZapis(this->locationCounter, simboli[i].simbol);
  
    }else{    //literal
      KontekstElem* novi = new KontekstElem();
      novi->word = true;
      novi->word_bajtovi = simboli[i].literal;
      novi->skip = false;
      novi->skip_bajtovi = 0;
      novi->disp = 0;
      novi->oc_instrukcije = 0;
      novi->mod = 0;
      novi->regA = 0;
      novi->regB = 0;
      novi->regC = 0;
      novi->sekcija = this->trenutnaSekcija;
      novi->ascii = false;
      novi->ascii_hex = nullptr;
      novi->jump = false;
      novi->sledeci = nullptr;

      KontekstElem* pom = this->kontekst_ostatak;
        if(this->kontekst_ostatak == nullptr){
          this->kontekst_ostatak = novi;
        }else{
          while(pom->sledeci){
            pom = pom->sledeci;
          }

          pom->sledeci = novi;
        }
    }
    this->locationCounter+=4;
  }
}

void Kontekst::dodajSkipUKontekst(int broj){   //popunjava nulama
  KontekstElem* pom = this->kontekst_ostatak;
  KontekstElem* novi = new KontekstElem();
  novi->skip = true;
  novi->jump = false;
  novi->word = false;
  novi->skip_bajtovi = broj;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->ascii = false;
  novi->ascii_hex = nullptr;
  novi->jump = false;
  novi->sledeci = nullptr;


  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }

    pom->sledeci = novi;
  }
  
}

void Kontekst::dodajAscii(char* niz){
  int br = 0;
  KontekstElem* novi = new KontekstElem();
  int brKaraktera = 2 * strlen(niz) + 2;
  if(brKaraktera % 8){
    brKaraktera = brKaraktera + (8 - (brKaraktera % 8));
  }
  char* bajtovi = new char[brKaraktera];
  int indeks = 0;
  while(niz[br]!='\0'){    //ovde se pretvara u hex
    sprintf(&bajtovi[indeks], "%02X", static_cast<unsigned char>(niz[br]));
    indeks += 2;
    br++;
  }
  sprintf(&bajtovi[indeks], "%02x", static_cast<unsigned char>('\0'));
  indeks += 2;
  for(int i = indeks; i < brKaraktera; ){
    sprintf(&bajtovi[i], "%02x", static_cast<unsigned char>('\0'));
    i += 2;
  }
  //preprvka u little endian format
  for(int i = 0; i < brKaraktera; i += 8) { 
    std::swap(bajtovi[i], bajtovi[i + 6]);
    std::swap(bajtovi[i + 1], bajtovi[i + 7]);
    std::swap(bajtovi[i + 2], bajtovi[i + 4]);
    std::swap(bajtovi[i + 3], bajtovi[i + 5]);
  }
  novi->skip = false;
  novi->ascii = true;
  novi->ascii_bajtovi  = brKaraktera / 2;
  novi->ascii_hex = bajtovi;
  novi->jump = false;
  novi->word = false;
  novi->oc_instrukcije = 0;
  novi->mod = 0;
  novi->regA = 0;
  novi->regB = 0;
  novi->regC = 0;
  novi->disp = 0;
  novi->skip_bajtovi = 0;
  novi->sledeci = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  this->locationCounter += brKaraktera / 2;

  KontekstElem* pom = this->kontekst_ostatak;
  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }

    pom->sledeci = novi;
  }
}

void Kontekst::dodajAddSubMulDiv(int id, int reg1, int reg2){
  KontekstElem* novi = new KontekstElem();
  novi->regA = reg2;
  novi->regB = reg2;
  novi->regC = reg1;
  novi->oc_instrukcije = OC_AR;
  novi->skip = false;
  novi->jump = false;
  novi->word = false;
  novi->ascii = false;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->sledeci = nullptr;
  novi->ascii_hex = nullptr;
  novi->ascii_bajtovi = 0;

  switch (id)
  {
  case 0:   //add
    novi->mod = AR_ADD;
    break;
  
  case 1:   //sub
    novi->mod = AR_SUB;
    break;
  case 2:   //mul
    novi->mod = AR_MUL;
    break;
  case 3:   //div
    novi->mod = AR_DIV;
    break;
  default:

    break;
  }
  KontekstElem* pom = this->kontekst_ostatak;
  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajShlShr(int id, int reg1, int reg2){
  KontekstElem* novi = new KontekstElem();
  novi->regA = reg2;
  novi->regB = reg2;
  novi->regC = reg1;
  novi->oc_instrukcije = OC_SH;
  novi->skip = false;
  novi->jump = false;
  novi->word = false;
  novi->ascii = false;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->sledeci = nullptr;
  novi->ascii_hex = nullptr;
  novi->ascii_bajtovi = 0;

  switch (id)
  {
  case 0:   //shl
    novi->mod = SH_L;
    break;
  
  case 1:   //shr
    novi->mod = SH_R;
    break;
  
  }
  KontekstElem* pom = this->kontekst_ostatak;
  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajNotAndOrXor(int id, int reg1, int reg2){
  KontekstElem* novi = new KontekstElem();
  novi->regA = reg2;
  novi->regB = reg2;
  novi->oc_instrukcije = OC_LOG;
  novi->skip = false;
  novi->jump = false;
  novi->word = false;
  novi->ascii = false;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->sledeci = nullptr;
  novi->ascii_hex = nullptr;
  novi->ascii_bajtovi = 0;

  switch (id)
  {
  case 0:   //not
    novi->mod = LOG_NOT;
    break;
  
  case 1:   //and
    novi->regC = reg1;
    novi->mod = LOG_AND;
    break;
  case 2:   //or
    novi->regC = reg1;
    novi->mod = LOG_OR;
    break;
  case 3:   //xor
    novi->regC = reg1;
    novi->mod = LOG_XOR;
    break;
  default:

    break;
  }
  KontekstElem* pom = this->kontekst_ostatak;
  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajXchg(int reg1, int reg2){
  KontekstElem* novi = new KontekstElem();
  novi->regA = 0; //ovaj bajt kod ove instrukcije su nule
  novi->regB = reg2;
  novi->regC = reg1;
  novi->oc_instrukcije = OC_XCHG;
  novi->skip = false;
  novi->jump = false;
  novi->word = false;
  novi->ascii = false;
  novi->mod = 0;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->sledeci = nullptr;
  novi->ascii_hex = nullptr;
  novi->ascii_bajtovi = 0;

  KontekstElem* pom = this->kontekst_ostatak;
  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajHalt(){
  KontekstElem* novi = new KontekstElem();
  novi->regA = 0; //ovaj bajt kod ove instrukcije su nule
  novi->regB = 0;
  novi->regC = 0;
  novi->oc_instrukcije = OC_HALT;
  novi->skip = false;
  novi->jump = false;
  novi->word = false;
  novi->ascii = false;
  novi->mod = 0;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->sledeci = nullptr;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

UlazTabeleSimbola* Kontekst::ulazTabeleSimbola(char* simbol){
  UlazTabeleSimbola* pom = this->tabela_simbola;
  while(pom){
    if(!strcmp(simbol, pom->naziv)){
        return pom;
    }
    pom = pom->sledeci;
  }
  return nullptr;
}

char* Kontekst::nazivSekcije(int rbr){
  Sekcija* pom = this->sekcije;
  while(pom){
    if(pom->rbr == rbr){
      return pom->naziv;
    }
    pom = pom->sledeca;
  }
  return nullptr;
}

void Kontekst::dodajRelokacioniZapis(uint32_t lc, char* simbol){
  //ova metoda treba prvo da proveri da li je simbol koji je prosledjen u tabeli simbola definisan
  //ako je definisan znaci da je simbol u drugoj sekciji i onda se proverava da li je lokalan ili globalan
  //ako je lokalan onda se u relokacioni zapis stavljaju broj sekije i pomeraj simbola od pocetka sekcije u simbol i addend
  //u offset relokacionig zapisa se stavlja procledjeni lc (pomeraj od pocetka trenutne sekcije)
  //ako simbol nije definisan onda je uvezen pa se stavlja identifikator simbola kao simbol addend na 0 i kao offset lc
  //ostali slucajevi svi bi trebalo da mogu backpatching-om da se razrese
  this->brojRelokacionihZapisa ++;
  RelokacioniZapisElem* novi = new RelokacioniZapisElem();
  novi->sledeci = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  if(this->simbolUTabeli(simbol)){  //simbol postoji i definisan je
    UlazTabeleSimbola* ulaz = this->ulazTabeleSimbola(simbol);
    novi->offset = lc;
    if(ulaz->global){  //ako je globalan
      novi->simbol = ulaz->rbr;
      novi->addend = 0;
      novi->backpatching = false;
      
    }else{  //ako je lokalan
      char* nazivSekcije = this->nazivSekcije(ulaz->sekcija);
      UlazTabeleSimbola* sekc = this->ulazTabeleSimbola(nazivSekcije);
      novi->simbol = sekc->rbr;  //identifikator simbola iz tabele simbola
      novi->addend = ulaz->vrednost;  //ovo je mozda pogresno
      novi->backpatching = false;
    }
  }else{
    UlazTabeleSimbola* ulaz = this->ulazTabeleSimbola(simbol);
    if(ulaz!=nullptr){
      if(ulaz->uvezen){ //uvezen simbol
        novi->addend = 0;
        novi->simbol = ulaz->rbr;
        novi->offset = lc;
        novi->backpatching = false;
      }else{  //ako simbol nije jos definisan i nije uvezen onda treba da se ovaj zapis prepravi u bekpecingu jer je u pitanju nedefinisan simbol
        if(ulaz->global){
          novi->addend = 0;
          novi->simbol = ulaz->rbr;
          novi->offset = lc;
          novi->backpatching = true;
        }else{
          char* nazivSekcije = this->nazivSekcije(ulaz->sekcija);
          UlazTabeleSimbola* sekc = this->ulazTabeleSimbola(nazivSekcije);
          novi->simbol = sekc->rbr;  //identifikator simbola iz tabele simbola
          novi->addend = ulaz->vrednost;
          novi->backpatching = true;
        }
      }
    }else{
      this->dodajSimbol(simbol, 0, false, false, false, false);
      this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, simbol);
      return;
    }
  }

  RelokacioniZapisElem* pom = this->relokacioni_zapisi;
  if(pom==nullptr){
    this->relokacioni_zapisi = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajUTabeluObracanjaUnapred(uint32_t lc, bool literal, bool simbol, uint32_t lit, char* sim){
  BackpatchingElem* novi = new BackpatchingElem();
  if(literal){
    novi->literal  =literal;
    novi->simbol = simbol;
    novi->lit = lit;
    novi->sledeci = nullptr;
    novi->lokacija = lc;
    novi->sekcija = this->trenutnaSekcija;
  }else if(simbol){
    novi->literal  = literal;
    novi->simbol = simbol;
    novi->sim = sim;
    novi->sledeci = nullptr;
    novi->lokacija = lc;
    novi->sekcija = this->trenutnaSekcija;
  }
  if(this->tabela_obracanja_unapred == nullptr){
    this->tabela_obracanja_unapred  = novi;
  }else{
    BackpatchingElem* pom = this->tabela_obracanja_unapred;
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajUTabeluLiterala(uint32_t literal){
  TabelaLiterala* novi = new TabelaLiterala();
  novi->vrednost = literal;
  novi->sledeci = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  if(this->tabela_literala == nullptr){
    this->tabela_literala = novi;
  }else{
    TabelaLiterala* pom = this->tabela_literala;
    bool vecTu = false;
    if(pom->vrednost == literal && pom->sekcija == this->trenutnaSekcija){
        vecTu = true;
    }
    while(pom->sledeci){
      if(pom->vrednost == literal && pom->sekcija == this->trenutnaSekcija){
        vecTu = true;
      }
      pom = pom->sledeci;
    }
    if(pom->vrednost == literal && pom->sekcija == this->trenutnaSekcija){
        vecTu = true;
    }
    if(vecTu == false){  //dodajemo literal u tabelu samo ako vec nije tu
      pom->sledeci = novi;
    }
  }
}


void Kontekst::dodajSkok(int id, Instrukcija instrukcija){
  //skok moze da skace na literal ili na simbol tj na adresu koja je vrednost literala ili na vrednost simbola
  //mora da se koristi bazen literala za literale i tada se pravi backpatch zapis
  //ako se skace u drugu sekciju (ovde je simbol ne literal ) to razresava linker u procesu povezivanja pa onda pravimo relokacioni zapis
  //ako simbol na koji se skace nije definisan onda se pravi backpatch zapis
  //ako je simbol definisan moze da se odmah razresi pomeraj
  KontekstElem* novi = new KontekstElem();
  novi->oc_instrukcije = OC_JMP;
  novi->skip = false;
  novi->jump = true;
  novi->word = false;
  novi->ascii = false;
  novi->sekcija = this->trenutnaSekcija;
  novi->sledeci = nullptr;
  novi->ascii_hex = nullptr;
  novi->ascii_bajtovi = 0;
  novi->word_bajtovi = 0;
  novi->regA = 15; //ovo mora pc da bude

  if(instrukcija.operand.simbol.vrsta == 1){  //proveravamo da li je literal
    switch (id)
    {
    case 0:   //jmp
      novi->mod = JMP_1;    //pc<=gprA + DDD   --> gprA = pc
      break;
    case 1:   //beq
      novi->mod = JMP_2;   //gprB i gprC su registri koji se porede
      novi->regB = instrukcija.regA;
      novi->regC = instrukcija.regB;
      break;
    case 2:   //bne
      novi->mod = JMP_3;
      novi->regB = instrukcija.regA;
      novi->regC = instrukcija.regB;
      break;
    case 3:   //bgt
      novi->mod = JMP_4;
      novi->regB = instrukcija.regA;
      novi->regC = instrukcija.regB;
      break;
    default:
      break;
    }
    if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
      novi->regA = 0;
      novi->disp = instrukcija.operand.simbol.literal;
    }else{
      novi->disp = 0;
      switch (id)
      {
      case 0:   //jmp
        novi->mod = JMP_5;    //pc<=gprA + DDD   --> gprA = pc
        break;
      case 1:   //beq
        novi->mod = JMP_6;   //gprB i gprC su registri koji se porede
        novi->regB = instrukcija.regA;
        novi->regC = instrukcija.regB;
        break;
      case 2:   //bne
        novi->mod = JMP_7;
        novi->regB = instrukcija.regA;
        novi->regC = instrukcija.regB;
        break;
      case 3:   //bgt
        novi->mod = JMP_8;
        novi->regB = instrukcija.regA;
        novi->regC = instrukcija.regB;
        break;
      default:
        break;
      }
      this->dodajUTabeluLiterala(instrukcija.operand.simbol.literal);
      this->dodajUTabeluObracanjaUnapred(this->locationCounter, true, false, instrukcija.operand.simbol.literal, nullptr);
    }
  }else{ //ako je simbol u pitanju
    switch (id)
    {
    case 0:   //jmp
      novi->mod = JMP_1;    //pc<=gprA + DDD   --> gprA = pc
      break;
    case 1:   //beq
      novi->mod = JMP_2;   //gprB i gprC su registri koji se porede
      novi->regB = instrukcija.regA;
      novi->regC = instrukcija.regB;
      break;
    case 2:   //bne
      novi->mod = JMP_3;
      novi->regB = instrukcija.regA;
      novi->regC = instrukcija.regB;
      break;
    case 3:   //bgt
      novi->mod = JMP_4;
      novi->regB = instrukcija.regA;
      novi->regC = instrukcija.regB;
      break;
    default:
      break;
    }

    if(this->simbolUTabeli(instrukcija.operand.simbol.simbol)){  //ako se simbol već nalazi u tabeli i definisan je
      uint32_t vr = this->vrednostSimbola(instrukcija.operand.simbol.simbol);
      int sect = this->sekcijaSimbola(instrukcija.operand.simbol.simbol);  //treba da se proveri da li je skok unutar iste sekcije - ako nije relok. zapis ako ne backpatch zapis
      if(sect == this->trenutnaSekcija){ //skok unutar iste sekcije
        //u ovom slucaju disp je pomeraj do tog simbola
        uint16_t disp = vr - (this->locationCounter + 4);
        novi->disp = disp & 0x0fff;
      }else{ //skok u drugu sekciju ->linker treba da razreši->formiraj relokacioni zapis
        this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
      }
    }else{  //backpatch zapis
      UlazTabeleSimbola* ulaz = this->ulazTabeleSimbola(instrukcija.operand.simbol.simbol);
      if(ulaz == nullptr){
        this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
      }else{
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, instrukcija.operand.simbol.simbol);  
      } 
    }
  }

  
  
  KontekstElem* pom = this->kontekst_ostatak;
  if(this->kontekst_ostatak == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajInt(){
  KontekstElem* novi = new KontekstElem();
  novi->regA = 0; //ovaj bajt kod ove instrukcije su nule
  novi->regB = 0;
  novi->regC = 0;
  novi->oc_instrukcije = OC_INT;
  novi->skip = false;
  novi->jump = false;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->ascii = false;
  novi->mod = 0;
  novi->sekcija = this->trenutnaSekcija;
  novi->disp = 0;
  novi->sledeci = nullptr;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajCall(Instrukcija& instrukcija){
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_CALL;
  novi->regA = 15; //pc
  novi->regB = 0;
  novi->regC = 0;
  novi->jump = false;

  if(instrukcija.operand.simbol.vrsta == 1){   //literal
    //ovde treba bazen literala da se koristi
    novi->mod = CALL_1;
    if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
      novi->regA = 0;
      novi->disp = instrukcija.operand.simbol.literal;
    }else{
      novi->disp  = 0;
      novi->mod = CALL_2;  //citamo onda iz memorije sa pc + pomeraj do bazena
      this->dodajUTabeluLiterala(instrukcija.operand.simbol.literal);
      //sada nam treba i backpatching zapis
      this->dodajUTabeluObracanjaUnapred(this->locationCounter, true, false, instrukcija.operand.simbol.literal, nullptr);
    }
  }else{  //simbol
    novi->mod = CALL_2;
    //za simbol opet imamo proveru da li je definisan i uistoj sekciji kao i kod skokova
    if(this->simbolUTabeli(instrukcija.operand.simbol.simbol)){  //ako se simbol već nalazi u tabeli i definisan je
      uint32_t vr = this->vrednostSimbola(instrukcija.operand.simbol.simbol);
      int sect = this->sekcijaSimbola(instrukcija.operand.simbol.simbol);  //treba da se proveri da li je skok unutar iste sekcije - ako nije relok. zapis ako ne backpatch zapis
      if(sect == this->trenutnaSekcija){ //skok unutar iste sekcije
        //u ovom slucaju disp je pomeraj do tog simbola
        uint16_t disp = vr - (this->locationCounter + 4);
        novi->disp = disp & 0x0fff;
      }else{ //skok u drugu sekciju ->linker treba da razreši->formiraj relokacioni zapis
        this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
      }
    }else{  //backpatch zapis
      if(this->sekcijaSimbola(instrukcija.operand.simbol.simbol) == this->trenutnaSekcija){
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, instrukcija.operand.simbol.simbol);
      }else{  //relokacioni zapis
        this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
      }
    }
  }
  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajCsrwr(Instrukcija& instrukcija){
  //ld -> varijanta 5
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_LD;
  novi->mod = LD_5;
  novi->regB = instrukcija.regA;
  novi->regC = 0;
  novi->disp = 0;
  novi->jump = false;

  if(!strcmp("status", instrukcija.simbol)){
    novi->regA = 0;
  }else if(!strcmp("handler", instrukcija.simbol)){
    novi->regA = 1;
  }else if(!strcmp("cause", instrukcija.simbol)){
    novi->regA = 2;
  }

  
  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajCsrrd(Instrukcija& instrukcija){
  //ld -> varijanta 1
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_LD;
  novi->mod = LD_1;
  novi->jump = false;

  novi->regA = instrukcija.regA;
  novi->regC = 0;
  novi->disp = 0;

  if(!strcmp("status", instrukcija.simbol)){
    novi->regB = 0;
  }else if(!strcmp("handler", instrukcija.simbol)){
    novi->regB = 1;
  }else if(!strcmp("cause", instrukcija.simbol)){
    novi->regB = 2;
  }

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajLd(Instrukcija& instrukcija){
  //ovde imam 2 moguca moda [2, 3] bez 1 i 5(csrwr i csrrd) ( 4 -> pop)
  //ld ucitava neki operand u registar koji je dat u instrukcijij
  //ako je operand simbol treba koristiti tabelu simbola i ako je definisan resiti odmah sve ako nije onda treba napraviti zapis za backpatch
  //ako je literal onda bazen literala + backpatcing zapis
  //ako je regitar -> najjednostavnije
  //neposredna vrednost -> opet imamo literal simbol situaciju

  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_LD;
  novi->regA = instrukcija.regA;
  novi->jump = false;
  novi->sledeci = nullptr;

  bool drugi  = false;

  if(instrukcija.operand.adresiranje == 0){  //immed -> ucitava se direktno ta vrednost
    novi->mod = LD_2;   //gprA = gprB + pc
    if(instrukcija.operand.simbol.vrsta == 0){  //simbol
      novi->regB = 15; //pc
      novi->regC = 0;
      if(this->simbolUTabeli(instrukcija.operand.simbol.simbol)){  //ako se simbol već nalazi u tabeli i definisan je
        uint32_t vr = this->vrednostSimbola(instrukcija.operand.simbol.simbol);
        int sect = this->sekcijaSimbola(instrukcija.operand.simbol.simbol);  //treba da se proveri da li je skok unutar iste sekcije - ako nije relok. zapis ako ne backpatch zapis
        if(sect == this->trenutnaSekcija){ //skok unutar iste sekcije
          //u ovom slucaju disp je pomeraj do tog simbola
          uint16_t disp = vr - (this->locationCounter + 4);
          novi->disp = disp & 0x0fff;
        }else{ //simbol iz druge sekcije ->linker treba da razreši -> formiraj relokacioni zapis
          novi->mod = LD_3;
          this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
          novi->disp = 0;
        }
      }else{  //backpatch zapis
        UlazTabeleSimbola* ulaz = this->ulazTabeleSimbola(instrukcija.operand.simbol.simbol);
        if(ulaz == nullptr){
          this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
        }else{
          novi->mod = LD_3;
          if(this->sekcijaSimbola(instrukcija.operand.simbol.simbol) == this->trenutnaSekcija){
            this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, instrukcija.operand.simbol.simbol);
          }else{  //relokacioni zapis
            this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
          }
        }
        
      }
    }else{  //literal
      if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
        novi->disp = instrukcija.operand.simbol.literal & 0xfff;
        novi->regB = 0;
        novi->regC = 0;
      }else{
        novi->regB = 15; //pc
        novi->regC = 0;
        novi->disp = 0;
        novi->mod = LD_3;
        //za literal treba da se doda prvo u bazen literala pa backpatc zapis da se napravi
        this->dodajUTabeluLiterala(instrukcija.operand.simbol.literal);
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, true, false, instrukcija.operand.simbol.literal, nullptr);
      }
      
    }
  }else if(instrukcija.operand.adresiranje == 1){  //memorijsko direktno -> isto kao za immed samo je drugaciji mod jer ce da se cita iz memorije na toj adresi a ne da se upisuje vrednost
    novi->mod = LD_3;   //gprA = mem32[ gprB + gprC + pc ]
    if(instrukcija.operand.simbol.vrsta == 0){ //simbol
      novi->regB = 15; //pc
      novi->regC = 0;
      if(this->simbolUTabeli(instrukcija.operand.simbol.simbol)){  //ako se simbol već nalazi u tabeli i definisan je
        uint32_t vr = this->vrednostSimbola(instrukcija.operand.simbol.simbol);
        int sect = this->sekcijaSimbola(instrukcija.operand.simbol.simbol);  //treba da se proveri da li je skok unutar iste sekcije - ako nije relok. zapis ako ne backpatch zapis
        if(sect == this->trenutnaSekcija){ //skok unutar iste sekcije
          //u ovom slucaju disp je pomeraj do tog simbola
          uint16_t disp = vr - (this->locationCounter + 4);
          novi->disp = disp & 0x0fff;
        }else{ //simbol iz druge sekcije ->linker treba da razreši -> formiraj relokacioni zapis
          //treba dodati novi ld jer ne postoji mem32[mem32[...]] mod za ld instrukciju
          drugi = true;
          this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
          novi->disp = 0;
        }
      }else{  //backpatch zapis
        UlazTabeleSimbola* ulaz = this->ulazTabeleSimbola(instrukcija.operand.simbol.simbol);
        if(ulaz == nullptr){
          this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
        }else{
          //treba dodati novi ld jer ne postoji mem32[mem32[...]] mod za ld instrukciju ako simbol nije lokalan jer je to onda neki od spolja
          drugi = true;
          this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, instrukcija.operand.simbol.simbol);
        }
      }
    }else{ // literal
      if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
        novi->disp = instrukcija.operand.simbol.literal & 0xfff;
        novi->regB = 0;
        novi->regC = 0;
      }else{
        novi->regB = 15; //pc
        novi->regC = 0;
        novi->disp = 0;
        //za literal treba da se doda prvo u bazen literala pa backpatc zapis da se napravi
        this->dodajUTabeluLiterala(instrukcija.operand.simbol.literal);
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, true, false, instrukcija.operand.simbol.literal, nullptr);
      }
    }
  }else if(instrukcija.operand.adresiranje == 2){  //regdir -> mod 2
    novi->mod = LD_2;
    novi->disp = 0;
    novi->regC = 0;
    novi->regB = instrukcija.operand.reg;
  }else if(instrukcija.operand.adresiranje == 3){ //regind -> mod 3
    novi->mod = LD_3;
    novi->disp = 0;
    novi->regC = 0;
    novi->regB = instrukcija.operand.reg;
  }else if(instrukcija.operand.adresiranje == 4){ //regind pom -> mod 3
    novi->mod = LD_3;
    novi->regB = instrukcija.operand.reg;
    if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
      novi->disp = instrukcija.operand.simbol.literal & 0xfff;
      novi->regC = 0;
    }else{
      cerr<<endl<<"Litral preveliki za regindpom adresiranje"<<endl;
      exit(1);
    }
  }

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }

  if(drugi){
    KontekstElem* novi1 = new KontekstElem();
    novi1->sledeci = nullptr;
    novi1->ascii = false;
    novi1->ascii_bajtovi = 0;
    novi1->ascii_hex = nullptr;
    novi1->sekcija = this->trenutnaSekcija;
    novi1->skip = false;
    novi1->skip_bajtovi = 0;
    novi1->word = false;
    novi1->word_bajtovi = 0;
    novi1->oc_instrukcije = OC_LD;
    novi1->regA = instrukcija.regA;
    novi1->jump = false;
    novi1->sledeci = nullptr;
    novi1->mod = LD_3;
    novi1->regB = instrukcija.regA; //pc
    novi1->regC = 0;
    novi1->disp = 0;
    KontekstElem* pom1 = this->kontekst_ostatak;
    if(pom1 == nullptr){
      this->kontekst_ostatak = novi1;
    }else{
      while(pom1->sledeci){
        pom1 = pom1->sledeci;
      }
      pom1->sledeci = novi1;
    }
    this->locationCounter += 4;
  }
}

void Kontekst::dodajPop(Instrukcija& instrukcija){
  //ovo je zapravo ld sa modom 4 -> gprA <= mem32[gprB]; gprB = gprB + D
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_LD;
  novi->regA = instrukcija.regA;
  novi->sledeci = nullptr;
  novi->jump = false;

  novi->mod = LD_4;
  novi->regB = 14; //sp
  novi->regC = 0;
  novi->disp = 0b000000000100;  //4

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajPush(Instrukcija& instrukcija){
  //ovo ce da bude st mod 3  -> gprA = gprA + D; mem32[ gprA ] = gprC;
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_ST;
  novi->regC = instrukcija.regA;
  novi->jump = false;
  novi->sledeci = nullptr;

  novi->regA = 14; //sp
  novi->regB = 0;
  novi->disp = 0xffc;  //-4
  novi->mod = ST_3;

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajSt(Instrukcija& instrukcija){
  //modovi 1 i 2 za ST
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_ST;
  novi->regC = instrukcija.regA;  //ovo je registar koji se smesta u memoriju
  novi->jump = false;
  novi->sledeci = nullptr;

  if(instrukcija.operand.adresiranje == 0){  //immed ->mod 1  mem32[ gprA + gprB + D ] <= gprC
    novi->mod = ST_1;   //gprA = gprB + pc
    if(instrukcija.operand.simbol.vrsta == 0){  //simbol
      novi->regB = 0; 
      novi->regA = 15;  //pc
      if(this->simbolUTabeli(instrukcija.operand.simbol.simbol)){  //ako se simbol već nalazi u tabeli i definisan je
        uint32_t vr = this->vrednostSimbola(instrukcija.operand.simbol.simbol);
        novi->regB = 0; 
        novi->regA = 15;  //pc
        int sect = this->sekcijaSimbola(instrukcija.operand.simbol.simbol);  //treba da se proveri da li je skok unutar iste sekcije - ako nije relok. zapis ako ne backpatch zapis
        if(sect == this->trenutnaSekcija){ //skok unutar iste sekcije
          //u ovom slucaju disp je pomeraj do tog simbola
          uint16_t disp = vr - (this->locationCounter + 4);
          novi->disp = disp & 0x0fff;
        }else{ //simbol iz druge sekcije ->linker treba da razreši -> formiraj relokacioni zapis
          this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
          novi->disp = 0;
          novi->mod = ST_2;
        }
      }else{  //backpatch zapis
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, instrukcija.operand.simbol.simbol);
      }
    }else{  //literal
      if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
        novi->regA = 0;
        novi->regB = 0;
        novi->disp = instrukcija.operand.simbol.literal & 0xfff;
      }else{
        novi->regB = 0; 
        novi->regA = 15;  //pc
        novi->disp = 0;
        novi->mod = ST_2;
        //za literal treba da se doda prvo u bazen literala pa backpatc zapis da se napravi
        this->dodajUTabeluLiterala(instrukcija.operand.simbol.literal);
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, true, false, instrukcija.operand.simbol.literal, nullptr);
      }
    }
  }else if(instrukcija.operand.adresiranje == 1){  //memdir -> mod 2  mem32[ mem32[ gprA + gprB + D ] ] <= gprC
    novi->mod = ST_2;
    novi->regB = 0; 
    novi->regA = 15;  //pc   
    if(instrukcija.operand.simbol.vrsta == 0){  //simbol
      if(this->simbolUTabeli(instrukcija.operand.simbol.simbol)){  //ako se simbol već nalazi u tabeli i definisan je
        uint32_t vr = this->vrednostSimbola(instrukcija.operand.simbol.simbol);
        novi->regB = 0; 
        novi->regA = 15;  //pc
        int sect = this->sekcijaSimbola(instrukcija.operand.simbol.simbol);  //treba da se proveri da li je skok unutar iste sekcije - ako nije relok. zapis ako ne backpatch zapis
        if(sect == this->trenutnaSekcija){ //skok unutar iste sekcije
          //u ovom slucaju disp je pomeraj do tog simbola
          uint16_t disp = vr - (this->locationCounter + 4);
          novi->disp = disp & 0x0fff;
        }else{ //simbol iz druge sekcije ->linker treba da razreši -> formiraj relokacioni zapis
          this->dodajRelokacioniZapis(this->locationCounter, instrukcija.operand.simbol.simbol);
          novi->disp = 0;
        }
      }else{  //backpatch zapis
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, false, true, 0, instrukcija.operand.simbol.simbol);
      }
    }else{  //literal
      if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
        novi->mod = ST_1;
        novi->regA = 0;
        novi->regB = 0;
        novi->disp = instrukcija.operand.simbol.literal & 0xfff;
      }else{
        novi->regB = 0; 
        novi->regA = 15;  //pc
        novi->disp = 0;
        //za literal treba da se doda prvo u bazen literala pa backpatc zapis da se napravi
        this->dodajUTabeluLiterala(instrukcija.operand.simbol.literal);
        this->dodajUTabeluObracanjaUnapred(this->locationCounter, true, false, instrukcija.operand.simbol.literal, nullptr);
      }
    }
  }else if(instrukcija.operand.adresiranje == 2){  //regdir -> mod 1
    novi->mod = ST_1;
    novi->disp = 0;
    novi->regB = 0;
    novi->regA = instrukcija.operand.reg;
  }else if(instrukcija.operand.adresiranje == 3){  //regind -> mod 2
    novi->mod = ST_1;
    novi->disp = 0;
    novi->regB = 0;
    novi->regA = instrukcija.operand.reg;
  }else if(instrukcija.operand.adresiranje == 4){  //regindpom -> mod 2
    novi->mod = ST_1;
    if(instrukcija.operand.simbol.literal >= -2048 && instrukcija.operand.simbol.literal <= 2047){
      novi->regB = 0;
      novi->regA = instrukcija.operand.reg;
      novi->disp = instrukcija.operand.simbol.literal & 0xfff;
    }else{
      cerr<<endl<<"Preveliki literal za regindpom -> treba da bude maksimalno 12 bita"<<endl;
      exit(1);
    }
  }

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajRet(){
  //ret je samo pop za PC -> pop pc tj bice ld u pc sa steka
  //ovo je zapravo ld sa modom 4 -> gprA <= mem32[gprB]; gprB = gprB + D
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_LD;
  novi->regA = 15;   //pc
  novi->jump = false;
  novi->sledeci = nullptr;

  novi->mod = LD_4;
  novi->regB = 14; //sp
  novi->regC = 0;
  novi->disp = 4;  //4

  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
}

void Kontekst::dodajIret(){
  //gprA = gprB + D (sp = sp + 8)   -> ld sp, sp + 8
  //csrA = mem32[gprB + gprC + D] (status = mem32[sp - 4])
  //gprA = mem32[gprB + gprC + D] (pc = mem32[sp - 8])
  KontekstElem* novi = new KontekstElem();
  novi->sledeci = nullptr;
  novi->ascii = false;
  novi->ascii_bajtovi = 0;
  novi->ascii_hex = nullptr;
  novi->sekcija = this->trenutnaSekcija;
  novi->skip = false;
  novi->skip_bajtovi = 0;
  novi->word = false;
  novi->word_bajtovi = 0;
  novi->oc_instrukcije = OC_LD;
  novi->regA = 14;   //pc
  novi->mod = LD_2;
  novi->regB = 14;
  novi->disp = 0b000000001000;  //8
  novi->jump = false;
  novi->sledeci = nullptr;
  KontekstElem* pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi;
  }
  KontekstElem* novi1 = new KontekstElem();
  novi1->sledeci = nullptr;
  novi1->ascii = false;
  novi1->ascii_bajtovi = 0;
  novi1->ascii_hex = nullptr;
  novi1->sekcija = this->trenutnaSekcija;
  novi1->skip = false;
  novi1->skip_bajtovi = 0;
  novi1->word = false;
  novi1->word_bajtovi = 0;
  novi1->oc_instrukcije = OC_LD;   
  novi1->mod = LD_7;  //csrA<=mem32[gptB + gprC + D]
  novi1->regA = 0; //sstatus
  novi1->regB = 14; // sp
  novi1->regC = 0;
  novi1->disp = 0b111111111100; // -4
  novi1->jump = false;
  novi1->sledeci = nullptr;
  pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi1;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi1;
  }
  KontekstElem* novi2 = new KontekstElem();
  novi2->sledeci = nullptr;
  novi2->ascii = false;
  novi2->ascii_bajtovi = 0;
  novi2->ascii_hex = nullptr;
  novi2->sekcija = this->trenutnaSekcija;
  novi2->skip = false;
  novi2->skip_bajtovi = 0;
  novi2->word = false;
  novi2->word_bajtovi = 0;
  novi2->oc_instrukcije = OC_LD;
  novi2->mod = LD_3;   //isto kao malopre samo smesta u gpr
  novi2->regA = 15; // pc
  novi2->regB = 14; // sp
  novi2->regC = 0;
  novi2->disp = 0b111111111000; // -8
  novi2->jump = false;

  pom = this->kontekst_ostatak;
  if(pom == nullptr){
    this->kontekst_ostatak = novi2;
  }else{
    while(pom->sledeci){
      pom = pom->sledeci;
    }
    pom->sledeci = novi2;
  }
}

void Kontekst::proveraSimbola(){
  //ova metoda proverava da li u tabeli simbola ima lokalnih nedefinisanih simbola -> ako ima baca gresku
  //takodje proverava i da li ima globalnih nedefinisanih i ako ima menja ih sa global na extern
  UlazTabeleSimbola* pom = this->tabela_simbola;
  while(pom){
    if(pom->global && pom->definisan == false && pom->uvezen == false && pom->rbr != 0){
      pom->global = false;
      pom->uvezen = true;
    }else if(pom->global == false && pom->definisan == false && pom->uvezen == false && pom->rbr != 0){
      cerr << endl << "Greška: Nedefinisan simbol: " << pom->naziv << endl;
      exit(1);
    }
    pom = pom->sledeci;
  }
}

void Kontekst::napraviBazeneLiterala(){
  for (int i = 0; i <= this->brojSekcija; i++){
    BazeniLiterala* bazen = new BazeniLiterala();
    bazen->sledeci = nullptr;
    bazen->sekcija = i;
    TabelaLiterala* pom = this->tabela_literala;
    while(pom){  //formiramo bazen literala
      //cout<<endl<<"Sekcija literala: "<<" " << pom->sekcija<< " " << "vrednost i "<<i<< " "<< hex<< std::setw(8)<< std::setfill('0')<< pom->vrednost ;
      if(pom->sekcija == i){
        bazen->bazen.push_back(pom->vrednost);
      }
      pom = pom->sledeci;
    }
    if(this->bazen_literala == nullptr){
      this->bazen_literala = bazen;
    }else{
      BazeniLiterala* baz = this->bazen_literala;
      while(baz->sledeci){
        baz = baz->sledeci;
      }
      baz->sledeci = bazen;
    }
  }
  Sekcija* sek = this->sekcije;
  while(sek){
    BazeniLiterala* pom = this->bazen_literala;
    while(pom){
      if(pom->sekcija == sek->rbr){
        sek->velicina += pom->bazen.size()  * 4;
      }
      pom = pom->sledeci;
    }
    sek = sek->sledeca;
  }
}

uint16_t Kontekst::velicinaSekcije(int id){
  Sekcija* pom = this->sekcije;
  uint32_t ret = 0;
  while(pom){
    if(pom->rbr == id){
      return pom->velicina;
    }
    pom = pom->sledeca;
  }
  return ret;
}

void Kontekst::postaviVelicinuPoslednjeSekcije(){
  Sekcija* pom = this->sekcije;
  while(pom){
    if(pom->rbr == this->trenutnaSekcija){
      pom->velicina = this->locationCounter;
      break;
    }
    pom = pom->sledeca;
  }
}

void Kontekst::poravnajSekcije(){
  //ovo treba nakon bekpecinga
  Sekcija* sekcija = this->sekcije;
  while(sekcija){
    if(sekcija->velicina % 4){
      //treba dodati skip bajtova
      int brSkipBajtova = 4 - (sekcija->velicina % 4);
      KontekstElem* pom = this->kontekst_ostatak;
      KontekstElem* novi = new KontekstElem();
      novi->skip = true;
      novi->jump = false;
      novi->word = false;
      novi->skip_bajtovi = brSkipBajtova;
      novi->sekcija = sekcija->rbr;
      novi->disp = 0;
      novi->ascii = false;
      novi->ascii_hex = nullptr;
      novi->jump = false;
      novi->sledeci = nullptr;


      if(this->kontekst_ostatak == nullptr){
        this->kontekst_ostatak = novi;
      }else{
        while(pom->sledeci){
          pom = pom->sledeci;
        }

        pom->sledeci = novi;
      }
      sekcija->velicina += brSkipBajtova;
    }

    sekcija = sekcija->sledeca;
  }
}
