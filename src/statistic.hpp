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
 
#include <iostream>
#include <string>
#include "SDD.h"
#include "DDD.h"



typedef enum {CSV, LATEX} OutputType;

class Statistic {
  /// number of final states
	long double nb_Stat;
  /// number of final DDD nodes
	long double DDD_size;
  /// number of final SDD nodes
	long double SDD_size;
  /// peak DDD unicity table size
	long double DDD_peak_size;
  /// peak DDD unicity table size
	long double SDD_peak_size;
  /// total time 
	double total_time;
  /// resident mem (kb)
	size_t memory;
  /// Hom final
	double nbHom;
  /// SHom final
	double nbShom;
  /// DDD cache
	double ddd_cache;
  /// SDD cache peak
	double sdd_cache;
  /// Mode : include SDD or not
	bool isPureDDD;
  /// Mode : outputType
	OutputType style;
  /// Name of this statistic
	std::string stat_name;

  /// Size of SHom cache
	size_t shom_cache;

  /// load from a SDD
	void load (const SDD & s);
  /// load from a DDD
	void load (const DDD & s);
public:

  /// Create a statistic for the current SDD s, style LATEX by default.
  /// The statistic is sampled and stored, this call triggers SDD mode (more columns).
  /// \param s the SDD used for final node and state count.
  /// \param name the row name (first column)
  /// \param style use LATEX for formatted latex table or CSV to import in excel or gnumeric.
  Statistic (const SDD & s, const std::string & name, OutputType style=LATEX);
  /// Create a statistic for the current DDD s, style LATEX by default.
  /// The statistic is sampled and stored, this call triggers DDD mode (less columns).
  /// \param s the DDD used for final node and state count.
  /// \param name the row name (first column)
  /// \param style use LATEX for formatted latex table or CSV to import in excel or gnumeric.
  Statistic (const DDD & s, const std::string & name, OutputType style=LATEX);

  /// Print column headers, using current style.
  /// For latex this also outputs a header (\begin document etc...).
  void print_header (std::ostream & os);

  /// Print trailer.
  /// The trailer includes a key to reading the columns. In latex mode it also closes the document.
  void print_trailer (std::ostream & os, bool withLegend=true);

  /// Print a line for current statistic. 
  /// To produce tables call print_header once then print_line for all your statistics.
  void print_line (std::ostream &os);

  /// Convenience print a table.
  /// Useful if you have a single stat to show.
  /// Calls print_header,print_line,print_trailer in this order.
  void print_table (std::ostream & os);

  /// Setter for style.
  void setStyle (OutputType style);

  /// Accessors.
  double getTime() const { return total_time; }
  long double getNbStates () const { return nb_Stat; }
  
};

