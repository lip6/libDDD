/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2004-2008 Yann Thierry-Mieg                            */
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

#ifndef __DOTEXPORTER__H__
#define __DOTEXPORTER__H__


#include "ddd/SDD.h"
#include "ddd/DDD.h"
using namespace std;


/* exports a SDD g into the file specified by path. 
 * If hierarchical is true, single dot file is produced with dashed lines to represent arc values.
 * The default produces one graph xxx.dot with SDD and another with DDD d3XXX.dot 
 */
int exportDot(const GSDD & g, const string & path="test",bool hierarchical=false, bool multiT=true);


/* Exports the full unique table of SDD into a dot file specified by path.
 * Highlights nodes that belong to the SDD provided.
 * NB : avoid if unique table size is too large... 
 **/
void exportUniqueTable ( const GSDD & d, const string & path="table" );

class dotExporter;

/** a more evolved API for highlighting parts of a graph **/
class dotHighlight {
  class dotExporter  * de;
public:
  virtual ~dotHighlight ();
  // prepares to export in file named "path"
  dotHighlight (const string & path);
  // Call this to empty the "known nodes" lists
  void initialize (const string & path);
  // Sets the vars to be aligned 
  void setVarAlignment (bool isAligned);
  // This adds an SDD node and all sons to a graph
  void addSDD (const GSDD & g);
  // This adds an SDD node and names it with the provided label
  void addSDD (const GSDD & g, const string &label);
  // This changes the color of a node and sons
  void setColor(const GSDD & g, const string & color);
  // This creates the actual file
  void exportDot();
};


#endif 
