
include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++11 -pedantic -Wextra -g -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -march=core2 $(FLAGS) $(OPT)
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir $(LDFLAGS)

all: Test.x Test-FHE.x Test-Adv-FHE.x 

LIB = ../src/libMPC.a

OBJ =  Test.o Test-FHE.o Test-Adv-FHE.o 

SRC = Test.cpp Test-FHE.cpp Test-Adv-FHE.cpp 

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
	

clean:
	- rm *.x *.o 
