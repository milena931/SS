#include <iostream>
#include <fstream>
#include "../../inc/asembler/parser.hpp"

using namespace std;

extern FILE* yyin;  // Ulazni fajl za Flex


//komandna linija PROGRAM -o fajl.o fajl.s

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Greska: " << argv[0] << " <input file>" << endl;
        return 1;
    }

    if (std::string(argv[1]) != "-o") {
        cerr << "Nepostojeća opcija" << endl;;
        return 1;;
    }
    
    // Otvorite ulazni fajl
    FILE* inputFile = fopen(argv[3], "r");
    if (!inputFile) {
        perror("Ulazni fajl ne moze da se otvori ili ne postoji");
        return 1;
    }

    char* outputFile = argv[2];
    std::string outputFileStr(outputFile);
    size_t lastDotPos = outputFileStr.find_last_of('.');
    std::string debugFileStr;

    if (lastDotPos != std::string::npos) {
        debugFileStr = outputFileStr.substr(0, lastDotPos) + ".txt";
    } else {
        // Ako nema tačke, samo dodaj ".txt" na kraj
        debugFileStr = outputFileStr + ".txt";
    }

    // Pretvori std::string u char*
    char* debugFile = strdup(debugFileStr.c_str());

    yyin = inputFile;

    Parser* parser = new Parser(outputFile, debugFile);
    
    delete parser;

    // Zatvorite ulazni fajl
    fclose(inputFile);

    // Ovo je mesto gde biste obavili backpatching ili druge operacije na parsiranim podacima
    // Za primer, samo ispišemo poruku da je parsiranje uspešno
    cout << endl << "Uspesno asembliranje! Faj: "<< argv[3] << endl;

    return 0;
}
