
all: $(wildcard *.tex)
	pdflatex Documentation.tex
	bibtex Documentation
	pdflatex Documentation.tex
	pdflatex Documentation.tex

clean:
	rm *.pdf *.aux *.log *.out *.toc texput.log


