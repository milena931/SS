A = src/asembler

# Programi
CC = g++
LEXER = flex
PARSER = bison

# Parser flegovi
PFLAGS = -Wall -d -o flexbison.tab.cpp

# Compajler flegovi
CFLAGS = -Wall -Wextra -Iinc -I. -std=c++17 
ifdef DEBUG
CFLAGS += -ggdb -DDEBUG -O0
endif

# Build
.PHONY: default
default: assembler linker emulator

# Parser -  prvo se pokrece bison jer on definise tokene 
flexbison.tab.cpp flexbison.tab.hpp: misc/flexbison.ypp
	$(PARSER) $(PFLAGS) misc/flexbison.ypp

# Lekser 
lex.yy.c: misc/flexbison.l flexbison.tab.hpp
	$(LEXER) misc/flexbison.l

# Asembler
assembler: lex.yy.c flexbison.tab.cpp flexbison.tab.hpp src/asembler/*.cpp
	$(CC) $(CFLAGS) flexbison.tab.cpp lex.yy.c src/asembler/*.cpp -o assembler

linker: src/linker/*.cpp
	$(CC) $(CFLAGS) src/linker/*.cpp -o linker

emulator: src/emulator/*.cpp
	$(CC) $(CFLAGS) src/emulator/*.cpp -o emulator

clean:
	rm -f lex.yy.c
	rm -f flexbison.tab.cpp
	rm -f flexbison.tab.hpp
	rm -f assembler
	rm -f linker
	rm -f emulator
