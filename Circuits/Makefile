include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++11 -pedantic -Wextra -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -march=core2 $(FLAGS) $(OPT)
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

LIB = ../src/libMPC.a

CPPFLAGS = $(CFLAGS) 

all: 
	$(CC) $(CFLAGS) -o convert.x convert.cpp $(LIB) $(LDLIBS)
	$(CC) $(CFLAGS) -o process.x process.cpp $(LIB) $(LDLIBS)
	$(CC) $(CFLAGS) -o make-mand.x make-mand.cpp $(LIB) $(LDLIBS)

clean:
	-rm *.x
	-cd VHDL; $(MAKE) clean

