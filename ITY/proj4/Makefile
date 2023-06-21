all:
	latex proj4.tex
	bibtex proj4
	latex proj4.tex
	latex proj4.tex
	dvips proj4.dvi
	ps2pdf -sPAPERSIZE=a4 proj4.ps

clean:
	-rm -f proj4.aux proj4.dvi proj4.log proj4.ps proj4.pdf proj4.bbl

zip:
	zip xmimoc01-fit.zip proj4.tex czechiso.bst proj4.bib Makefile
