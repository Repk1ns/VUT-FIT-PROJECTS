all:
	latex proj3.tex
	latex proj3.tex
	dvips -t a4 proj3.dvi
	ps2pdf -sPAPERSIZE=a4 proj3.ps

clean:
	-rm -f proj3.aux proj3.dvi proj3.log proj3.ps

zip:
	zip xmimoc01-fit.zip proj3.tex etiopan.eps oniisan.eps oniisan2.eps Makefile
