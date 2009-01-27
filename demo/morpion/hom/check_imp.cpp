//
// C++ Implementation: home_check_imp
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/foreach.hpp>
#include <iostream>

#include "hom/check_imp.hpp"
#include "hom/check_impossible.hpp"

// pretty print of a cell state
const char *
  printCell (int cellState) {
  const char * PA_str = "O";
  const char * PB_str = "X";
  const char * E_str =  ".";
  switch (cellState) {
    case 0 :
      return PA_str;
    case 1 :
      return PB_str;
    default :
      return E_str;
  }
}

// pretty print of a game state stored in an array
void
    printState (const array_type& cc) {
  for (int lig = 0; lig < 3 ; ++lig) {
    for (int col = 0; col < 3 ; ++col) {
      std::clog << printCell(cc[lig][col]);
    }
    std::clog << std::endl;
  }
}


bool
    check_line(
      array_type cc
    , int line)
{
  bool res = true;
  int ref = cc[line][0];
  
  for(size_t j=1;j<COLUMN;++j)
  {
    res = res and (cc[line][j] != EMPTY)
              and (cc[line][j] == ref);
  }

  return res;
}

bool
    check_column(
     array_type cc
    ,int column)
{
  bool res = true;
  int ref = cc[0][column];
  
  for(size_t j=1;j<LINE;++j)
  {
    res = res and (cc[j][column] != EMPTY)
        and (cc[j][column] == ref);
  }

  return res;
}

bool
    check_cross(array_type cc)
{
  bool res_1 = true;
  bool res_2 = true;
  int ref = cc[0][0];

  // First Cross
  for(size_t j=1;j<LINE;++j)
  {
    res_1 = res_1 and (cc[j][j] != EMPTY)
        and (cc[j][j] == ref);
    res_2 = res_2 and (cc[j][LINE - 1 - j] != EMPTY)
        and (cc[j][LINE - 1 - j] == ref);
  }

  return res_1 or res_2;
}

/**
 * An inductive homomorphism _CheckImpossible used only for check if there is some impossible configuration.
 * Validation Class
 */
