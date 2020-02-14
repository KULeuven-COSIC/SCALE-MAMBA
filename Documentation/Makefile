
all: $(wildcard *.tex)
	cd ../Assembler ; cargo test generate_instruction_tex_table
	pdflatex Documentation-SCALE.tex
	bibtex Documentation-SCALE
	pdflatex Documentation-SCALE.tex
	pdflatex Documentation-SCALE.tex

clean:
	rm *.pdf *.aux *.log *.out *.toc texput.log


