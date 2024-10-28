#include "../../inc/asembler/glavnaPetlja.hpp"


void direktiva_handler(Linija& linija, Kontekst* kontekst){

  if(linija.lab){
    kontekst->dodajSimbol(linija.labela, kontekst->locationCounter, false, true, false, false);  //ovde treba da se doda simbol u tabelu simbola
  }

  if(linija.direktiva.vrsta == 0){  //global

    kontekst->dodajGlobal(linija.direktiva.simboli);

  }else if(linija.direktiva.vrsta == 1){  //extern

    kontekst->dodajExtern(linija.direktiva.simboli);

  }else if(linija.direktiva.vrsta == 2){  //section

    kontekst->dodajSekciju(linija.direktiva.simbol);   //ova metoda odradi sve potrebne stvari kad se naidje na novu sekciju - ova direktiva ne utice na location counter

  }else if(linija.direktiva.vrsta == 3){  //skip

    //za skip direktivu je potrebno samo trenutnu sekciju da prosirimo nulama tj ona pise nule onoliko bajtova koliko je dato uz nju
    //plus se location counter povecava za broj bajtova dat uz direktivu
    //plus se dodaje u kontekst jer menja memoriju
    kontekst->dodajSkipUKontekst(linija.direktiva.broj);
    kontekst->locationCounter = kontekst->locationCounter + linija.direktiva.broj;

  }else if(linija.direktiva.vrsta == 4){  //word
    
    //ova direktiva menja location counter za 4B
    kontekst->dodajWord(linija.direktiva.simboli);
    //kontekst->locationCounter = kontekst->locationCounter + 4;
 
  }else if(linija.direktiva.vrsta == 5){  //ascii

    //treba da u trenutnoj sekciji alocira prostor za string -> 1B je jedan karakter ( njegov ascii kod )
    kontekst->dodajAscii(linija.direktiva.ascii_string);

  }else if(linija.direktiva.vrsta == 7){  //end

    //end direktiva treba da zavrsi proces asembliranja i posle se ne gleda nista
    kontekst->kraj = true;
  }

}


void instrukcija_handler(Linija& linija, Kontekst* kontekst){

  if(linija.lab == true){
    kontekst->dodajSimbol(linija.labela, kontekst->locationCounter, false, true, false, false);  //ovde treba da se doda simbol u tabelu simbola
  }

  if(linija.instrukcija.instrukcija == 0){        //halt
    kontekst->dodajHalt();  //lako samo se popuni nulama sve
    kontekst->locationCounter = kontekst->locationCounter + 4;  

  }else if(linija.instrukcija.instrukcija == 1){  //int
    kontekst->dodajInt();    //slicno kao halt samo je oc drugaciji
    kontekst->locationCounter = kontekst->locationCounter + 4;  

  }else if(linija.instrukcija.instrukcija == 2){  //iret
    //iret nema opcode tako da se pravi kao
    //gprA = gprB + D (sp = sp + 8)
    //csrA = mem32[gprB + gprC + D] (status = mem32[sp - 4])
    //gprA = mem32[gprB + gprC + D] (ststus = mem32[sp - 8])
    kontekst->dodajIret();
    kontekst->locationCounter = kontekst->locationCounter + 12;
    
  }else if(linija.instrukcija.instrukcija == 3){  //call

    kontekst->dodajCall(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;  
    
  }else if(linija.instrukcija.instrukcija == 4){  //ret

    //ret je samo pop za PC -> pop pc tj bice ld u pc sa steka
    kontekst->dodajRet();
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 5){  //jmp

    kontekst->dodajSkok(0, linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;  
    
  }else if(linija.instrukcija.instrukcija == 6){  //beq

    kontekst->dodajSkok(1, linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 7){  //bne

    kontekst->dodajSkok(2, linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 8){  //bgt

    kontekst->dodajSkok(3, linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 9){  //push

    kontekst->dodajPush(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 10){ //pop

    kontekst->dodajPop(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 11){ //xchg

    kontekst->dodajXchg(linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 12){ //add

    // add %gpr, %gpr
    kontekst->dodajAddSubMulDiv(0, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 13){ //sub

    kontekst->dodajAddSubMulDiv(1, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 14){ //mul

    kontekst->dodajAddSubMulDiv(2, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 15){ //div

    kontekst->dodajAddSubMulDiv(3, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 16){ //not

    kontekst->dodajNotAndOrXor(0, 0, linija.instrukcija.regA);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 17){ //and

    kontekst->dodajNotAndOrXor(1, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 18){ //or

    kontekst->dodajNotAndOrXor(2, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 19){ //xor

    kontekst->dodajNotAndOrXor(3, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 20){ //shl

    kontekst->dodajShlShr(0, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 21){ //shr

    kontekst->dodajShlShr(1, linija.instrukcija.regA, linija.instrukcija.regB);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 22){ //ld
    
    //ovde treba da podrzimo sve modove osim 1 i 5
    kontekst->dodajLd(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;

  }else if(linija.instrukcija.instrukcija == 23){ //st

    kontekst->dodajSt(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 24){ //csrrd

    //u okviru ld -> 1. varijanta
    kontekst->dodajCsrrd(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
    
  }else if(linija.instrukcija.instrukcija == 25){ //csrwr
    
    //u okviru ld -> 5. varijanta
    kontekst->dodajCsrwr(linija.instrukcija);
    kontekst->locationCounter = kontekst->locationCounter + 4;
  }

}

void glavnaPetlja(vector<Linija>& kontekst_fajla, int broj_linija, char* objFajl, char* debugFajl){

  Kontekst* kontekst = new Kontekst();

  for(int i = 0; i < broj_linija; i++){  
    if(kontekst_fajla[i].vrsta == 0 && kontekst->kraj == false){   // ako je linija prazna - ne radi se nista (samo se doda labela ako treba)
      if(kontekst_fajla[i].lab == true){
        kontekst->dodajSimbol(kontekst_fajla[i].labela, kontekst->locationCounter, false, true, false, false);
      }else{
        continue;
      }
    }else if( kontekst_fajla[i].vrsta == 1 && kontekst->kraj == false){  // direktiva
      direktiva_handler(kontekst_fajla[i], kontekst);

    }else if(kontekst_fajla[i].vrsta == 2 && kontekst->kraj == false){   // instrukcija
      instrukcija_handler(kontekst_fajla[i], kontekst);

    }
  }

  kontekst->postaviVelicinuPoslednjeSekcije(); //ovo mora sada da se uradi posto poslednja sekcija moze da ostane sa nenamestenom velicinom ako je veca od 0

  kontekst->proveraSimbola();  //ova metoda proverava da li ima neki lokalni ili globalni simbol da je ostao nedefinisan

  //do sada su literali bili svi smesteni u listu onu tj. tabelu medjutim sada mozemo da napravimo bazen kao neki vektor posto su sbvi na 32b da posle bude lakse da se samo nalepi svaki na svoju sekciju
  kontekst->napraviBazeneLiterala();  

  backpatching(kontekst);

  //treba poravnati sekcije na 4 tj treba im velicine dodati na 4 tako sto cemo da dodamo skipbajtove onoliko koliko fali
  kontekst->poravnajSekcije();

  //pozivanje  metoda za formiranje fajlova
  formiranjeObjektnogFajla(kontekst, objFajl);
  formiranjeTekstualnogFajla(kontekst, debugFajl);
}