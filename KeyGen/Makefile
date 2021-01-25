include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++14 -pedantic -Wextra -g -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -mavx -march=core2 $(FLAGS) $(OPT) -I$(OSSL)/include 
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: $(LIB) KeyGen.x 

KEYGEN = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

LIBMPC = ../src/libMPC.a

LIBKG = libKeyGen.a

%.o: %.cpp %.d
	$(CC) $(CFLAGS) -o $@ -c $< 

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $*.cpp

$(LIBKG): $(KEYGEN) 
	$(AR) -crs libKeyGen.a $^

KeyGen.x: PlayerKeyGen.o $(LIBKG)
	$(CC) $(CFLAGS) -o PlayerKeyGen.x PlayerKeyGen.o $(LIBKG) $(LIBMPC) $(LDLIBS)
	- mv PlayerKeyGen.x ../
clean:
	- rm *.o
	- rm ../PlayerKeyGen.x
