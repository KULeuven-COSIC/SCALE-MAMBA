clean:
	-cd tests  ; rm -f *.bc
	-cd tests/scasm-tests ; rm -f *.bc

scasm-tests:
	-$(MAKE) clean
	cargo run --bin scasm_tests --release

tests:
	-$(MAKE) clean
	cargo test
