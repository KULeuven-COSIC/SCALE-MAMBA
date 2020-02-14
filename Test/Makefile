
include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++11 -pedantic -Wextra  -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -march=core2 $(FLAGS) $(OPT)
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: Test.x Test-FHE.x Test-Adv-FHE.x Test-Circuit.x FHE-P.x Test-Simplify.x Test-Convert.x 

LIB = ../src/libMPC.a

.PRECIOUS: %.o
%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $< 

%.x: %.o
	$(CC) $(CFLAGS) -o $@ $< $(LIB) $(LDLIBS)
	- mv $@ ../

clean:
	- rm *.o 
