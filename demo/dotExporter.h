/****************************************************************************/
/*								            */
/* This file is part of the PNDDD, Petri Net Data Decision Diagram  package.*/
/*     						                            */
/*     Copyright (C) 2004 Denis Poitrenaud and Yann Thierry-Mieg            */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or    */
/*     (at your option) any later version.                                  */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU General Public License for more details.                         */
/*     						                            */
/*     You should have received a copy of the GNU General Public License    */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/
 


#ifndef __DOTEXPORTER__H__
#define __DOTEXPORTER__H__


#include "SDD.h"
#include "DDD.h"
using namespace std;


/* exports a net R 's state space g into the file specified by path */
int exportDot(const GSDD & g, const string & path="test",bool hierarchical=false);

class dotExporter;

/** a more evolved API for highlighting parts of a graph **/
class dotHighlight {
  static class dotExporter  * de;
public:
  // prepares to export in file named "path"
  dotHighlight (const string & path);
  // Call this to empty the "known nodes" lists
  static void initialize (const string & path);
  // This adds an SDD node and all sons to a graph
  static void addSDD (const GSDD & g);
  // This changes the color of a node and sons
  static void setColor(const GSDD & g, const string & color);
  // This creates the actual file
  static void exportDot();
  // Allows visualization (not for weaklings)
  void view();

};


#endif 
