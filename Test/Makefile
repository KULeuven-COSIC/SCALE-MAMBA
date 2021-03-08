
include ../CONFIG.mine

CC = g++

ARCH = -mtune=native -maes -mpclmul -msse4.1 -mavx -mbmi2 -madx -march=core2
CFLAGS = -Wall -std=c++14 -pedantic -Wextra  -pthread -I$(ROOT)/src $(ARCH) $(FLAGS) $(OPT)
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: Test.x Test-FHE.x Test-Adv-FHE.x Test-Circuit.x FHE-P.x Test-Simplify.x Test-Convert.x  Test-FP.x Test-gfp.x Test-Mod2-CAS.x Test-Merge.x

LIB = ../src/libMPC.a

.PRECIOUS: %.o
%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $< 

%.x: %.o
	$(CC) $(CFLAGS) -o $@ $< $(LIB) $(LDLIBS)
	- mv $@ ../

clean:
	- rm *.o *.x
