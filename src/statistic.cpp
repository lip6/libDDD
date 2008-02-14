/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg, Jean-Michel Couvreur      */
/*                             and Denis Poitrenaud                         */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU Lesser General Public License as       */
/*     published by the Free Software Foundation; either version 3 of the   */
/*     License, or (at your option) any later version.                      */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU LEsserGeneral Public License for more details.                   */
/*     						                            */
/* You should have received a copy of the GNU Lesser General Public License */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/

#include "statistic.hpp"
#include "process.hpp"
#include "MemoryManager.h"

static const char * const value_sep[] = {","," & "};
static const char * const line_sep[] = {"\n","\\\\ \n \\hline \n"};


Statistic::Statistic (const SDD & s, const std::string & name, OutputType sstyle): isPureDDD(false),style(sstyle),stat_name(name) {
  load (s);
}

Statistic::Statistic (const DDD & s, const std::string & name, OutputType sstyle): isPureDDD(true),style(sstyle),stat_name(name) {
  load (s);
}

void Statistic::load (const SDD & s) {
  nb_Stat=s.nbStates();
  std::pair<unsigned long int,unsigned long int> sizes = s.node_size(); 
  SDD_size = sizes.first;
  DDD_size = sizes.second;
  DDD_peak_size = DDD::peak();
  SDD_peak_size = SDD::peak();
  total_time = process::getTotalTime();
  memory = process::getResidentMemory();
  nbHom = GHom::statistics();
  nbShom = GShom::statistics();
  ddd_cache = DED::peak();
  sdd_cache = SDED::peak();
}

void Statistic::load (const DDD & s) {
  nb_Stat=s.nbStates();
  DDD_size = s.size();
  DDD_peak_size = DDD::peak();
  total_time = process::getTotalTime();
  memory = process::getResidentMemory();
  nbHom = GHom::statistics();
  ddd_cache = DED::peak();
}

void Statistic::print_header (std::ostream & os) {
  
  if (style == LATEX) {
    os << "\\documentclass[a4paper,10pt]{article} \n \n";
    os << "\\usepackage{rotating} \n \n" ;
    os << "\\begin{document} \n \n" ;
    os << "\\pagestyle{empty} \n \n" ;
    os << "\\begin{sidewaystable} \n";
    os << "# \\begin{footnotesize} \n ";
    if (! isPureDDD) os << "\\begin{tabular}{|c||c|c|c|c|c|c|c|c|c|c|c|} \n \\hline \n" ;
    else os << "\\begin{tabular}{|c||c|c|c|c|c|c|c|} \n \\hline \n" ;
  }

  os << "Model " << value_sep[style];
  os << "Nb. States "<< value_sep[style]; 
  os << "Time "<< value_sep[style];
  os << "Mem(kb) "<< value_sep[style];
  if (! isPureDDD) os << "fin. SDD "<< value_sep[style];
  os << "fin. DDD "<< value_sep[style];
  if (! isPureDDD) os << "peak SDD "<< value_sep[style];
  os << "peak DDD "<< value_sep[style];
  if (! isPureDDD) os << "SDD Hom "<< value_sep[style];
  if (! isPureDDD) os << "SDD cache "<< value_sep[style];
  os << "DDD Hom "<< value_sep[style];
  os << "DDD cache";
  os << line_sep[style] ;
}


void Statistic::print_line (std::ostream & os) {
  os << stat_name << value_sep[style] ;
  os << nb_Stat  << value_sep[style] ;
  os << total_time <<  value_sep[style] ;
  os << memory << value_sep[style];
  os << SDD_size << value_sep[style];
  os << DDD_size << value_sep[style];
  os << SDD_peak_size << value_sep[style];
  os << DDD_peak_size << value_sep[style];
  os << nbShom << value_sep[style];
  os << sdd_cache << value_sep[style];
  os << nbHom << value_sep[style] ;
  os << ddd_cache ;
  os << line_sep[style] ;
}



void Statistic::print_trailer (std::ostream & os, bool withLegend) {
  if (style == LATEX) {
    os << "\\hline \n\\end{tabular} \n  \n \\end{sidewaystable} \n\\end{document} \n";
  }
}


void Statistic::print_table (std::ostream & os) {
  print_header(os);
  print_line (os);
  print_trailer(os);
}
  
