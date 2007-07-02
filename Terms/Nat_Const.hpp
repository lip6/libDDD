#ifndef __NAT_CONST_HH__
#define __NAT_CONST_HH__

#include "IntDataSet.h"
#include "SDD.h"

// SDD Variables 
// Natural 
#define NAT 0
// Naturals expect an IntDataSet value representing one of the symbolic operators  if < 0 :
#define PLUS -1
#define SUC -2
// else a constant 0, 1 , 2 ...

// left and right operands of binary operators (e.g. plus) : expect a SDD value
#define LEFT 1
#define RIGHT 2

// a dataset for "+"
extern const IntDataSet natPlus ;
// a dataset for constant 0
extern const IntDataSet natZero ;
// a dataset for constant 1
extern const IntDataSet natOne ;

// a SDD dataset for constant 0
extern const SDD SDDnatZero;
// a SDD dataset for constant 1
extern const SDD SDDnatOne;


#endif // __NAT_CONST_HH__
