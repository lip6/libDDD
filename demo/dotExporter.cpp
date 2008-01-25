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
 


#include "dotExporter.h"
#include <typeinfo>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ext/hash_set>
using namespace std;
using namespace __gnu_cxx;
    
class dotExporter{
protected:
  map<GSDD,string> name;
  map<GDDD,string> d3name;
  map<GDDD,string> entryd3Name;
  map<GDDD,int> entryd3Nb;
  map<GSDD,string> entryName;
  map<GSDD,int> entryNb;
  // vars nextid
  int nextid;
  // Philos nextid
  int nextPid;
  // modules nextid
  int nextMid;
  // Arcs nextid
  int nextAid;
  // Path to ouptput file
  string path;
  
  ostream * D3out;
  ostream * out;
  

  void collect( const SDD& g){    
    collect((const GSDD &)g);
  }
  void collect(const DDD& g){    
    collect((const GDDD &) g);
  }

public :
  virtual ~dotExporter() {};

   /* make a census and name all nodes in the sdd */
  virtual void collect(const GSDD& g){    
    if(name.find(g)==name.end()){
      stringstream tmp ;
      if (g== GSDD::one) {
	tmp << "one";
      } else if (g == GSDD::top) {
	tmp << "top";
      } else if (g == GSDD::top) {
	tmp << "null";
      } else {
	  tmp << "\"" << "mod_" <<  g.variable() << "_" << nextid++ << "\"";
//	  if (g.variable() == varP) {
	  *out << "     " << tmp.str() << "  [label=\"" <<  "mod " << g.variable()     ;
#ifdef OTF_GARBAGE
	  *out << "\",color=\""<< (g.isSon()?string("green"):string("red")) ;
#endif
	  *out << "\",shape=box];\n";
// 	  } else {
// 	    os << "     " << tmp.str() << "  [label=\"" ;
// 	    if (g.variable() == 1)  os << "2nd half";
// 	    else if (g.variable() == 0) os << "1st half";
// 	    else os << "CloseLoop";
// //	    os <<"\\n2^"<< g.variable()/2 
// 	      os <<  "\"];\n";
// 	  }
      }
       
      string myname =  tmp.str(); 
      name[g] =  myname; 
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
	if(typeid(*gi->first)==typeid(DDD) ) {
	  //(g.variable() == varP) {
	  // only one level of deepness supported at present, referenced value is always DDD type
	  DDD & arc = (DDD &) *gi->first;
	  collect(arc);
	  if (entryd3Name.find(arc) == entryd3Name.end()) {
	    stringstream tmp2 ;
	    tmp2 <<  "Mod" << "_" << nextPid++ ;
	    entryd3Name[arc] = tmp2.str() ;
	    entryd3Nb[arc] = 1;
	  } else {
	    entryd3Nb[arc]++;
	  }
	}  else {
	  // should not happen
	  SDD & arc = (SDD &) *gi->first;
	  collect(arc);
	}
      }

      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
	collect(gi->second);
      }
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
// 	stringstream arctmp ;
// 	arctmp << "Arc_" << nextAid++;
// 	os << "     " << arctmp.str() <<  "  [shape=point,label=\"\"];\n" ;
// 	os << "     " << myname << "->" <<  arctmp.str() <<  "  [arrowhead=none];\n" ;
// 	if ( name[gi->second] != "one" )
// 	  os << "     " << arctmp.str() << "->" << name[gi->second]   ;
// 	else {
// 	  os << "     " << "one_" << nextAid++ <<  " [shape=box,width=.3,height=.4,label=\"1\"];\n" ;
// 	  os << "     " << arctmp.str() << "->" << "one_" << nextAid-1  << " ;\n";  //name[gi->second]   ;
// 	}
// 	os << "     " << arctmp.str() << "->" ;

	
	if (typeid(*gi->first)==typeid(DDD)) //(g.variable() == varP)
	  *out <<  "     " << myname << "->" << name[gi->second] << "    [label=\""<< entryd3Name[(DDD &)*gi->first] << "\"];" << endl;
	     //"[style=dotted,minlen=2,constraint=false];\n" ;//constraint=false];\n" ;// minlen=2];\n" ;
	else
	  *out <<  "     " << myname << "->" << name[gi->second] << "    [label=\""<< ((SDD &)*gi->first).nbStates() << "\"];" << endl; 
	// "  [style=dotted,minlen=2];\n" ;//constraint=false];\n" ;minlen=2];\n" ;//
// 	if (g.variable() < 2)
// 	  os << entryd3Name[(DDD &)*gi->first] << "  [style=dotted];\n" ;
// 	else
// 	  os << entryName[(SDD &)*gi->first] << "  [style=dotted];\n" ;
      }
    }
  }

  void collect( const GDDD& g) {
    if(d3name.find(g)==d3name.end()){
      stringstream tmp ;
      if (g== GDDD::one) {
	tmp << "one";
      } else if (g == GDDD::top) {
	tmp << "top";
      } else if (g == GDDD::null) {
	tmp << "null";
      } else
	tmp << GDDD::getvarName(g.variable()) << "_" << nextid++;
      d3name[g] = tmp.str();
      if (g == GDDD::top||g == GDDD::null) { 
	*D3out << "     " << tmp.str() << ";\n";
	return;
      } else if (g== GDDD::one ) /* g== GDDD::one  -> suppress output we use square boxes*/
	return;
      (*D3out) << "     " << tmp.str() << "  [label=\""<< GDDD::getvarName(g.variable()) << "\"];"<<endl;
      for(GDDD::const_iterator gi=g.begin();gi!=g.end();gi++) 
	collect(gi->second);
      for(GDDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
// 	if ( d3name[gi->second] != "one" )
// 	  (*D3out) << "     " << tmp.str() << "->" << d3name[gi->second]  <<"    [label=\""<< gi->first << "\",shape=box];" <<endl; 
// 	else {
// 	  (*D3out) << "     " << "one_" << nextAid++ <<  " [shape=box,width=.3,height=.4,label=\"1\"];\n" ;
// 	  (*D3out) << "     " << tmp.str() << "->" << "one_" << nextAid-1  <<"    [label=\""<< gi->first << "\"];" <<endl; 
// 	}
	// use  next line for only one terminal node
		(*D3out) << "     " << tmp.str() << "->" << d3name[gi->second] << "    [label=\""<< gi->first << "\"];" <<endl;
      }
    }
  }

  dotExporter(const string &s="test"):path(s) {};

  void printColor (const GSDD & g, const string & color,hash_set<GSDD> &visited) {
    if (visited.find(g) == visited.end() ) {
      *out << "     " << name.find(g)->second << " [color=\""<< color << "\"];"<<endl;
      visited.insert(g);
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
	printColor(gi->second,color,visited);
      }
    }
  }

  void setColor(const GSDD & g, const string & color) {
    hash_set<GSDD> visited;
    collect(g);
    printColor(g,color,visited);
  }

  void init () {
    nextid=0;
    nextMid=0;
    nextPid=0;
    nextAid=0;

    name.clear();
    d3name.clear();
    entryNb.clear();
    entryName.clear();
    entryd3Nb.clear();
    entryd3Name.clear();


    out = new ofstream ( string(path+".dot").c_str() );
    D3out = new ofstream ( string("d3"+path+".dot").c_str());
    *out << "digraph "<<path<<" {\n    size=\"20,20\";\n"; 
    *D3out << "digraph  d3"<<path<<" {\n    size=\"20,20\";\n"; 

  }

  int operator()(const GSDD& g){
    init();
    collect(g);
    finish();
    return 1;
  }

  void finish () {
    for (map<GDDD,string>::iterator it = entryd3Name.begin() ; it != entryd3Name.end() ; it++ ) {
      *D3out <<  "     " << it->second   << "  [shape=invhouse];\n";
      *D3out <<  "     " << it->second   << "  [label=\"" << it->second << "\"];\n";
      *D3out <<  "     " << it->second   <<  "->" <<  d3name[it->first] << "    [label=\""<< entryd3Nb[it->first] << "\"];" <<endl;
    }
    for (map<GSDD,string>::iterator it = entryName.begin() ; it != entryName.end() ; it++ ) {
      *out <<  "     " << it->second   << "  [shape=invhouse];\n";
      *out <<  "     " << it->second   << "  [label=\"" << it->second << "\"];\n";
      *out <<  "     " << it->second   <<  "->" <<  name[it->first] << "    [label=\""<< entryNb[it->first] << "\"];" <<endl;
    }


    *out << "}\n" <<endl;
     *D3out << "}\n" <<endl;
  
  }
};


