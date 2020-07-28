
include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++11 -pedantic -Wextra -g -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -mavx -march=core2 $(FLAGS) $(OPT) -I$(OSSL)/include 
CPPFLAGS = $(CFLAGS) 
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: $(LIB) KeyGen.x 

OFFLINE = $(patsubst %.cpp,%.o,$(wildcard ../src/Offline/*.cpp))
ONLINE = $(patsubst %.cpp,%.o,$(wildcard ../src/Online/*.cpp))
SYSTEM = $(patsubst %.cpp,%.o,$(wildcard ../src/System/*.cpp))
TOOLS = $(patsubst %.cpp,%.o,$(wildcard ../src/Tools/*.cpp))
MATH = $(patsubst %.cpp,%.o,$(wildcard ../src/Math/*.cpp))
LSSS = $(patsubst %.cpp,%.o,$(wildcard ../src/LSSS/*.cpp))
OT = $(patsubst %.cpp,%.o,$(wildcard ../src/OT/*.cpp))
PROCESSOR = $(patsubst %.cpp,%.o,$(wildcard ../src/Processor/*.cpp))
IO = $(patsubst %.cpp,%.o,$(wildcard ../src/Input_Output/*.cpp))
FHE = $(patsubst %.cpp,%.o,$(wildcard ../src/FHE/*.cpp))
GC = $(patsubst %.cpp,%.o,$(wildcard ../src/GC/*.cpp))
LOCAL_FUNC = $(patsubst %.cpp,%.o,$(wildcard ../src/Local/*.cpp))
EXCEPTIONS = $(patsubst %.cpp,%.o,$(wildcard ../src/Exceptions/*.cpp))
KEYGEN = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

LIB = ../src/libMPC.a

OBJ = ../src/Setup.o ../src/Player.o 

SRC = ../src/Setup.cpp ../src/Player.cpp 

%.o: %.cpp %.d
	$(CC) $(CFLAGS) -o $@ -c $< 

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $*.cpp

$(LIB): $(SYSTEM) $(MATH) $(TOOLS) $(LSSS) $(OFFLINE) $(ONLINE) $(OT) $(PROCESSOR) $(IO) $(FHE) $(GC) $(LOCAL_FUNC) $(EXCEPTIONS) $(KEYGEN) 
	$(AR) -crs ../src/libMPC.a $^

KeyGen.x: $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -o PlayerKeyGen.x PlayerKeyGen.o $(LIB) $(LDLIBS)
	- mv PlayerKeyGen.x ../
clean:
	- rm *.o
	- rm ../PlayerKeyGen.x
