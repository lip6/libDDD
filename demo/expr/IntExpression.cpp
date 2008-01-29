#include "IntExpression.hpp"





class NaryIntExpr : public IntExpression {
protected :
  NaryParamType params ;
public :
  virtual const char * getOpString() const = 0;
  
  NaryIntExpr (const NaryParamType & pparams):params(pparams) {};

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

};

class MultExpr : public NaryIntExpr {

public :
  MultExpr (const NaryParamType  & pparams):NaryIntExpr(pparams) {};
  IntExprType getType() const  { return MULT; }
  const char * getOpString() const { return " * ";}

};

class BinaryIntExpr : public IntExpression {
protected :
  const IntExpression *left;
  const IntExpression *right;
public :
  virtual const char * getOpString() const = 0;
  BinaryIntExpr (const IntExpression * lleft, const IntExpression * rright) : left (lleft),right(rright){};
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


};

class DivExpr : public BinaryIntExpr {

public :
  DivExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return DIV; }
  const char * getOpString() const { return " / ";}


};

class ModExpr : public BinaryIntExpr {

public :
  ModExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return MOD; }
  const char * getOpString() const { return " % ";}


};

class PowExpr : public BinaryIntExpr {

public :
  PowExpr (const IntExpression * left, const IntExpression * right) : BinaryIntExpr(left,right) {};
  IntExprType getType() const  { return POW; }
  const char * getOpString() const { return " ** ";}


};

class VarExpr : public IntExpression {
  const Variable & var;  

public :
  VarExpr (const Variable & vvar) : var(vvar){};
  IntExprType getType() const  { return VAR; }

  void print (std::ostream & os) const {
    os << var.getName();
  }

};

class ConstExpr : public IntExpression {
  int val;

public :
  ConstExpr (int vval) : val(vval) {}
  IntExprType getType() const  { return CONST; }

  void print (std::ostream & os) const {
    os << val;
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
  const IntExpression & IntExpression::operator+(const IntExpression & e) const {
    NaryParamType p;
    p.insert(this);
    p.insert(&e);
    return * IntExpressionFactory::createNary(PLUS, p);
  } 
  const IntExpression & IntExpression::operator*(const IntExpression & e) const {
    NaryParamType p;
    p.insert(this);
    p.insert(&e);
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
