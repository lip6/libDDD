# Tools
TEXI2DVI =  texi2dvi
TEXI2PDF = $(TEXI2DVI) --pdf
DVIPS = dvips
TAR = tar
GZIP = gzip -9
DOT2PS = dot -Tps

# Sources
TEX_MASTERS = terms_doc.tex
TEX_INCLUDED =  intro.tex definition.tex encoding.tex terms_hom.tex  tool.tex   perfs.tex   conclu.tex  bibliography.bib 
terms_doc_FIGURES =
terms_doc_DOTS =
terms_doc_JPGS =
ALL_FIGURES = $(terms_doc_FIGURES)
ALL_JPGS = $(terms_doc_JPGS)
ALL_DOTS = $(terms_doc_DOTS)
EXTRA_DIST = terms_doc.html bibliography.bib

DVI_MASTERS = $(TEX_MASTERS:.tex=.dvi)
PS_MASTERS = $(TEX_MASTERS:.tex=.ps)
PDF_MASTERS = $(TEX_MASTERS:.tex=.pdf)
HTML_MASTERS = $(TEX_MASTERS:.tex=.html)
terms_doc_EPS_FIGURES = $(terms_doc_FIGURES:.fig=.eps)
terms_doc_PDF_FIGURES = $(terms_doc_FIGURES:.fig=.pdf)
terms_doc_EPS_DOTS = $(terms_doc_DOTS:.dot=.eps)
terms_doc_PDF_DOTS = $(terms_doc_DOTS:.dot=.pdf)
terms_doc_EPS_JPGS = $(terms_doc_JPGS:.jpg=.eps)
terms_doc_PDF_JPGS = $(terms_doc_JPGS)
ALL_EPS_FIGURES = $(ALL_FIGURES:.fig=.eps)
ALL_PDF_FIGURES = $(ALL_FIGURES:.fig=.pdf)
ALL_EPS_DOTS = $(ALL_DOTS:.fig=.eps)
ALL_PDF_DOTS = $(ALL_DOTS:.fig=.pdf)
ALL_EPS_JPGS = $(ALL_JPGS:.jpg=.eps)
ALL_PDF_JPGS = $(ALL_JPGS)

# What should be distributed?
DIST_DEPS = Makefile $(TEX_MASTERS) $(TEX_INCLUDED) $(DVI_MASTERS) \
            $(ALL_FIGURES) $(ALL_EPS_FIGURES) $(PDF_MASTERS) $(EXTRA_DIST) \
            $(ALL_JPGS) $(ALL_DOTS)
DIST = $(DIST_DEPS) terms_doc*.png

# Name for the tarball.
PACKAGE = terms_doc-04

#for makeindex
IDX_FILE  := $(shell ls $(DOCUMENT).idx 2>/dev/null)
ifneq ($(IDX_FILE),)
	IND_FILE := $(DOCUMENT).ind
endif


.POSIX:

# Phony targets
all: dvi ps # pdf html

dvi: $(DVI_MASTERS)
ps: $(PS_MASTERS)
pdf: $(PDF_MASTERS)
html: $(HTML_MASTERS)

.PHONY: all dvi ps pdf clean mostlyclean dist

mostlyclean:
	rm -f *.aux *.log *.toc *~ *.haux *.htoc *.bbl *.blg

clean: mostlyclean
	rm -f $(DVI_MASTERS) $(PS_MASTERS) $(PDF_MASTERS) $(HTML_MASTERS)
	rm -f $(ALL_EPS_FIGURES) $(ALL_PDF_FIGURES) *.gif *.png

dist: $(DIST_DEPS)
	rm -rf $(PACKAGE)
	mkdir $(PACKAGE)
	cp -rfp $(DIST) $(PACKAGE)
	$(TAR) chof - $(PACKAGE) | $(GZIP) -c >$(PACKAGE).tar.gz
	rm -rf $(PACKAGE)



# Inference rules
.SUFFIXES: .tex .dvi .pdf .ps .fig .eps .dot .jpg .html
#
# Generate a .ind file from the .idx file.
#

%.ind : %.idx
	makeindex $<

.tex.dvi:
	if ($(TEXI2DVI) --version) >/dev/null 2>&1; then \
	  $(TEXI2DVI) $<; \
	else \
	  latex $< && \
	  bibtex $* && \
	  latex $< && \
	  latex $<; \
	fi
.dvi.pdf:
	dvipdf $< $@
.dvi.ps:
	$(DVIPS) -o $@ $<
.fig.eps:
	fig2dev -L eps $< $@
.jpg.eps:
	convert $< $@
.fig.pdf:
	case $< in \
	  imp*) \
	    $(MAKE) ${@:.pdf=.eps}; \
	    epstopdf --outfile=$@ ${@:.pdf=.eps};; \
	  *) \
	    fig2dev -L pdf -p x $< $@;; \
	esac
.dot.eps:
	$(DOT2PS) -o $@ $<
.eps.pdf:
	epstopdf --outfile=$@ $<

.tex.html:
	hevea -fix -O article.hva png.hva $<
	imagen -png -mag 2000 $*

# Dependencies
terms_doc.dvi terms_doc.ps terms_doc.html: $(terms_doc_EPS_FIGURES) \
  $(terms_doc_EPS_DOTS) $(terms_doc_EPS_JPGS) $(TEX_INCLUDED)
terms_doc.pdf: $(terms_doc_PDF_FIGURES) $(terms_doc_PDF_DOTS) \
  $(terms_doc_PDF_JPGS) $(TEX_INCLUDED)
