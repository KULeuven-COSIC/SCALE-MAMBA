all:
	$(MAKE) progs
	$(MAKE) test
	$(MAKE) doc

progs:
	-cd src ; $(MAKE)

test:
	-cd Test ; $(MAKE)

doc:
	-cd Documentation ; $(MAKE)
	-cd RustDocumentation ; $(MAKE)
	-cd Compiler ; doxygen doxy.config

circuits:
	-cd Circuits ; $(MAKE) ; ./convert.sh

clean:
	-rm SetupBinary.x PlayerBinary.x
	-cd src ; $(MAKE) clean
	-cd Test ; $(MAKE) clean
	-cd Circuits ; $(MAKE) clean

pclean:
	-cd Programs ; rm */*.bc ; rm */*.sch ; rm */*.asm
	-cd Scripts ; rm -r logs

vclean:
	- $(MAKE) clean
	- $(MAKE) pclean
	- cd Documentation ; $(MAKE) clean

format:
	- clang-format -i */*.cpp
	- clang-format -i */*/*.h
	- clang-format -i */*/*.cpp
