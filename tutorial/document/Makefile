.PHONY: clean all

all: StatisticsTutorial.pdf
clean:
	rm StatisticsTutorial.aux StatisticsTutorial.log

%.pdf: %.tex utfsymb.sty
	pdflatex $<

