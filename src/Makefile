
include ../CONFIG.mine

CC = g++

CFLAGS = -Wall -std=c++11 -pedantic -Wextra -g -pthread -I$(ROOT)/src -maes -mpclmul -msse4.1 -mavx -march=core2 $(FLAGS) $(OPT) -I$(OSSL)/include
CPPFLAGS = $(CFLAGS)
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: $(LIB) Setup.x Player.x Api.x Client-Api.x

api: $(LIB) Api.x Client-Api.x

OFFLINE = $(patsubst %.cpp,%.o,$(wildcard Offline/*.cpp))
ONLINE = $(patsubst %.cpp,%.o,$(wildcard Online/*.cpp))
SYSTEM = $(patsubst %.cpp,%.o,$(wildcard System/*.cpp))
TOOLS = $(patsubst %.cpp,%.o,$(wildcard Tools/*.cpp))
MATH = $(patsubst %.cpp,%.o,$(wildcard Math/*.cpp))
LSSS = $(patsubst %.cpp,%.o,$(wildcard LSSS/*.cpp))
PROCESSOR = $(patsubst %.cpp,%.o,$(wildcard Processor/*.cpp))
IO = $(patsubst %.cpp,%.o,$(wildcard Input_Output/*.cpp))
FHE = $(patsubst %.cpp,%.o,$(wildcard FHE/*.cpp))
OT = $(patsubst %.cpp,%.o,$(wildcard OT/*.cpp))
GC = $(patsubst %.cpp,%.o,$(wildcard GC/*.cpp))
EXCEPTIONS = $(patsubst %.cpp,%.o,$(wildcard Exceptions/*.cpp))

APP = $(patsubst %.cpp,%.o,$(wildcard App/*.cpp))

LIB = libMPC.a

OBJ = Setup.o Player.o Api.o Client-Api.o

API_OBJ = Api.o
API_SRC = Api.cpp

SRC = Setup.cpp Player.cpp Api.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $*.cpp

$(API_OBJ): $(API_SRC)
	$(CC) $(CFLAGS) -c $*.cpp

$(LIB): $(SYSTEM) $(MATH) $(TOOLS) $(LSSS) $(OFFLINE) $(ONLINE) $(PROCESSOR) $(IO) $(FHE) $(OT) $(GC) $(EXCEPTIONS) $(APP)
	$(AR) -crs libMPC.a $^

Setup.x: $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -o Setup.x Setup.o $(LIB) $(LDLIBS)
	- mv Setup.x ../

Player.x: $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -o Player.x Player.o $(LIB) $(LDLIBS)
	- mv Player.x ../

Api.x: $(API_OBJ) $(LIB)
	$(CC) $(CFLAGS) -o Api.x Api.o $(LIB) $(LDLIBS)
	- mv Api.x ../

Client-Api.x: $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -o Client-Api.x Client-Api.o $(LIB) $(LDLIBS)
	- mv Client-Api.x ../


clean:
	- rm *.x *.o libMPC.a
	- cd System ; rm *.o
	- cd Tools ; rm *.o
	- cd Math ; rm *.o
	- cd LSSS ; rm *.o
	- cd Processor ; rm *.o
	- cd Offline ; rm *.o
	- cd Online ; rm *.o
	- cd Input_Output ; rm *.o
	- cd FHE ; rm *.o
	- cd OT ; rm *.o
	- cd GC ; rm *.o
	- cd Exceptions ; rm *.o
	- cd App; rm *.o
