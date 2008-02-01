#include "BoolExpression.hpp"



class NaryBoolExpr : public BoolExpression {
protected :
  NaryBoolParamType params ;
public :
  virtual const char * getOpString() const = 0;
  virtual ~NaryBoolExpr () {
    for (NaryBoolParamType::iterator it = params.begin() ; it != params.end() ; it++)
      delete *it;
  }

  NaryBoolExpr (const NaryBoolParamType & pparams):params(pparams) {};

  void print (std::ostream & os) const {
    os << "( ";
    for (NaryBoolParamType::const_iterator it = params.begin() ;  ; ) {
      (*it)->print(os);
      if (++it == params.end())
	break;
      else
	os << getOpString();
    }
    os << " )";
  }

};

class OrExpr : public NaryBoolExpr {

public :
  OrExpr (const NaryBoolParamType & pparams):NaryBoolExpr(pparams) {};
  BoolExprType getType() const  { return OR; }
  const char * getOpString() const { return " || ";}

};

class AndExpr : public NaryBoolExpr {

public :
  AndExpr (const NaryBoolParamType  & pparams):NaryBoolExpr(pparams) {};
  BoolExprType getType() const  { return AND; }
  const char * getOpString() const { return " && ";}

};

class BinaryBoolComp : public BoolExpression {
protected :
  const IntExpression *left;
  const IntExpression *right;
public :
  virtual const char * getOpString() const = 0;
  BinaryBoolComp (const IntExpression * lleft, const IntExpression * rright) : left (lleft),right(rright){};
  void print (std::ostream & os) const {
    os << "( ";
    left->print(os);
    os << getOpString();
    right->print(os);
    os << " )";
  }
};

class BoolEq : public BinaryBoolComp {

public :
  BoolEq (const IntExpression * left, const IntExpression * right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return EQ; }
  const char * getOpString() const { return " == ";}
};

class BoolNeq : public BinaryBoolComp {

public :
  BoolNeq (const IntExpression * left, const IntExpression * right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return NEQ; }
  const char * getOpString() const { return " != ";}
};

class BoolGeq : public BinaryBoolComp {

public :
  BoolGeq (const IntExpression * left, const IntExpression * right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return GEQ; }
  const char * getOpString() const { return " >= ";}
};


class BoolLeq : public BinaryBoolComp {

public :
  BoolLeq (const IntExpression * left, const IntExpression * right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LEQ; }
  const char * getOpString() const { return " <= ";}
};

class BoolLt : public BinaryBoolComp {

public :
  BoolLt (const IntExpression * left, const IntExpression * right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}
};

class BoolGt : public BinaryBoolComp {

public :
  BoolGt (const IntExpression * left, const IntExpression * right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}
};


class NotExpr : public BoolExpression {
  const BoolExpression *  exp;  

public :
  NotExpr (const BoolExpression * eexp) : exp(eexp){};
  BoolExprType getType() const  { return NOT; }

  void print (std::ostream & os) const {
    os << " ! (" ;
    exp->print(os);
    os << " ) ";
  }

};


class VarBoolExpr : public BoolExpression {
  const Variable & var;  

public :
  VarBoolExpr (const Variable & vvar) : var(vvar){};
  BoolExprType getType() const  { return BOOLVAR; }

  void print (std::ostream & os) const {
    os << var.getName();
  }

};

class BoolConstExpr : public BoolExpression {
  bool val;

public :
  BoolConstExpr (bool vval) : val(vval) {}
  BoolExprType getType() const  { return BOOLCONST; }

  void print (std::ostream & os) const {
    os << val;
  }

};


namespace BoolExpressionFactory {

  UniqueTable<BoolExpression>  unique = UniqueTable<BoolExpression>();


  const BoolExpression * createNary (BoolExprType type, NaryBoolParamType params) {
    BoolExpression * create;
    switch (type) {
    case OR :
      create = new OrExpr (params);      
      break;
    case AND :
      create = new AndExpr (params);      
      break;
    default :
      throw "Operator is not nary";
    }
//    return unique(create);
    return create;
  }

  const BoolExpression * createNot  (const BoolExpression * e) {
    BoolExpression * create = new NotExpr(e);
    return create;
  }

  const BoolExpression * createConstant (bool b) {
    return new BoolConstExpr(b);
  }

  const BoolExpression * createVariable (const Variable & v) {
    return new VarBoolExpr(v);
  }

  // a comparison (==,!=,<,>,<=,>=) between two integer expressions
  const BoolExpression * createComparison (BoolExprType type, const IntExpression * l, const IntExpression * r) {
    BoolExpression * create;
    switch (type) {
    case EQ :
      create = new BoolEq (l,r);
      break;
    case NEQ :
      create = new BoolNeq (l,r);
      break;
    case GEQ :
      create = new BoolGeq (l,r);
      break;
    case LEQ :
      create = new BoolLeq (l,r);
      break;
    case LT :
      create = new BoolLt (l,r);
      break;
    case GT :
      create = new BoolGt (l,r);
      break;      
    default :
      throw "not a binary comparison operator !";
    }
    return create;
  }
    
}


// nary constructions
const BoolExpression & BoolExpression::operator&&(const BoolExpression & e) const {
  NaryBoolParamType p;
  p.insert(this);
  p.insert(&e);
  return * BoolExpressionFactory::createNary(AND, p);
} 

const BoolExpression & BoolExpression::operator||(const BoolExpression & e) const {
  NaryBoolParamType p;
  p.insert(this);
  p.insert(&e);
  return * BoolExpressionFactory::createNary(OR, p);
} 

const BoolExpression & BoolExpression::operator!() const {
  return * BoolExpressionFactory::createNot(this);
} 


// binary comparisons
const BoolExpression & operator==(const IntExpression & l, const IntExpression & r) {
  return * BoolExpressionFactory::createComparison (EQ,&l,&r);
}
const BoolExpression & operator!=(const IntExpression & l, const IntExpression & r) {
  return * BoolExpressionFactory::createComparison (NEQ,&l,&r);
}
const BoolExpression & operator<=(const IntExpression & l, const IntExpression & r) {
  return * BoolExpressionFactory::createComparison (LEQ,&l,&r);
}
const BoolExpression & operator>=(const IntExpression & l, const IntExpression & r) {
  return * BoolExpressionFactory::createComparison (GEQ,&l,&r);
}
const BoolExpression & operator<(const IntExpression & l, const IntExpression & r) {
  return * BoolExpressionFactory::createComparison (LT,&l,&r);
}
const BoolExpression & operator>(const IntExpression & l, const IntExpression & r) {
  return * BoolExpressionFactory::createComparison (GT,&l,&r);
}

std::ostream & operator << (std::ostream & os, const BoolExpression & e) {
  e.print(os);
  return os;
}

// end class IntExpression}
