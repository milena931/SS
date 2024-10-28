#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "../../inc/emulator/emulator.hpp"
#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>



Emulator::Emulator(char *program)
{
    tcgetattr(STDIN_FILENO, &this->term);

    // Ikljucivanje eha i baferisanja
    this->term.c_lflag &= ~(ECHO | ICANON);
    this->term.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &this->term);

    this->program = program;
    this->registri[15] = 0x40000000; // uvek se izvrsava od ove adrese
    this->registri[0] = 0;
    this->term_in = 0xffffff04;
    this->term_out = 0xffffff00;
}

void Emulator::postaviMemoriju(uint32_t addr, uint32_t podatak)
{
    if (addr >= 0xffffff00LL - 3)
    { // ovo je periferija terminal
        //cout<<endl<<std::hex<<addr<<" "<<std::hex<<this->term_out<<" podatak: "<<std::hex<<podatak;
        if (addr == this->term_out)
        {
            cout<< static_cast<char>(podatak & 0xff)<<std::flush;  //ovo znaci da je upis u term out i da treba da se ispise rezultat
        }
        else if (addr == this->term_in)
        {
            for (int i = 0; i < 4; i++)
            {
                this->kontekst[addr + i] = podatak & 0xff;
                podatak = podatak >> 8;
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            this->kontekst[addr + i] = podatak & 0xff;
            podatak = podatak >> 8;
        }
    }
}

uint32_t Emulator::dohvatiIzMemorije(uint32_t addr)
{
    uint32_t byte0 = this->kontekst[addr + 3];
    uint32_t byte1 = this->kontekst[addr + 2];
    uint32_t byte2 = this->kontekst[addr + 1];
    uint32_t byte3 = this->kontekst[addr];

    return ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
}

int Emulator::tasterPritisnut()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void Emulator::procitajFaj()
{
    // ova metoda treba da formira kontekst po lokacijama dok cita fajl
    // lc ce da broj likacije i njega cemo da upisujemo u map
    // vrednost koju mecujemo sa adresom je jedan baj -> adresibilna jedinica
    ifstream ulaz(this->program);

    if (!ulaz.is_open())
    {
        cerr << "Greška pri otvaranju fajla: " << this->program << endl;
        exit(1);
    }

    string linija;
    while (getline(ulaz, linija))
    {
        // Proveri da li linija sadrži ':'
        size_t dvotacka = linija.find(':');
        if (dvotacka == string::npos)
        {
            cerr << "Neispravna linija formata: " << linija << endl;
            continue;
        }

        // Prvo uzmi lc sa početka linije
        string lcStr = linija.substr(0, dvotacka);
        if (lcStr.length() > 8)
        { // Provera veličine za 32-bitni int
            cerr << "Los format hex fajla" << endl;
            continue;
        }
        uint32_t lc = stoul(lcStr, nullptr, 16);

        // Parsiraj heksadecimalne vrednosti nakon ':'
        istringstream hexStream(linija.substr(dvotacka + 1));
        string byteStr;
        while (hexStream >> byteStr)
        {
            // Ignoriši praznine i proveri da li je validan heksadecimalni bajt
            if (byteStr.length() == 2 && isxdigit(byteStr[0]) && isxdigit(byteStr[1]))
            {
                uint8_t byte = stoul(byteStr, nullptr, 16);
                this->kontekst[lc] = static_cast<int>(byte);
                lc++;
            }
            else
            {
                cerr << "Neispravna heksadecimalna veličina bajta: " << byteStr << endl;
            }
        }
    }
    ulaz.close();
}

void Emulator::izvrsiAr(int regA, int regB, int regC, int mod)
{
    if (regA == 0)
    {
        return; // u ovom slucaju treba da bude bez efekta
    }
    switch (mod)
    {
    case 0: // add
        this->registri[regA] = this->registri[regB] + this->registri[regC];
        break;
    case 1: // sub
        this->registri[regA] = this->registri[regB] - this->registri[regC];
        break;
    case 2: // mul
        this->registri[regA] = this->registri[regB] * this->registri[regC];
        break;
    case 3: // div
        this->registri[regA] = this->registri[regB] / this->registri[regC];
        break;

    default:
        cerr << endl
             << "Nepostojeci mod aritmeticke instrukcije: " << mod << endl;
        this->nespostojecaInstrukcija();
        break;
    }
}

void Emulator::izvrsiLog(int regA, int regB, int regC, int mod)
{
    if (regA == 0)
    {
        return; // u ovom slucaju treba da bude bez efekta
    }
    switch (mod)
    {
    case 0: // not
        this->registri[regA] = ~this->registri[regB];
        break;
    case 1: // and
        this->registri[regA] = this->registri[regB] & this->registri[regC];
        break;
    case 2: // or
        this->registri[regA] = this->registri[regB] | this->registri[regC];
        break;
    case 3: // xor
        this->registri[regA] = this->registri[regB] ^ this->registri[regC];
        break;

    default:
        cerr << endl
             << "Nepostojeci mod logicke instrukcije: " << mod << endl;
        this->nespostojecaInstrukcija();
        break;
    }
}
void Emulator::izvrsiPomeracku(int regA, int regB, int regC, int mod)
{
    if (regA == 0)
    {
        return; // u ovom slucaju treba da bude bez efekta
    }
    switch (mod)
    {
    case 0: // levo
        this->registri[regA] = this->registri[regB] << this->registri[regC];
        break;
    case 1: // desno
        this->registri[regA] = this->registri[regB] >> this->registri[regC];
        break;

    default:
        cerr << endl
             << "Nepostojeci mod pomeracke instrukcije: " << mod << endl;
        this->nespostojecaInstrukcija();
        break;
    }
}

void Emulator::izvrsiXchg(int regA, int regB, int regC, int mod)
{
    uint32_t temp = registri[regB];
    if (regB != 0)
    {
        registri[regB] = registri[regC];
    }
    if (regC != 0)
    {
        registri[regC] = temp;
    }
}

void Emulator::izvrsiSt(int regA, int regB, int regC, int mod, uint32_t disp)
{
    switch (mod)
    {
    case 0:
        //cout << " mesto smestanja: " << std::hex << this->registri[regA] + this->registri[regB] + disp << " registri r" << regA << " r" << regB << " podatak: " << std::hex << this->registri[regC] << " registar r" << regC;
        this->postaviMemoriju(this->registri[regA] + this->registri[regB] + disp, this->registri[regC]);
        break;
    case 1:
        //cout << " mesto smestanja: " << std::hex << this->dohvatiIzMemorije(this->registri[regA] + this->registri[regB] + disp) << " registri r" << regA << " r" << regB << " podatak: " << std::hex << this->registri[regC] << " registar r" << regC;
        this->postaviMemoriju(this->dohvatiIzMemorije(this->registri[regA] + this->registri[regB] + disp), this->registri[regC]);
        break;
    case 2:
        this->registri[regA] = this->registri[regA] + disp;
        this->postaviMemoriju(this->registri[regA], this->registri[regC]);
        //cout << " mesto smestanja: " << std::hex << this->registri[regA] << " podatak: " << std::hex << this->registri[regC];
        break;
    default:
        cerr << endl
             << "Ilegalni mod st instrukcije: " << mod << endl;
        this->nespostojecaInstrukcija();
    }
}

void Emulator::izvrsiLd(int regA, int regB, int regC, int mod, uint32_t disp)
{
    if (regA == 0)
    {
        return;
    }
    switch (mod)
    {
    case 0:
        if (regB == 0)
        {
            this->registri[regA] = this->status;
        }
        else if (regB == 1)
        {
            this->registri[regA] = this->handler;
        }
        else if (regB == 2)
        {
            this->registri[regA] = this->cause;
        }
        else
        {
            cerr << endl
                 << "Greska prilikom upisa csr registra" << endl;
            this->nespostojecaInstrukcija();
        }

        break;
    case 1:
        this->registri[regA] = this->registri[regB] + disp;
        //cout << " " << std::hex << this->registri[regA] << " registar: r" << regA;
        break;
    case 2:
        this->registri[regA] = this->dohvatiIzMemorije(this->registri[regB] + this->registri[regC] + disp);
        //cout  << " " << std::hex << this->registri[regA] << " registar: r" << regA;
        break;
    case 3:
        this->registri[regA] = this->dohvatiIzMemorije(this->registri[regB]);
        this->registri[regB] = this->registri[regB] + disp;
        break;
    case 4:
        if (regA == 0)
        {
            this->status = this->registri[regB];
        }
        else if (regA == 1)
        {
            this->handler = this->registri[regB];
        }
        else if (regA == 2)
        {
            this->cause = this->registri[regB];
        }
        else
        {
            cerr << endl
                 << "Greska prilikom upisa u csr registar" << endl;
            this->nespostojecaInstrukcija();
        }
        break;
    case 5:
        if (regA == 0)
        {
            if (regB == 0)
            {
                this->status = this->status | disp;
            }
            else if (regB == 1)
            {
                this->status = this->handler | disp;
            }
            else if (regB == 2)
            {
                this->status = this->cause | disp;
            }
            else
            {
                cerr << endl
                     << "Greska prilikom upisa u csr registar" << endl;
                this->nespostojecaInstrukcija();
            }
        }
        else if (regA == 1)
        {
            if (regB == 0)
            {
                this->handler = this->status | disp;
            }
            else if (regB == 1)
            {
                this->handler = this->handler | disp;
            }
            else if (regB == 2)
            {
                this->handler = this->cause | disp;
            }
            else
            {
                cerr << endl
                     << "Greska prilikom upisa u csr registar" << endl;
                this->nespostojecaInstrukcija();
            }
        }
        else if (regA == 2)
        {
            if (regB == 0)
            {
                this->cause = this->status | disp;
            }
            else if (regB == 1)
            {
                this->cause = this->handler | disp;
            }
            else if (regB == 2)
            {
                this->cause = this->cause | disp;
            }
            else
            {
                cerr << endl
                     << "Greska prilikom upisa u csr registar" << endl;
            }
        }
        else
        {
            cerr << endl
                 << "Greska prilikom upisa u csr registar" << endl;
            this->nespostojecaInstrukcija();
        }
        cout << " " << std::hex << this->registri[regA] << " registar: r" << regA;
        break;
    case 6:
        if (regA == 0)
        {
            this->status = this->dohvatiIzMemorije(this->registri[regB] + this->registri[regC] + disp);
        }
        else if (regA == 1)
        {
            this->handler = this->dohvatiIzMemorije(this->registri[regB] + this->registri[regC] + disp);
            
        }
        else if (regA == 2)
        {
            this->cause = this->dohvatiIzMemorije(this->registri[regB] + this->registri[regC] + disp);
        }
        else
        {
            cerr << endl
                 << "Greska prilikom upisa u csr registar" << endl;
            this->nespostojecaInstrukcija();
        }
        break;
    case 7:
        if (regA == 0)
        {
            this->status = this->dohvatiIzMemorije(this->registri[regB]);
            this->registri[regB] = this->registri[regB] + disp;
        }
        else if (regA == 1)
        {
            this->handler = this->dohvatiIzMemorije(this->registri[regB]);
            this->registri[regB] = this->registri[regB] + disp;
        }
        else if (regA == 2)
        {
            this->cause = this->dohvatiIzMemorije(this->registri[regB]);
            this->registri[regB] = this->registri[regB] + disp;
        }
        else
        {
            cerr << endl
                 << "Greska prilikom upisa u csr registar" << endl;
        }
        break;
    default:
        cerr<<endl<<"Nepostojeci mod ld instrukcije"<<endl;
        this->nespostojecaInstrukcija();
    }
}

void Emulator::izvrsiSkok(int regA, int regB, int regC, int mod, uint32_t disp)
{
    if (mod == 0)
    {
        this->registri[15] = this->registri[regA] + disp;
        //cout << endl << "Pc na koji se skace: " << std::hex << this->registri[15];
    }
    else if (mod == 1)
    {
        if (this->registri[regB] == this->registri[regC])
        {
            this->registri[15] = this->registri[regA] + disp;
            //cout << endl << "Pc na koji se skace: " << std::hex << this->registri[15];
        }
    }
    else if (mod == 2)
    {
        if (this->registri[regB] != this->registri[regC])
        {
            this->registri[15] = this->registri[regA] + disp;
        }
    }
    else if (mod == 3)
    {
        if ((signed)this->registri[regB] > (signed)this->registri[regC])
        {
            this->registri[15] = this->registri[regA] + disp;
        }
    }
    else if (mod == 4)
    {
        this->registri[15] = this->dohvatiIzMemorije(this->registri[regA] + disp);
    }
    else if (mod == 5)
    {
        if (this->registri[regB] == this->registri[regC])
        {
            this->registri[15] = this->dohvatiIzMemorije(this->registri[regA] + disp);
        }
    }
    else if (mod == 6)
    {
        if (this->registri[regB] != this->registri[regC])
        {
            this->registri[15] = this->dohvatiIzMemorije(this->registri[regA] + disp);
        }
    }
    else if (mod == 7)
    {
        if ((signed)this->registri[regB] > (signed)this->registri[regC])
        {
            this->registri[15] = this->dohvatiIzMemorije(this->registri[regA] + disp);
        }
    }
    else
    {
        this->nespostojecaInstrukcija();
    }
}

void Emulator::izvrsiCall(int regA, int regB, int regC, int mod, uint32_t disp)
{
    // prva stvar je push pc
    this->registri[14] -= 4;
    this->postaviMemoriju(this->registri[14], this->registri[15]);
    // zatim ucitavano novi pc
    if (mod == 0)
    {
        this->registri[15] = this->registri[regA] + this->registri[regB] + disp;
        //cout << endl << "Pc na koji se skace - call: " << std::hex << this->registri[15];
    }
    else if (mod == 1)
    {
        //cout << endl << "pc pre: " << std::hex << this->registri[15] << " izraz: " << std::hex << (this->registri[regA] + this->registri[regB] + disp);
        this->registri[15] = this->dohvatiIzMemorije(this->registri[regA] + this->registri[regB] + disp);
        //cout << endl << "Pc na koji se skace - call: " << std::hex << this->registri[15];
    }
    else
    {
        cerr<<endl<<"Nepostojeci mod call instrukcije"<<endl;
        this->nespostojecaInstrukcija();
    }
}

void Emulator::izvrsiInt()
{
    // push status
    // push pc
    // cause <= 4
    // status <= status & (~0x1)
    // pc <= handle
    this->registri[14] -= 4;
    this->postaviMemoriju(this->registri[14], this->status);
    this->registri[14] -= 4;
    this->postaviMemoriju(this->registri[14], this->registri[15]);
    this->cause = 4;
    this->status = this->status & (~0x1);
    this->registri[15] = this->handler;
}

void Emulator::nespostojecaInstrukcija(){
    // push status
    // push pc
    // cause <= 1
    // status <= status & (~0x1)
    // pc <= handle
    this->registri[14] -= 4;
    this->postaviMemoriju(this->registri[14], this->status);
    this->registri[14] -= 4;
    this->postaviMemoriju(this->registri[14], this->registri[15]);
    this->cause = 1;
    this->status = this->status & (~0x1);
    this->registri[15] = this->handler;
}

void Emulator::izvrsi()
{
    bool izvrsi = true;
    uint8_t oc_mod = 0;
    uint8_t regA_regB = 0;
    uint8_t regC_disp1 = 0;
    uint8_t disp2_3 = 0;

    while (izvrsi)
    {
        // uzima se 4 po 4 bajta i onda se gleda koja je instrukcija u pitanju
        oc_mod = this->kontekst[this->registri[15] + 3];
        regA_regB = this->kontekst[this->registri[15] + 2];
        regC_disp1 = this->kontekst[this->registri[15] + 1];
        disp2_3 = this->kontekst[this->registri[15]];
        // cout<<endl<<hex<<setw(2)<<setfill('0')<<static_cast<int>(oc_mod)<<hex<<setw(2)<<setfill('0')<<static_cast<int>(regA_regB)<<hex<<setw(2)<<setfill('0')<<static_cast<int>(regC_disp1)<<hex<<setw(2)<<setfill('0')<<static_cast<int>(disp2_3);
        this->registri[15] += 4;
        // cout<<endl<<" oc: "<<std::hex<<((oc_mod & 0xf0)>>4);
        if (((oc_mod & 0xf0) >> 4) == OC_HALT)
        { // halt
            //cout << endl << "halt";
            izvrsi = false;
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_AR)
        { // ar instrukcije
            //cout << endl << "ar instrukcija";
            this->izvrsiAr((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_LOG)
        { // log instrukcije
            //cout << endl << "log instrukcija";
            this->izvrsiLog((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_SH)
        { // pomeracke instrukcije
            //cout << endl << "pomeracka instrukcija";
            this->izvrsiPomeracku((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_INT)
        { // int
            //cout << endl << "int";
            this->izvrsiInt();
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_JMP)
        { // instrukcije skoka
            //cout << endl << "instrukcija skoka";
            uint32_t disp = ((regC_disp1 & 0x0f) << 8) | disp2_3;
            if (disp & 0x800)
            {
                disp = disp | 0xfffff000;
            }
            this->izvrsiSkok((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f, disp);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_LD)
        { // ld
            //cout << endl << "ld";
            uint32_t disp = ((regC_disp1 & 0x0f) << 8) | disp2_3;
            if (disp & 0x800)
            {
                disp = disp | 0xfffff000;
            }
            this->izvrsiLd((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f, disp);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_ST)
        { // st
            //cout << endl << "st";
            uint32_t disp = ((regC_disp1 & 0x0f) << 8) | disp2_3;
            if (disp & 0x800)
            {
                disp = disp | 0xfffff000;
            }
            this->izvrsiSt((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f, disp);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_XCHG)
        { // xchg
            //cout << endl << "xchg";
            this->izvrsiXchg((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f);
        }
        else if (((oc_mod & 0xf0) >> 4) == OC_CALL)
        { // call
            //cout << endl << "call";
            uint32_t disp = ((regC_disp1 & 0x0f) << 8) | disp2_3;
            if (disp & 0x800)
            {
                disp = disp | 0xfffff000;
            }
            //cout<<endl<<"call disp "<<hex<<setw(8)<<setfill('0')<<static_cast<int>(disp);
            this->izvrsiCall((regA_regB & 0xf0) >> 4, regA_regB & 0x0f, (regC_disp1 & 0xf0) >> 4, oc_mod & 0x0f, disp);
        }
        else
        {
            cerr << endl
                 << "Greska: Nepoznati operacioni kod instrukcije: " << ((oc_mod & 0xf0) >> 4) << endl;
            this->nespostojecaInstrukcija();
        }

        if (this->tasterPritisnut())
        { // prekid od terminala koji treba da se obradi
            int ch = getchar();
            this->postaviMemoriju(this->term_in, ch);
            if ((this->status & 0x3) == 0)
            {
                // obrada prekida od terminala -> 3
                //  push status
                //  push pc
                //  cause <= 3
                //  status <= status & (~0x1)
                //  pc <= handle
                this->registri[14] -= 4;
                this->postaviMemoriju(this->registri[14], this->status);
                this->registri[14] -= 4;
                this->postaviMemoriju(this->registri[14], this->registri[15]);
                this->cause = 3;
                this->status = this->status & (~0x1);  //maskiranje prekida
                this->registri[15] = this->handler;  //prekidna rutina
            }
        }
    }
    this->term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &this->term);
    ispisi();
}

void Emulator::ispisi()
{
    cout << endl
         << "Emulated processor executed halt instruction" << endl
         << "Emulated processor state: " << endl;
    cout << "r0: 0x" << hex << setw(8) << setfill('0') << this->registri[0] << "    r1: 0x" << hex << setw(8) << setfill('0') << this->registri[1] << "    r2: 0x" << hex << setw(8) << setfill('0') << this->registri[2] << "    r3: 0x" << hex << setw(8) << setfill('0') << this->registri[3] << endl;
    cout << "r4: 0x" << hex << setw(8) << setfill('0') << this->registri[4] << "    r5: 0x" << hex << setw(8) << setfill('0') << this->registri[5] << "    r6: 0x" << hex << setw(8) << setfill('0') << this->registri[6] << "    r7: 0x" << hex << setw(8) << setfill('0') << this->registri[7] << endl;
    cout << "r8: 0x" << hex << setw(8) << setfill('0') << this->registri[8] << "    r9: 0x" << hex << setw(8) << setfill('0') << this->registri[9] << "   r10: 0x" << hex << setw(8) << setfill('0') << this->registri[10] << "   r11: 0x" << hex << setw(8) << setfill('0') << this->registri[11] << endl;
    cout << "r12: 0x" << hex << setw(8) << setfill('0') << this->registri[12] << "  r13: 0x" << hex << setw(8) << setfill('0') << this->registri[13] << "   r14: 0x" << hex << setw(8) << setfill('0') << this->registri[14] << "   r15: 0x" << hex << setw(8) << setfill('0') << this->registri[15] << endl;
}

Emulator::~Emulator()
{

}