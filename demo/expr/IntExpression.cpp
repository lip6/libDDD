#include "IntExpression.hpp"

#include <cmath>



class VarExpr : public IntExpression {
  const Variable & var;  

public :
  VarExpr (const Variable & vvar) : var(vvar){};
  IntExprType getType() const  { return VAR; }

  void print (std::ostream & os) const {
    os << var.getName();
  }

  const IntExpression & eval () const {
    return *this;
  }
};

class ConstExpr : public IntExpression {
  int val;

public :
  ConstExpr (int vval) : val(vval) {}
  IntExprType getType() const  { return CONST; }

  int getValue() const { return val; }
  const IntExpression & eval () const {
    return *this;
  }

  void print (std::ostream & os) const {
    os << val;
  }

};


class NaryIntExpr : public IntExpression {
protected :
  NaryParamType params ;
public :
  virtual const char * getOpString() const = 0;
  virtual ~NaryIntExpr () {
    for (NaryParamType::iterator it = params.begin() ; it != params.end() ; it++)
      delete *it;
  }

  const NaryParamType & getParams () const { return params; }
  
  NaryIntExpr (const NaryParamType & pparams):params(pparams) {};

  virtual int constEval (int i, int j) const = 0;
  const IntExpression & eval () const {
    NaryParamType p ;
    int constant=0;
    for (NaryParamType::const_iterator it = getParams().begin(); it != getParams().end() ; ++it ) {
      const IntExpression & e = (*it)->eval();
      if (e.getType() == CONST) {
	constant = constEval(constant, ((const ConstExpr &)e).getValue());
      } else {
	p.insert(&e);
      }
    }
    p.insert ( IntExpressionFactory::createConstant(constant) );
    if (p.size() == 1) 
      return ** p.begin();
    else 
      return *IntExpressionFactory::createNary(getType(),p);
  }


  void print (std::ostream & os) const {
    os << "( ";
    for (NaryParamType::const_iterator it = params.begin() ;  ; ) {
      (*it)->print(os);
      if (++it == params.end())
	break;
      else
	os << getOpString();
    }
    os << " )";
  }

};

class PlusExpr : public NaryIntExpr {

public :
  PlusExpr (const NaryParamType & pparams):NaryIntExpr(pparams) {};
  IntExprType getType() const  { return PLUS; }
  const char * getOpString() const { return " + ";}
  
  int constEval (int i, int j) const {
    return i+j;
  }
};

class MultExpr : public NaryIntExpr {

public :
  MultExpr (const NaryParamType  & pparams):NaryIntExpr(pparams) {};
  IntExprType getType() const  { return MULT; }
  const char * getOpString() const { return " * ";}

  int constEval (int i, int j) const {
    return i*j;
  }

};

class BinaryIntExpr : public IntExpression {
protected :
  const IntExpression *left;
  const IntExpression *right;
public :
  virtual const char * getOpString() const = 0;
  BinaryIntExpr (const IntExpression * lleft, const IntExpression * rright) : left (lleft),right(rright){};

  virtual int constEval (int i, int j) const = 0;

  const IntExpression & eval () const {
    const IntExpression & l = left->eval();
    const IntExpression & r = right->eval();

    if (l.getType() == CONST && r.getType() == CONST ) {
      return * IntExpressionFactory::createConstant( constEval( ((const ConstExpr &) l).getValue(),
								((const ConstExpr &) r).getValue()) );
    } else {
      return * IntExpressionFactory::createBinary( getType(), &l, &r );
    }
  }

  void print (std::ostream & os) const {
    os << "( ";
    left->print(os);
    os << getOpString();
    right->print(os);
    os << " )";
  }
};

class MinusExpr : public BinaryIntExpr {

public :
  MinusExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return MINUS; }
  const char * getOpString() const { return " - ";}
  int constEval (int i, int j) const {
    return i-j;
  }


};

class DivExpr : public BinaryIntExpr {

public :
  DivExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return DIV; }
  const char * getOpString() const { return " / ";}

  int constEval (int i, int j) const {
    return i/j;
  }


};

class ModExpr : public BinaryIntExpr {

public :
  ModExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return MOD; }
  const char * getOpString() const { return " % ";}

  int constEval (int i, int j) const {
    return i % j;
  }


};

class PowExpr : public BinaryIntExpr {

public :
  PowExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return POW; }
  const char * getOpString() const { return " ** ";}

  int constEval (int i, int j) const {
    return pow(i,j);
  }


};


namespace IntExpressionFactory {

  UniqueTable<IntExpression>  unique = UniqueTable<IntExpression>();


  const IntExpression * createNary (IntExprType type, NaryParamType params) {
    IntExpression * create;
    switch (type) {
    case PLUS :
      create = new PlusExpr (params);      
      break;
    case MULT :
      create = new MultExpr (params);      
      break;
    default :
      throw "Operator is not nary";
    }
//    return unique(create);
    return create;
  }

  const IntExpression * createBinary (IntExprType type, const IntExpression * l, const IntExpression * r) {
    IntExpression * create;
    switch (type) {
    case MINUS :
      create = new MinusExpr (l,r);      
      break;
    case DIV :
      create = new DivExpr (l,r);      
      break;
    case MOD :
      create = new ModExpr (l,r);      
      break;
    case POW :
      create = new PowExpr (l,r);      
      break;
    default :
      throw "Operator is not binary";
    }
//    return  unique(create);
    return create;
  }

  const IntExpression * createConstant (int v) {
    // return unique (new ConstExpr(v));
    return new ConstExpr(v);
  }

  const IntExpression * createVariable (const Variable & v) {
   // return unique (new VarExpr(v));
    return new VarExpr(v);
  }

}


// namespace IntExpression {
const IntExpression & operator+(const IntExpression & l,const IntExpression & r) {  
  NaryParamType p;
  if (l.getType() == PLUS) {
    const NaryParamType & p2 = ((const NaryIntExpr &)l).getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert(&l);
  }
  if (r.getType() == PLUS) {
    const NaryParamType & p2 = ((const NaryIntExpr &)r).getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert (&r);
  }
  return * IntExpressionFactory::createNary(PLUS, p);
} 

const IntExpression & operator*(const IntExpression & l,const IntExpression & r) {  
  NaryParamType p;
  if (l.getType() == MULT) {
    const NaryParamType & p2 = ((const NaryIntExpr &)l).getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert(&l);
  }
  if (r.getType() == MULT) {
    const NaryParamType & p2 = ((const NaryIntExpr &)r).getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert (&r);
  }
  return * IntExpressionFactory::createNary(MULT, p);
} 


// binary
const IntExpression & IntExpression::operator-(const IntExpression & e) const {
  return *IntExpressionFactory::createBinary(MINUS,this,&e);
}
const IntExpression & IntExpression::operator/(const IntExpression & e) const {
  return *IntExpressionFactory::createBinary(DIV,this,&e);
}
const IntExpression & IntExpression::operator%(const IntExpression & e) const {
  return *IntExpressionFactory::createBinary(MOD,this,&e);
}
const IntExpression & IntExpression::operator^(const IntExpression & e) const {
  return *IntExpressionFactory::createBinary(POW,this,&e);
}

std::ostream & operator << (std::ostream & os, const IntExpression & e) {
  e.print(os);
  return os;
}

// end class IntExpression}
