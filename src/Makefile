include ../CONFIG.mine

CC = g++

ARCH = -mtune=native -maes -mpclmul -msse4.1 -mavx -mbmi2 -madx -march=core2
CFLAGS = -Wall -std=c++14 -pedantic -Wextra -pthread -I$(ROOT)/src $(ARCH) $(FLAGS) $(OPT) -I$(OSSL)/include
CPPFLAGS = $(CFLAGS)
LDLIBS = -L/$(OSSL)/lib -lm -lssl -lcrypto -lmpirxx -lmpir -lcryptopp $(LDFLAGS)

all: $(LIB) Setup.x Player.x

OFFLINE = $(patsubst %.cpp,%.o,$(wildcard Offline/*.cpp))
ONLINE = $(patsubst %.cpp,%.o,$(wildcard Online/*.cpp))
SYSTEM = $(patsubst %.cpp,%.o,$(wildcard System/*.cpp))
TOOLS = $(patsubst %.cpp,%.o,$(wildcard Tools/*.cpp))
MATH = $(patsubst %.cpp,%.o,$(wildcard Math/*.cpp))
LSSS = $(patsubst %.cpp,%.o,$(wildcard LSSS/*.cpp))
OT = $(patsubst %.cpp,%.o,$(wildcard OT/*.cpp))
MOD2ENGINE = $(patsubst %.cpp,%.o,$(wildcard Mod2Engine/*.cpp))
PROCESSOR = $(patsubst %.cpp,%.o,$(wildcard Processor/*.cpp))
IO = $(patsubst %.cpp,%.o,$(wildcard Input_Output/*.cpp))
FHE = $(patsubst %.cpp,%.o,$(wildcard FHE/*.cpp))
GC = $(patsubst %.cpp,%.o,$(wildcard GC/*.cpp))
LOCAL_FUNC = $(patsubst %.cpp,%.o,$(wildcard Local/*.cpp))
EXCEPTIONS = $(patsubst %.cpp,%.o,$(wildcard Exceptions/*.cpp))

LIB = libMPC.a

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $*.cpp

$(LIB): $(SYSTEM) $(MATH) $(TOOLS) $(LSSS) $(ONLINE) $(OT) $(MOD2ENGINE) $(PROCESSOR) $(IO) $(FHE) $(GC) $(LOCAL_FUNC) $(EXCEPTIONS) $(OFFLINE)
	$(AR) -crs libMPC.a $^

Setup.x: Setup.o $(LIB)
	$(CC) $(CFLAGS) -o Setup.x Setup.o $(LIB) $(LDLIBS)
	- mv Setup.x ../SetupBinary.x

Player.x: Player.o $(LIB)
	$(CC) $(CFLAGS) -o Player.x Player.o $(LIB) $(LDLIBS)
	- mv Player.x ../PlayerBinary.x

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
	- cd Mod2Engine ; rm *.o
	- cd GC ; rm *.o
	- cd Local ; rm *.o
	- cd Exceptions ; rm *.o
