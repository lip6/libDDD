## Copyright (C) 2003, 2004, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
## département Systèmes Répartis Coopératifs (SRC), Université Pierre
## et Marie Curie.
##
## This file is part of libDDD, a library for Data Decision Diagram manipulation.
##
## It is based on a file from the Spot distribution by Alexendre Duret-Lutz
## adapted to libddd on 15/03/2005   
##
## Spot is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## Spot is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
## License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Spot; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

DOXYGEN = doxygen

.PHONY: doc fast-doc

all-local: $(srcdir)/stamp

doc:
	rm -f $(srcdir)/stamp
	$(MAKE) $(srcdir)/stamp

fast-doc:
	$(MAKE) Doxyfile
	$(DOXYGEN)
	touch $(srcdir)/stamp

$(srcdir)/stamp: $(srcdir)/Doxyfile.in $(top_srcdir)/configure.ac
	$(MAKE) Doxyfile
	rm -rf libddd.html libddd.latex
	$(DOXYGEN)
	touch $@

$(srcdir)/libddd.html $(srcdir)/libddd.tag: $(srcdir)/stamp
libddd.latex: $(srcdir)/stamp

$(srcdir)/libdddref.pdf: $(srcdir)/stamp
	cd libddd.latex \
	  && texi2dvi --pdf refman.tex \
	  && cd .. && mv -f libddd.latex/refman.pdf $@

# libddd.tag is useful to third-party packages that must link to the
# Libddd documentation.
dist_pkgdata_DATA = $(srcdir)/libddd.tag

EXTRA_DIST = \
  footer.html \
  mainpage.dox \
  $(srcdir)/stamp \
  $(srcdir)/libddd.html \
  $(srcdir)/libdddref.pdf \
  manuel.ps
