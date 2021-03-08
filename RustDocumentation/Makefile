
all: $(wildcard *.tex)
	pdflatex RustDocumentation.tex
	bibtex RustDocumentation
	pdflatex RustDocumentation.tex
	pdflatex RustDocumentation.tex

clean:
	rm *.pdf *.aux *.log *.out *.toc texput.log
