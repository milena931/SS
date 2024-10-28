#include <iostream>
#include "string.h"
#include "../../inc/emulator/emulator.hpp"

using namespace std;

int main(int argc, char** argv) {
    
    if(argc < 2){
        cerr<<"Emulator greška: niste naveli program za emuliranje"<<endl;
        exit(1);
    }
    char* program = argv[1];
    Emulator* emulator = new Emulator(program);
    emulator->procitajFaj();
    emulator->izvrsi();

    delete emulator;

    return 0;
}
