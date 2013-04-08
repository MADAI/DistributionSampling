target := CovarianceFunctions

style_files := utfsymb.sty
.PHONY: clean all
temporaries := $(target).aux $(target).log $(target).out
all: $(target).pdf
clean:
	rm $(temporaries)
%.pdf: %.tex $(style_files)
	pdflatex $<
