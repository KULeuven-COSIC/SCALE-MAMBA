
include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++11 -pedantic -Wextra -g -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -march=core2 $(FLAGS) $(OPT)
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: Test.x Test-FHE.x Test-Adv-FHE.x Test-Circuit.x FHE-P.x

LIB = ../src/libMPC.a

OBJ =  Test.o Test-FHE.o Test-Adv-FHE.o Test-Circuit.o FHE-P.o

SRC = Test.cpp Test-FHE.cpp Test-Adv-FHE.cpp Test-Circuit.cpp FHE-P.cpp

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $*.cpp

Test.x: $(OBJ) 
	$(CC) $(CFLAGS) -o Test.x Test.o $(LIB) $(LDLIBS)
	- mv Test.x ../

Test-FHE.x: $(OBJ) 
	$(CC) $(CFLAGS) -o Test-FHE.x Test-FHE.o $(LIB) $(LDLIBS)
	- mv Test-FHE.x ../

Test-Adv-FHE.x: $(OBJ)
	$(CC) $(CFLAGS) -o Test-Adv-FHE.x Test-Adv-FHE.o $(LIB) $(LDLIBS)
	- mv Test-Adv-FHE.x ../

FHE-P.x: $(OBJ)
	$(CC) $(CFLAGS) -o FHE-P.x FHE-P.o $(LIB) $(LDLIBS)
	- mv FHE-P.x ../


Test-Circuit.x: $(OBJ)
	$(CC) $(CFLAGS) -o Test-Circuit.x Test-Circuit.o $(LIB) $(LDLIBS)
	- mv Test-Circuit.x ../

	

clean:
	- rm *.x *.o 
