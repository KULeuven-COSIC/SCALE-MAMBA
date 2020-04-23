all: 
	make progs
	make test
	make doc

progs:
	-cd src ; make

test:
	-cd Test ; make

doc: 
	-cd Documentation ; make
	-cd Compiler ; doxygen doxy.config

circuits:
	-cd Circuits ; make ; ./convert.sh

clean:
	-rm  *.x
	-cd src ; make clean
	-cd Test ; make clean
	-cd Circuits ; make clean 

pclean:
	-cd Programs ; rm */*.bc ; rm */*.sch ; rm */*.asm
	-cd Scripts ; rm -r logs

vclean:
	- make clean
	- make pclean
	- cd Documentation ; make clean

format:
	- clang-format -i */*.cpp
	- clang-format -i */*/*.h
	- clang-format -i */*/*.cpp
