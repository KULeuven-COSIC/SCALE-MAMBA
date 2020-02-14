clean:
	-cd tests  ; rm -f *.bc
	-cd tests/scasm-tests ; rm -f *.bc

scasm-tests:
	-make clean
	cargo run --bin scasm_tests --release

tests:
	-make clean
	cargo test
