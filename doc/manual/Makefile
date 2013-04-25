target := manual

.PHONY: clean all
%.pdf: %.tex $(wildcard *.sty *.tex)
	pdflatex $<
	pdflatex $<
all: ${target}.pdf
clean:
	@if test -f "${target}.aux"; then rm "${target}.aux"; fi
	@if test -f "${target}.log"; then rm "${target}.log"; fi
	@if test -f "${target}.out"; then rm "${target}.out"; fi