class _checkImpossible
  : public StrongHom
{

  /**
   * List of variable
   */
  array_type cc;
  int first_index;
  int node_to_ignore;
  std::vector<ref_validate_base_type> tests;

protected:
 /**
   * Constructor with initialisation
   */
  _checkImpossible (const array_type& c,int fi,int nti,const ref_tests& tts)
  : cc(c),first_index(fi),node_to_ignore(nti),tests(tts)
    {
    }
    
public:

  /**
  * Constructor with initialisation
  */
  _checkImpossible (int fi,int nti,const ref_tests& tts)
  : cc(boost::extents[LINE][COLUMN]) , first_index(fi),node_to_ignore(nti),tests(tts)
    {
      for(size_t i = 0; i< LINE ; ++i)
      {
        for(size_t j=0; j<COLUMN ; ++j)
        {
          cc[i][j]=-1;
        }
      }
    }

    /**
     * Skip all variable except Node 0
     */
    bool
        skip_variable(int vr) const
    {
      return (vr == node_to_ignore);
    }
    
    
  
    /**
     * PHI [1] : We have all state of the system and we can check if there are any impossible state
     */
    GDDD
        phiOne() const
    {


    for(ref_tests::const_iterator i = tests.begin() ; i != tests.end() ; ++i)
    {
      (**i)(cc);
    }
    /*
    BOOST_FOREACH(ref_validate_base_type e, tests)
    {
      (*e)(cc);
    }
    */
      
#if 0
      /* Check if the grid is full */
      bool is_full = true;
      for(size_t i=0;i<LINE and is_full;++i)
      {
        for(size_t j=0;j<COLUMN and is_full;++j)
        {
          if(cc[i][j]==-1)
          {
            is_full =false;
          }
        }
      }
      if(is_full)
      { /* Check if there is some winner on first line */
        std::clog << "Find one configuration completed :" << std::endl;
        printState(cc);
      }
      else
      {
        ; // There is no full grid
      }


      
      if(check_line(cc,0) and check_column(cc,0) )
      {
        /* Check if there is some winner on first line
         *  XXX
         *  XOO
         *  XOO
         */
        std::clog << "One configuration Winner OK on line 0 and Column 0 :" << std::endl;
        printState(cc);
      }

      

      // Check the impossible line
      if(check_line(cc,0) and ( check_line(cc,1) or check_line(cc,2) ) )
      {
        std::clog << "Impossible configuration detected 1 :" << std::endl;
        printState(cc);
      }
      if(check_line(cc,1) and ( check_line(cc,0) or check_line(cc,2) ) )
      {
        std::clog << "Impossible configuration detected 2 :" << std::endl;
        printState(cc);
      }
      if(check_line(cc,2) and ( check_line(cc,0) or check_line(cc,1) ) )
      {
        std::clog << "Impossible configuration detected 3 :" << std::endl;
        printState(cc);
      }
      
      // Check the impossible Column
      if(check_column(cc,0) && (check_column(cc,1) || check_column(cc,2)))
      {
        std::clog << "Impossible configuration detected 4 :" << std::endl;
        printState(cc);
      }
      if(check_column(cc,1) && (check_column(cc,0) || check_column(cc,2)))
      {
        std::clog << "Impossible configuration detected 5 :" << std::endl;
        printState(cc);
      }
      if(check_column(cc,2) && (check_column(cc,0) || check_column(cc,1)))
      {
        std::clog << "Impossible configuration detected 6 :" << std::endl;
        printState(cc);
      }
#endif
      return GDDD::one;
    }


    /**
     * PHI [vr,vl] : called on each arc labeled with "vl" when the homomorphism is applied to a node
     * of variable "vr" such that vr is NOT skipped.
     * When the hom is applied to a node this function is called for each arc, and the result
     * (a homomorphism) is applied to the successor node pointed by the arc.
     *  vr ==> Variable of current node
     *  vl ==> arc label (an integer) of current arc
     */
    GHom
        phi(int vr, int vl) const {
      
      /* Create new Homo with the current configuration */
      int i=(vr-first_index)/LINE; // Conversion sur la ligne
      int j=(vr-first_index)%COLUMN; // Conversion sur la colonne
  
      //std::clog << "Node [" << vr << "," << vl << "] Conversion on Grid [" << i << "," << j <<"] = " << vl << std::endl;
      if(cc[i][j]!=vl)
      {
        array_type tab(cc);
        tab[i][j] = vl;
        return GHom (vr,vl,_checkImpossible(tab,first_index,node_to_ignore,tests));
        //return GHom (vr,vl,_checkImpossible(tab));
      }
      else
      {
        return GHom (vr,vl,GHom(this));
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const {
      std::size_t seed = 3833;
      for(size_t i = 0; i< LINE ; ++i)
      {
        for(size_t j=0; j<COLUMN ; ++j)
        {
          boost::hash_combine(seed, cc[i][j]);
        }
      }
      
      return seed ;
    }
  
    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print (std::ostream & os) const {
      os << "Configuration Game : [";
      for(size_t i = 0; i< LINE ; ++i)
      {
        for(size_t j=0; j<COLUMN ; ++j)
        {
          os << " " << cc[i][j];
        }
        os << "\n";
      }
    }


    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const {
      const _checkImpossible& ps = dynamic_cast<const _checkImpossible&>(s);
      for(size_t i = 0; i< LINE ; ++i)
      {
        for(size_t j=0; j<COLUMN ; ++j)
        {
          if(cc[i][j] != ps.cc[i][j])
          {
            return false;
          }
        }
      }

      
      return node_to_ignore == ps.node_to_ignore;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom * clone () const {  return new _checkImpossible(*this); }
};


/**
 * Factory function of checkImpossible instance
 */
Hom
    checkImpossible (const ref_tests& tests)
{
  // Valeur par défaut sur 1 en first index et 0 node_to_ignore
  return GHom(_checkImpossible(1,0,tests));
}

Hom
    checkImpossible (int fi,int nti,const ref_tests& tests)
{
  return GHom(_checkImpossible(fi,nti,tests));
}