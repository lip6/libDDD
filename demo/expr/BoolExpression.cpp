#include "BoolExpression.hpp"
#include "hashfunc.hh"
#include <cassert>




// unique storage class
class _BoolExpression {
  // count references
  mutable size_t refCount;
  // access to refcount for garbage purpose
  friend class BoolExpressionFactory;
 public :
  // add a ref
  size_t ref () const { 
    return ++refCount;
  }
  // dereference
  size_t deref () const { 
    assert(refCount >0);
    return --refCount;
  }
  // default constructor
  _BoolExpression (): refCount(0) {}
  // reclaim memory
  virtual ~_BoolExpression () { assert (refCount==0); };
  
  ///////// Interface functions
  // for hash storage
  virtual size_t hash () const = 0 ;
  virtual bool operator==(const _BoolExpression & e) const = 0;

  // to avoid excessive typeid RTTI calls.
  virtual BoolExprType getType() const =0;

  // Because friend is not transitive. Offer access to BoolExpression concrete to subclasses.
  static const _BoolExpression * getConcrete ( const BoolExpression & e) { return e.concrete ;}

  // pretty print
  virtual void print (std::ostream & os) const =0 ;

  // Evaluate an expression.
  virtual BoolExpression eval() const = 0;

  virtual BoolExpression setAssertion (const Assertion & a) const = 0;

  virtual bool isSupport (const Variable & v) const = 0;
};


namespace __gnu_cxx {
  template<>
  struct hash<_BoolExpression*> {
    size_t operator()(_BoolExpression * g) const{
      return g->hash();
    }
  };
}

namespace std {
  template<>
  struct equal_to<_BoolExpression*> {
    bool operator()(_BoolExpression *g1,_BoolExpression *g2) const{
      return (typeid(*g1) == typeid(*g2) && *g1 == *g2);
    }
  };
}


class NaryBoolExpr : public _BoolExpression {
protected :
  NaryBoolParamType params ;
public :
  virtual const char * getOpString() const = 0;

  const NaryBoolParamType & getParams () const { return params; }

  NaryBoolExpr (const NaryBoolParamType & pparams):params(pparams) {};

  BoolExpression eval () const {
    NaryBoolParamType p ;
    for (NaryBoolParamType::const_iterator it = params.begin(); it != params.end() ; ++it ) {
      BoolExpression e = it->eval();
      if (e.getType() == BOOLCONST) {
	bool val = e.getValue();
	if ( (getType() == AND && ! val) // XXX && false = false
	     || (getType() == OR && val) // XXX || true = true
	     )
	  return e;
	else
	  // XXX && true = XXX
	  // XXX || false = XXX
	  continue;
      } else {
	p.insert(e);
      }
    }
    if (p.empty())
      return BoolExpressionFactory::createConstant(false);
    else if (p.size() == 1) 
      return *p.begin();
    else 
      return BoolExpressionFactory::createNary(getType(),p);
  }

  void print (std::ostream & os) const {
    os << "( ";
    for (NaryBoolParamType::const_iterator it = params.begin() ;  ; ) {
      it->print(os);
      if (++it == params.end())
	break;
      else
	os << getOpString();
    }
    os << " )";
  }

  bool operator== (const _BoolExpression & e) const {
    const NaryBoolExpr & other = (const NaryBoolExpr &)e ;
    return other.params == params;
  }

  size_t hash () const {
    size_t res = getType();
    for (NaryBoolParamType::const_iterator it = params.begin() ; it != params.end()  ; ++it ) {
      res = res*(it->hash() +  10099);
    }
    return res;
  }

  BoolExpression setAssertion (const Assertion & a) const {
    NaryBoolParamType res ;
    for (NaryBoolParamType::const_iterator it = params.begin() ; it != params.end()  ; ++it ) {
      res.insert( (*it) & a );
    }
    return BoolExpressionFactory::createNary(getType(),res);    
  }