class hDotExporter: public dotExporter {
public :
  hDotExporter(const string &s="test"):dotExporter(s) {};

  virtual ~hDotExporter() {};
  
   /* make a census and name all nodes in the sdd */
  void collect(const GSDD& g){    
    if(name.find(g)==name.end()){
      stringstream tmp ;
      if (g== GSDD::one) {
	tmp << "one";
      } else if (g == GSDD::top) {
	tmp << "top";
      } else if (g == GSDD::top) {
	tmp << "null";
      } else {
	  tmp << "\"" << "mod_" ;
	  if ( g.variable() > 0 )  tmp <<  g.variable() ;
	  else tmp << "_to" << - g.variable() ;
	  tmp << "_" << nextid++ << "\"";
//	  if (g.variable() == varP) {
	    *out << "     " << tmp.str() << "  [label=\"" ;
	    if ( g.variable() > 0 )  *out  <<  "mod " << g.variable()    <<"\"];\n";
	    else *out  << "(mod" << - g.variable()  <<")\"];\n";
// 	  } else {
// 	    *out << "     " << tmp.str() << "  [label=\"" ;
// 	    if (g.variable() == 1)  *out << "2nd half";
// 	    else if (g.variable() == 0) *out << "1st half";
// 	    else *out << "CloseLoop";
// //	    *out <<"\\n2^"<< g.variable()/2 
// 	      *out <<  "\"];\n";
// 	  }
      }
       
      string myname =  tmp.str(); 
      name[g] =  myname; 
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
	if (g.variable() >= 0) {
	  // in DDD level description

	  DDD & arc = (DDD &) *gi->first;
	  dotExporter::collect(arc);
	  if (entryd3Name.find(arc) == entryd3Name.end()) {
	    stringstream tmp2 ;
	    tmp2 <<  "Mod" << "_" << nextPid++ ;
	    entryd3Name[arc] = tmp2.str() ;
	    entryd3Nb[arc] = 1;
	  } else {
	    entryd3Nb[arc]++;
	  }
	}  else {
	  // recurse on ref'ed value
	  SDD & arc = (SDD &) *gi->first;
	  collect(arc);
// 	  if (entryName.find(arc) == entryName.end()) {
// 	    stringstream tmp2 ;
// 	    tmp2 <<  "mod"  << "_" << nextMid++ ;
// 	    entryName[arc] = tmp2.str() ;
// 	    os << "     " << tmp2.str() << ";" << endl ;
// 	    entryNb[arc] = 1;
// 	  } else {
// 	    entryNb[arc]++;
// 	  }
	}
      }

      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
	collect(gi->second);
      }
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) {
	if (g.variable() >= 0 )
	  *out <<  "     " << myname << "->" << name[gi->second] << "    [label=\""<< entryd3Name[(DDD &)*gi->first] << "\"];" << endl;
	//"[style=dotted,minlen=2,constraint=false];\n" ;//constraint=false];\n" ;// minlen=2];\n" ;
	else {
	  stringstream arctmp ;
	  arctmp << "Arc_" << nextAid++;
	  *out << "     " << arctmp.str() <<  "  [shape=point,label=\"\"];\n" ;
	  *out << "     " << myname << "->" <<  arctmp.str() <<  "  [arrowhead=none];\n" ;
	  if ( name[gi->second] != "one" )
	    *out << "     " << arctmp.str() << "->" << name[gi->second]   ;
	  else {
	    *out << "     " << "one_" << nextAid++ <<  " [shape=box,width=.3,height=.4,label=\"1\"];\n" ;
	    *out << "     " << arctmp.str() << "->" << "one_" << nextAid-1  << " ;\n";  //name[gi->second]   ;
	  }
	  *out << "     " << arctmp.str() << "->" ;
	  *out << name[(SDD &)*gi->first] << "  [style=dotted,minlen=2];\n" ;

//	  *out <<  "     " << name[gi->second] << "    [label=\""<< entryName[(SDD &)*gi->first] << "\"];" << endl;
//	  *out << name[(SDD &)*gi->first] << "  [style=dotted,minlen=2];\n" ;//constraint=false];\n" ;minlen=2];\n" ;//
// 	if (g.variable() < 2)
// 	  *out << entryd3Name[(DDD &)*gi->first] << "  [style=dotted];\n" ;
// 	else
// 	  *out << entryName[(SDD &)*gi->first] << "  [style=dotted];\n" ;
	}
      }
    }
  }


};


int exportDot(const GSDD & g,const string &path,bool hierarchical) {
  if (!hierarchical) {
    dotExporter dotout(path);
    return dotout(g);
  }  else {
    hDotExporter dotout(path);
    return dotout(g); 
  }
}

/************** CLASS dotHighlight ************/
dotExporter  * dotHighlight::de = NULL;

// prepares to export in file named "path"
dotHighlight::dotHighlight (const string & path){ de = new dotExporter(path);};


// Call this to empty the "known nodes" lists
void dotHighlight::initialize (const string & path) { de->init() ; }

// This adds an SDD node and all  sons to a graph
void dotHighlight::addSDD (const GSDD & g) {
  de->collect(g);
}


// This changes the color of a node and sons
void dotHighlight::setColor(const GSDD & g, const string & color) {
  de->setColor(g, color);
}

// This creates the actual file
void dotHighlight::exportDot() {
  de->finish();
}

// Allows visualization (not for weaklings)
void dotHighlight::view() {
  cerr<< "not yet dotHighlight::view, implement me if you need me!! "<<endl;

}
