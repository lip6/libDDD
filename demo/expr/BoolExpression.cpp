#include "BoolExpression.hpp"
#include "hashfunc.hh"
#include <cassert>
#include <typeinfo>



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
  virtual _BoolExpression * clone () const = 0;

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


namespace d3 { namespace util {
  template<>
  struct hash<_BoolExpression*> {
    size_t operator()(_BoolExpression * g) const{
      return g->hash();
    }
  };

  template<>
  struct equal<_BoolExpression*> {
    bool operator()(_BoolExpression *g1,_BoolExpression *g2) const{
      return (typeid(*g1) == typeid(*g2) && *g1 == *g2);
    }
  };
} }


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
  virtual _BoolExpression * clone () const { return new OrExpr(*this);}
};

class AndExpr : public NaryBoolExpr {

public :
  AndExpr (const NaryBoolParamType  & pparams):NaryBoolExpr(pparams) {};
  BoolExprType getType() const  { return AND; }
  const char * getOpString() const { return " && ";}
  virtual _BoolExpression * clone () const { return new AndExpr(*this);}
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
  virtual _BoolExpression * clone () const { return new BoolEq(*this);}
};

class BoolNeq : public BinaryBoolComp {

public :
  BoolNeq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return NEQ; }
  const char * getOpString() const { return " != ";}

  bool constEval (int i, int j) const {
    return i!=j;
  }

  virtual _BoolExpression * clone () const { return new BoolNeq(*this);}
};

class BoolGeq : public BinaryBoolComp {

public :
  BoolGeq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return GEQ; }
  const char * getOpString() const { return " >= ";}

  bool constEval (int i, int j) const {
    return i>=j;
  }
  virtual _BoolExpression * clone () const { return new BoolGeq(*this);}
};


class BoolLeq : public BinaryBoolComp {

public :
  BoolLeq (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LEQ; }
  const char * getOpString() const { return " <= ";}

  bool constEval (int i, int j) const {
    return i<=j;
  }
  virtual _BoolExpression * clone () const { return new BoolLeq(*this);}
};

class BoolLt : public BinaryBoolComp {

public :
  BoolLt (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}

  bool constEval (int i, int j) const {
    return i<j;
  }
  virtual _BoolExpression * clone () const { return new BoolLt(*this);}
};

class BoolGt : public BinaryBoolComp {

public :
  BoolGt (const IntExpression & left, const IntExpression & right) : BinaryBoolComp(left,right) {};
  BoolExprType getType() const  { return LT; }
  const char * getOpString() const { return " < ";}

  bool constEval (int i, int j) const {
    return i>j;
  }
  virtual _BoolExpression * clone () const { return new BoolGt(*this);}
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
  virtual _BoolExpression * clone () const { return new NotExpr(*this);}
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
  virtual _BoolExpression * clone () const { return new BoolConstExpr(*this);}
  BoolExpression setAssertion (const Assertion&) const {
    return this;
  }

  virtual size_t hash () const {
    return val?6829:102547;
  }
  void print (std::ostream & os) const {
    os << val;
  }

  bool isSupport (const Variable&) const {
    return false;
  }

};


// namespace BoolExpressionFactory {
UniqueTable<_BoolExpression>  BoolExpressionFactory::unique = UniqueTable<_BoolExpression>();

BoolExpression BoolExpressionFactory::createNary (BoolExprType type, const NaryBoolParamType & params) {
    switch (type) {
    case OR :
      return unique(OrExpr (params));      
    case AND :
      return unique(AndExpr (params));      
    default :
      throw "Operator is not nary";
    }
  }

BoolExpression BoolExpressionFactory::createNot  (const BoolExpression & e) {
  return unique(NotExpr(e));
}

BoolExpression BoolExpressionFactory::createConstant (bool b) {
  return unique(BoolConstExpr(b));
}

// a comparison (==,!=,<,>,<=,>=) between two integer expressions
BoolExpression BoolExpressionFactory::createComparison (BoolExprType type, const IntExpression & l, const IntExpression & r) {
  switch (type) {
  case EQ :
    return unique(BoolEq (l,r));
  case NEQ :
    return unique(BoolNeq (l,r));
  case GEQ :
    return unique(BoolGeq (l,r));
  case LEQ :
    return unique(BoolLeq (l,r));
  case LT :
    return unique(BoolLt (l,r));
  case GT :
    return unique(BoolGt (l,r));      
  default :
    throw "not a binary comparison operator !";
  }
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