   bool isSupport (const Variable & v) const {
    for (NaryBoolParamType::const_iterator it = params.begin() ; it != params.end()  ; ++it ) {
      if (it->isSupport(v)) 
	return true;
    }
    return false;
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

class BinaryBoolComp : public _BoolExpression {
protected :
  IntExpression left;
  IntExpression right;
public :
  virtual const char * getOpString() const = 0;
  BinaryBoolComp (const IntExpression & lleft, const IntExpression & rright) : left (lleft),right(rright){};

  virtual bool constEval (int i, int j) const = 0;

  BoolExpression eval () const {
    IntExpression  l = left.eval();
    IntExpression  r = right.eval();

    if (l.getType() == CONST && r.getType() == CONST ) {
      return  BoolExpressionFactory::createConstant( constEval( l.getValue(),
								r.getValue()) );
    } else {
      return  BoolExpressionFactory::createComparison( getType(), l, r );
    }
  }

  

  BoolExpression setAssertion (const Assertion & a) const {
    IntExpression l = left & a;
    IntExpression r = right & a;
    return BoolExpressionFactory::createComparison(getType(),l,r);    
  }


  bool operator==(const _BoolExpression & e) const{
    const BinaryBoolComp & other = (const BinaryBoolComp &)e ;
    return other.left.equals(left) && other.right.equals(right);
  }
  size_t hash () const {
    size_t res = getType();
    res *= left.hash() *  76303 + right.hash() * 76147;
    return res;
  }
  void print (std::ostream & os) const {
    os << "( ";
    left.print(os);
    os << getOpString();
    right.print(os);
    os << " )";
  }

  bool isSupport (const Variable & v) const {
    return left.isSupport(v) || right.isSupport(v);
  }


};

class BoolEq : public BinaryBoolComp {

public :
  BoolEq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return EQ; }
  const char * getOpString() const { return " == ";}

  bool constEval (int i, int j) const {
    return i==j;
  }
};

class BoolNeq : public BinaryBoolComp {

public :
  BoolNeq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return NEQ; }
  const char * getOpString() const { return " != ";}

  bool constEval (int i, int j) const {
    return i!=j;
  }

};

class BoolGeq : public BinaryBoolComp {

public :
  BoolGeq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return GEQ; }
  const char * getOpString() const { return " >= ";}

  bool constEval (int i, int j) const {
    return i>=j;
  }

};


class BoolLeq : public BinaryBoolComp {

public :
  BoolLeq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LEQ; }
  const char * getOpString() const { return " <= ";}

  bool constEval (int i, int j) const {
    return i<=j;
  }

};

class BoolLt : public BinaryBoolComp {

public :
  BoolLt (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}

  bool constEval (int i, int j) const {
    return i<j;
  }

};

class BoolGt : public BinaryBoolComp {

public :
  BoolGt (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}

  bool constEval (int i, int j) const {
    return i>j;
  }

};


class NotExpr : public _BoolExpression {
  BoolExpression exp;  

public :
  NotExpr (const BoolExpression & eexp) : exp(eexp){};
  BoolExprType getType() const  { return NOT; }

  BoolExpression eval () const {
    BoolExpression e = exp.eval();
    if (e.getType() == BOOLCONST)
      return  BoolExpressionFactory::createConstant(! e.getValue());
    return  BoolExpressionFactory::createNot(e);
  }

  BoolExpression setAssertion (const Assertion & a) const {
    return ! (exp & a);
  }
  bool operator==(const _BoolExpression & e) const{
    const NotExpr & other = (const NotExpr &)e ;
    return other.exp == exp;
  }
  
  size_t hash () const {
    return  7829 * exp.hash();
  }

  void print (std::ostream & os) const {
    os << " ! (" ;
    exp.print(os);
    os << " ) ";
  }

  bool isSupport (const Variable & v) const {
    return exp.isSupport(v);
  }

};

class BoolConstExpr : public _BoolExpression {
  bool val;

public :
  BoolConstExpr (bool vval) : val(vval) {}
  BoolExprType getType() const  { return BOOLCONST; }

  bool getValue() const { return val;}

  BoolExpression eval () const {
    return this;
  }
  bool operator==(const _BoolExpression & e) const {
    return val == ((const BoolConstExpr &)e).val;
  }
  
  BoolExpression setAssertion (const Assertion & a) const {
    return this;
  }

  virtual size_t hash () const {
    return val?6829:102547;
  }
  void print (std::ostream & os) const {
    os << val;
  }

  bool isSupport (const Variable & v) const {
    return false;
  }

};


// namespace BoolExpressionFactory {
UniqueTable<_BoolExpression>  BoolExpressionFactory::unique = UniqueTable<_BoolExpression>();

BoolExpression BoolExpressionFactory::createNary (BoolExprType type, const NaryBoolParamType & params) {
    _BoolExpression * create;
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
    return unique(create);
  }

BoolExpression BoolExpressionFactory::createNot  (const BoolExpression & e) {
    _BoolExpression * create = new NotExpr(e);
    return unique(create);
  }

BoolExpression BoolExpressionFactory::createConstant (bool b) {
  return unique(new BoolConstExpr(b));
}

// a comparison (==,!=,<,>,<=,>=) between two integer expressions
BoolExpression BoolExpressionFactory::createComparison (BoolExprType type, const IntExpression & l, const IntExpression & r) {
  _BoolExpression * create;
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
  return unique(create);
}


