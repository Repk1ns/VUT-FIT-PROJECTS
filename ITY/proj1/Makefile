all:
	latex proj1.tex
	dvips -t a4 proj1.dvi
	ps2pdf proj1.ps

clean:
	-rm -f proj1.aux proj1.dvi proj1.log proj1.ps

zip:
	zip xmimoc01-fit.zip proj1.tex Makefile
