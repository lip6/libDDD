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

std::string remove_bad_latex (const std::string & s) {
  std::string news;
  for (std::string::const_iterator it = s.begin() ; it != s.end(); ++it) {
    if (*it == '_')
      news.push_back('\\');
    news.push_back(*it);
  }
  return news;
}

Statistic::Statistic (const SDD & s, const std::string & name, OutputType sstyle): isPureDDD(false),style(sstyle),stat_name(remove_bad_latex (name)) {
  load (s);
}

Statistic::Statistic (const DDD & s, const std::string & name, OutputType sstyle): isPureDDD(true),style(sstyle),stat_name(remove_bad_latex (name)) {
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
  shom_cache = GShom::cache_size();
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
    os << "\\documentclass[a4paper,10pt]{article} \n \n"
       << "\\usepackage[usenames]{color}\n"
       << "\\usepackage{rotating,colortbl}\n\n"
       << "\\usepackage{lscape,longtable}\n\n"
       << "\\begin{document} \n \\pagestyle{empty}\n"
       << "\\begin{landscape} \n";
    if (! isPureDDD) os << "\\begin{longtable}{|c||c|c|c|c|c|c|c|c|c|c|c|c|} \n \\hline \n" ;
    else os << "\\begin{longtable}{|c||c|c|c|c|c|c|c|c|} \n \\hline \n" ;
  }

  os << "Model " << value_sep[style];
  os << "|S| "<< value_sep[style]; 
  os << "Time "<< value_sep[style];
  os << "Mem(kb) "<< value_sep[style];
  if (! isPureDDD) os << "fin. SDD "<< value_sep[style];
  os << "fin. DDD "<< value_sep[style];
  if (! isPureDDD) os << "peak SDD "<< value_sep[style];
  os << "peak DDD "<< value_sep[style];
  if (! isPureDDD) os << "SDD Hom "<< value_sep[style];
  if (! isPureDDD) os << "SDD cache "<< value_sep[style];
  os << "DDD Hom "<< value_sep[style];
  os << "DDD cache"<< value_sep[style];
  if (! isPureDDD) os << "SHom cache";
  os << line_sep[style] ;
}


void Statistic::print_line (std::ostream & os) {
  os << stat_name << value_sep[style] ;
  os << nb_Stat  << value_sep[style] ;
  os << total_time <<  value_sep[style] ;
  os << memory << value_sep[style];
  if (! isPureDDD) os << SDD_size << value_sep[style];
  os << DDD_size << value_sep[style];
  if (! isPureDDD) os << SDD_peak_size << value_sep[style];
  os << DDD_peak_size << value_sep[style];
  if (! isPureDDD) os << nbShom << value_sep[style];
  if (! isPureDDD) os << sdd_cache << value_sep[style];
  os << nbHom << value_sep[style] ;
  os << ddd_cache ;
  if (! isPureDDD) os << value_sep[style] << shom_cache;
  os << line_sep[style] ;
}



void Statistic::print_trailer (std::ostream & os, bool) {
  if (style == LATEX) {
    os << "\\hline \n\\end{longtable}\n\\end{landscape} \n\n \n\\end{document} \n";
  }
}


void Statistic::print_table (std::ostream & os) {
  print_header(os);
  print_line (os);
  print_trailer(os);
}

/// Print a legend to the table.
/// give interpretation for row headers
void Statistic::print_legend (std::ostream & os) {
  os << "Legend " << line_sep[style] << line_sep[style];
  os << "Each statistic line describes an SDD/DDD and gives some stats on memory/time usage at the point of the sample." << line_sep[style];
  os << "* Model : A title for this statistics line." << line_sep[style];
  os << "* |S| : Number of states (paths) in the SDD/DDD."<< line_sep[style]; 
  os << "* Time : process execution time when this stat was built."<< line_sep[style];
  os << "* Mem(kb) : process memory in kilobytes when this stat was built (may be unavailable for certain platforms)"<< line_sep[style];
  if (! isPureDDD) os << "* fin. SDD : Size in SDD nodes of the SDD."<< line_sep[style];
  os << "* fin. DDD : Size in DDD nodes of the DDD. "<< line_sep[style];
  if (! isPureDDD) os << "* peak SDD : peak number of nodes in SDD unique table."<< line_sep[style];
  os << "* peak DDD : peak number of DDD nodes in unique table."<< line_sep[style];
  if (! isPureDDD) os << "* SDD Hom : Number of SDD homomorphisms that exist in the unique table."<< line_sep[style];
  if (! isPureDDD) os << "* SDD cache : Number of elementary SDD node operations currently cached."<< line_sep[style];
  os << "* DDD Hom : Number of SDD homomorphisms that exist in the unique table."<< line_sep[style];
  os << "* DDD cache : Number of homomorphism applications to an SDD node currently cached."<< line_sep[style];
  if (! isPureDDD) os << "* SHom cache : Number of homomorphism applications to an SDD node currently cached." ;
  os << line_sep[style] ;  
}
  