void BoolExpressionFactory::destroy (_BoolExpression * e) {
  if (  e->deref() == 0 ){
    UniqueTable<_BoolExpression>::Table::iterator ci = unique.table.find(e);
    assert (ci != unique.table.end());
    unique.table.erase(ci);
    delete e;
  }
}


void BoolExpressionFactory::printStats (std::ostream &os) {
  os << "Boolean expression entries :" << unique.size() << std::endl;
}


// nary constructions
BoolExpression operator&&(const BoolExpression & a,const BoolExpression & b) {
  NaryBoolParamType p;
  if (a.getType() == AND) {
    const NaryBoolParamType & p2 = ((const NaryBoolExpr *) a.concrete)->getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert(a);
  }
  if (b.getType() == AND) {
    const NaryBoolParamType & p2 = ((const NaryBoolExpr *) b.concrete)->getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert (b);
  }
  return BoolExpressionFactory::createNary(AND, p);
}

BoolExpression operator||(const BoolExpression & a,const BoolExpression & b) {
  NaryBoolParamType p;
  if (a.getType() == OR) {
    const NaryBoolParamType & p2 = ((const NaryBoolExpr *) a.concrete)->getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert(a);
  }
  if (b.getType() == OR) {
    const NaryBoolParamType & p2 = ((const NaryBoolExpr *) b.concrete)->getParams();
    p.insert(p2.begin(), p2.end());
  } else {
    p.insert (b);
  }
  return BoolExpressionFactory::createNary(OR, p);
} 

BoolExpression BoolExpression::operator!() const {
  return BoolExpressionFactory::createNot(*this);
} 


// binary comparisons
BoolExpression operator==(const IntExpression & l, const IntExpression & r) {
  return  BoolExpressionFactory::createComparison (EQ,l,r);
}
BoolExpression operator!=(const IntExpression & l, const IntExpression & r) {
  return  BoolExpressionFactory::createComparison (NEQ,l,r);
}
BoolExpression operator<=(const IntExpression & l, const IntExpression & r) {
  return  BoolExpressionFactory::createComparison (LEQ,l,r);
}
BoolExpression operator>=(const IntExpression & l, const IntExpression & r) {
  return  BoolExpressionFactory::createComparison (GEQ,l,r);
}
BoolExpression operator<(const IntExpression & l, const IntExpression & r) {
  return  BoolExpressionFactory::createComparison (LT,l,r);
}
BoolExpression operator>(const IntExpression & l, const IntExpression & r) {
  return  BoolExpressionFactory::createComparison (GT,l,r);
}

std::ostream & operator << (std::ostream & os, const BoolExpression & e) {
  e.print(os);
  return os;
}
// necessary administrative trivia
// refcounting
BoolExpression::BoolExpression (const _BoolExpression * concret): concrete(concret) {
  concrete->ref();
}

BoolExpression::BoolExpression (const BoolExpression & other) {
  if (this != &other) {
    concrete = other.concrete;
    concrete->ref();
  }
}

bool BoolExpression::isSupport (const Variable & v) const {
  return concrete->isSupport(v);
}

BoolExpression::~BoolExpression () {
  // remove const qualifier for delete call
  BoolExpressionFactory::destroy((_BoolExpression *) concrete);  
}

BoolExpression & BoolExpression::operator= (const BoolExpression & other) {
  if (this != &other) {
    // remove const qualifier for delete call
    BoolExpressionFactory::destroy((_BoolExpression *) concrete);
    concrete = other.concrete;
    concrete->ref();
  }
  return *this;
}

bool BoolExpression::operator== (const BoolExpression & other) const {
  return concrete == other.concrete ;
}

bool BoolExpression::operator< (const BoolExpression & other) const {
  return concrete < other.concrete;
}

void BoolExpression::print (std::ostream & os) const {
  concrete->print(os);
}


BoolExpression BoolExpression::eval () const {
  return concrete->eval();
}

/// only valid for CONST expressions
/// use this call only in form : if (e.getType() == CONST) { int j = e.getValue() ; ...etc }
/// Exceptions will be thrown otherwise.
bool BoolExpression::getValue () const {
  if (getType() != BOOLCONST) {
    throw "Do not call getValue on non constant Boolean expressions.";
  } else {
    return ((const BoolConstExpr *) concrete)->getValue();    
  }
}



BoolExpression BoolExpression::operator& (const Assertion &a) const {
  return concrete->setAssertion(a);
}

BoolExprType BoolExpression::getType() const {
  return concrete->getType();
}


size_t BoolExpression::hash () const { 
  return ddd::knuth32_hash(reinterpret_cast<const size_t>(concrete)); 
}
