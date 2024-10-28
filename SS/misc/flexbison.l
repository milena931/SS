%{
  #include "inc/asembler/pomocneStrukture.hpp"
  #include <vector>

  #include "flexbison.tab.hpp"

  int linije = 1;
%}

%option noyywrap 
%option nounput

%%

[ \t]                   ;
halt                    { 
  //std::cout<<"HALT ";  
  return HALT; 
}
int                     { 
  //std::cout<<"INT ";  
  return INT; 
}
iret                    { 
  //std::cout<<"IRET ";  
  return IRET; 
}
call                    { 
  //std::cout<<"CALL ";  
  return CALL; 
}
ret                     { 
  //std::cout<<"RET ";  
  return RET; 
}
jmp                     { 
  //std::cout<<"JMP ";  
  return JMP; 
}
beq                     { 
  //std::cout<<"BEQ ";  
  return BEQ; 
}
bne                     { 
  //std::cout<<"BNE ";  
  return BNE; 
}
bgt                     { 
  //std::cout<<"BGT ";  
  return BGT; 
}
push                    { 
  //std::cout<<"PUSH ";  
  return PUSH; 
}
pop                     { 
  //std::cout<<"POP ";  
  return POP; 
}
xchg                    { 
  //std::cout<<"XCHG ";  
  return XCHG; 
}
add                     { 
  //std::cout<<"ADD ";  
  return ADD; 
}
sub                     { 
  //std::cout<<"SUB ";  
  return SUB; 
}
mul                     { 
  //std::cout<<"MUL ";  
  return MUL; 
}
div                     { 
  //std::cout<<"DIV ";  
  return DIV; 
}
not                     { 
  //std::cout<<"NOT ";  
  return NOT; 
} 
and                     { 
  //std::cout<<"AND ";  
  return AND; 
}
or                      { 
  //std::cout<<"OR ";  
  return OR; 
}
xor                     { 
  //std::cout<<"XOR ";  
  return XOR; 
}
shl                     { 
  //std::cout<<"SHL ";  
  return SHL; 
}
shr                     { 
  //std::cout<<"SHR "; 
   return SHR;
}
ld                      { 
  //std::cout<<"LD ";  
  return LD; 
}
st                      { 
  //std::cout<<"ST "; 
  return ST; 
}
csrrd                   { 
  //std::cout<<"CSRRD ";  
  return CSRRD; 
}
csrwr                   { 
  //std::cout<<"CSRWR ";  
  return CSRWR; 
}
".global"               { 
  //std::cout<<"GLOBAL ";  
  return GLOBAL; 
}
".extern"               { 
  //std::cout<<"EXTERN ";  
  return EXTERN; 
}
".section"              { 
  //std::cout<<"SECTION ";  
  return SECTION; 
}
".word"                 { 
  //std::cout<<"WORD ";  
  return WORD; 
}
".skip"                 { 
  //std::cout<<"SKIP ";  
  return SKIP; 
}
".ascii"                { 
  //std::cout<<"ASCII ";  
  return ASCII; 
}
".end"                  { 
  //std::cout<<"END ";  
  return END;
}
"+"                     { 
  //std::cout<<"PLUS ";  
  return PLUS;
}
"-"                     { 
  //std::cout<<"MINUS ";  
  return MINUS; 
}
"*"                     { 
  //std::cout<<"PUTA ";  
  return PUTA; 
}
"%"                     { 
  //std::cout<<"POSTO ";  
  return POSTO; 
}
"$"                     { 
  //std::cout<<"DOLAR ";  
  return DOLAR; 
}
"("                     { 
  //std::cout<<"LEVA_ZAGRADA ";  
  return LEVA_ZAGRADA; 
}
")"                     { 
  //std::cout<<"DESNA_ZAGRADA ";  
  return DESNA_ZAGRADA; 
}
"["                     { 
  //std::cout<<"LEVA_UGLASTA ";  
  return LEVA_UGLASTA; 
}
"]"                     { 
  //std::cout<<"DESNA_UGLASTA ";  
  return DESNA_UGLASTA; 
}
","                     { 
  //std::cout<<"ZAREZ ";  
  return ZAREZ; 
}
#.*                     { 
  //std::cout<<"KOMENTAR ";  
  return KOMENTAR; 
}
"\n"                    { 
  //std::cout<<"KRAJ_LINIJE ";  
  return KRAJ_LINIJE; 
}
[a-zA-Z_][a-zA-Z0-9_]*[ \t]*: {
  yylval.labela = strdup(yytext);

  int i = 0;
  while(yylval.labela[i] != '\0'){  //mora da se postavi vrednost labele u uniju
    if(yylval.labela[i] == ' ' || yylval.labela[i] == ':' || yylval.labela[i] == '\t'){
      yylval.labela[i] = '\0';
      break;
    }else{
      i++;
    }
  }

  //std::cout<<"LABELA "; 
  return LABELA;
}

r([0-9]|1[0-5])         {

  yylval.registar = atoi(yytext + 1);  //mora pokazivac na teskt da se pomeri pored slova r 

  //std::cout<<"REGISTAR "; 
  return REGISTAR;
}
"sp"                    { 
  if (isspace(yytext[yyleng]) || yytext[yyleng] == '\0') {
    yylval.registar = 14;
    //std::cout<<"REGISTAR ";
    return REGISTAR;
  }
}
"pc"                    { 
  if (isspace(yytext[yyleng]) || yytext[yyleng] == '\0') {
    yylval.registar = 15;
    //std::cout<<"REGISTAR ";
    return REGISTAR;
  }
 }
[1-9][0-9]*             { //decimalan broj
  yylval.broj = atoi(yytext); 
  //std::cout<<"BROJ"; 
  return BROJ; 
}
0x[0-9A-Fa-f]+         { //heksadecimalan broj
  yylval.broj = strtol(yytext + 2, NULL, 16); //ova funkcija pretvara
  //std::cout<<"BROJ "; 
  return BROJ;
}
0                       { 
  yylval.broj = 0; 
  //std::cout << "BROJ "; 
  return BROJ;
}
\"[^"]+\"               {
  yytext[strlen(yytext) - 1] = '\0';
  yytext = yytext + 1;
  yylval.simbol = strdup(yytext);
  //std::cout<<"STRING "; 
  return STRING;
}

[a-zA-Z_.][a-zA-Z0-9_]* {  //ovaj pattern je za simbole(kakve god simbole ili sekcije)
  yylval.simbol = strdup(yytext);
  //std::cout<<"SIMBOL "; 
  return SIMBOL;
}

\n                      {
  linije++;
  std::cout<<"KRAJ_LINIJE "<<std::endl; 
  return KRAJ_LINIJE;
}

%%