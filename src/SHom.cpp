#include <typeinfo>
#include <cassert>
#include <iostream>

#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "SHom.h"
#include "SDD.h"
#include "SDED.h"
#include "UniqueTable.h"
#include "DataSet.h"
#include "MemoryManager.h"
#include <typeinfo>

namespace namespace_SHom {

} //end namespace namespace_SHom 

/* Unique Table */
namespace __gnu_cxx {
  template<>  struct hash<_GShom*>{
    size_t operator()(_GShom * _h) const{
      return _h->hash();
    }
  };
}

namespace std {
  template<>  struct equal_to<_GShom*>{
    bool operator()(_GShom * _h1,_GShom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}

static UniqueTable<_GShom> canonical;
/*************************************************************************/
/*                         Class _GShom                                   */
/*************************************************************************/

namespace S_Homomorphism {
/************************** Identity */
class Identity:public _GShom{
public:
  /* Constructor */
  Identity(int ref=0):_GShom(ref,true){}

  /* Compare */
  bool operator==(const _GShom &h) const{return true;}
  size_t hash() const{return 17;}

  bool
  skip_variable(int) const 
  {
      return true;
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{return d;}
};

/************************** Constant */
class Constant:public _GShom{
private:
  GSDD value;
public:
  /* Constructor */
  Constant(const GSDD &d,int ref=0):_GShom(ref,true),value(d){}

  /* Compare */
  bool operator==(const _GShom &h) const{
    return value==((Constant*)&h )->value;
  }

  size_t hash() const{
    return value.hash();
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return d==GSDD::null?GSDD::null:value;
  }

  /* Memory Manager */
  void mark() const{
    value.mark();
  }
};

/************************** Mult */
class Mult:public _GShom{
private:
  GShom left;
  GSDD right;
public:
  /* Constructor */
  Mult(const GShom &l,const GSDD &r,int ref=0):_GShom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Mult*)&h )->left && right==((Mult*)&h )->right;
  }
  size_t hash() const{
    return 83*left.hash()+53*right.hash();
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(d)*right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};


/*************************************************************************/
/*                         Class LocalApply                              */
/*************************************************************************/

class LocalApply
	:
	public StrongShom
{

public:

  GHom h;
  int target;

	LocalApply()
		:
		h(), target(0)
	{}

  LocalApply (const GHom& hh,int t) :h(hh),target(t) {}

  GSDD phiOne() const {
	  // target not encountered !
    return GSDD::top;
  }     

  // optimize away needless exploration of upstream modules that dont contain the place
  bool skip_variable (int var) const {
	  return var != target;
  }
  
  GShom phi(int vr, const DataSet & vl) const {

//    cerr << "target // curvar :" << target << " // " << vr<<endl;
		assert( typeid(vl) == typeid(const DDD&) );
		DDD v2 = h((const DDD &)vl);
		return GShom(vr,v2);

  }

  void mark() const {
    h.mark();
  }  
  
  size_t hash() const {
    return  h.hash() ^ target * 21727; 
  }

  bool operator==(const StrongShom &s) const {
    LocalApply* ps = (LocalApply *)&s;
    return target == ps->target && h ==  ps->h;
  }  

};

/************************** Add */
/************************** Add */
class Add
    :
    public _GShom
{

// types
public:
	
	typedef
	struct
	{
		GShom F; 
		std::set<GShom> G;
		LocalApply* L;
		bool has_local;
		
	} partition;

    typedef std::map<int,partition> partition_cache_type;
    

private:
        
    std::set<GShom> parameters;
	mutable partition_cache_type partition_cache;
	bool have_id;


public:
        

    Add(const std::set<GShom>& p, int ref=0)
    	:
        _GShom(ref,true),
        parameters(),
		have_id(false)
    {
		std::map<int, GHom> local_homs;
	
        for( std::set<GShom>::const_iterator it = p.begin(); it != p.end(); ++it)
        {
            if( typeid( *get_concret(*it) ) == typeid(Add) )
            {
                std::set<GShom>& local_param = ((Add*)get_concret(*it))->parameters;
                parameters.insert( local_param.begin() , local_param.end());
            }
			else if( typeid(*get_concret(*it) ) == typeid(LocalApply) )
			{
				LocalApply* local = (LocalApply*)(get_concret(*it));
				std::map<int, GHom>::iterator f = local_homs.find( local->target );
				
				if( f != local_homs.end() )
				{
					f->second = f->second + local->h;
				}
				else
				{
					local_homs.insert(std::make_pair(local->target,local->h));
				}
				
			}
            else
            {
                if( typeid(*get_concret(*it)) == typeid(Identity) )
                {
                    have_id = true;
                }
                parameters.insert(*it);
            }
        }

		for( 	std::map<int, GHom>::iterator it = local_homs.begin();
				it != local_homs.end();
				++it)
		{
			if( have_id )
			{
				it->second = it->second + GHom::id;
			}
			parameters.insert(localApply(it->second,it->first));
		}
		
    }

    bool
    get_have_id() const
    {
        return have_id;
    }

    /* Compare */
    bool
    operator==(const _GShom &h) const
    {
    	return parameters == ((Add*)&h )->parameters;	
    }
    
    size_t
    hash() const
    {
	    size_t res = 0;
	    for(std::set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
	      res^=gi->hash();
	    return res;

    }

	bool
	skip_variable( int var ) const
	{
		partition_cache_type::iterator part_it = partition_cache.find(var);
		if( part_it == partition_cache.end() )
		{
			part_it = partition_cache.insert(std::make_pair(var,partition())).first;
			partition& part = part_it->second;
			part.has_local = false;
			part.L  = NULL;
			std::set<GShom> F;
			
			for(	std::set<GShom>::const_iterator gi = parameters.begin();
					gi != parameters.end();
					++gi )
			{
				if( get_concret(*gi)->skip_variable(var) )
                {
                    // F part
					F.insert(*gi);
                }
				else if( typeid(*get_concret(*gi) ) == typeid(LocalApply) )
				{
					// L part
					assert (!part.has_local);
					part.L = (LocalApply*)(get_concret(*gi));
					part.has_local = true;
				}
                else
                {
                    // G part
                    part.G.insert(*gi);
                }
			}
			part.F = GShom::add(F);
		}

		return part_it->second.G.empty() && !part_it->second.has_local;
	}

    partition
    get_partition(int var)
    {
        this->skip_variable(var);
        return partition_cache.find(var)->second;
    }


    /* Eval */
	GSDD
	eval(const GSDD& d)const
	{
		if( d == GSDD::null )
		{
			return GSDD::null;
		}
		else if( d == GSDD::one || d == GSDD::top )
		{
			std::set<GSDD> s;
		
			for(std::set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
			{
				s.insert((*gi)(d));
			}
			return SDED::add(s);
		}
		else
		{
			std::set<GSDD> s;
			int var = d.variable();

			partition_cache_type::iterator part_it = partition_cache.find(var);
			if( part_it == partition_cache.end() )
			{
				this->skip_variable(var);
				part_it = partition_cache.find(var);
			}              
		
			std::set<GShom>& G = part_it->second.G;

			for( 	std::set<GShom>::const_iterator it = G.begin(); 
					it != G.end();
					++it )
			{
				s.insert((*it)(d));                  
			} 

			s.insert( part_it->second.F(d) );
			if( part_it->second.L != NULL )
			  s.insert(GShom(part_it->second.L)(d));
			
			return SDED::add(s);
		}

	}

    /* Memory Manager */
    void
    mark() const
    {
		for( std::set<GShom>::const_iterator gi=parameters.begin();
			 gi!=parameters.end();
			 ++gi)
		{
			gi->mark();
		}

    }

};

/************************** Compose */
class Compose:public _GShom{
private:
  GShom left;
  GShom right;
public:
  /* Constructor */
  Compose(const GShom &l,const GShom &r,int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Compose*)&h )->left && right==((Compose*)&h )->right;
  }
  size_t hash() const{
    return 13*left.hash()+7*right.hash();
  }

	bool
    skip_variable(int var) const 
    {
        return get_concret(left)->skip_variable(var)
			and get_concret(right)->skip_variable(var);
    }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(right(d));
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** LeftConcat */
class LeftConcat:public _GShom{
private:
  GSDD left;
  GShom right;
public:
  /* Constructor */
  LeftConcat(const GSDD &l,const GShom &r,int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((LeftConcat*)&h )->left && right==((LeftConcat*)&h )->right;
  }
  size_t hash() const{
    return 23*left.hash()+47*right.hash();
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left^right(d);
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** RightConcat */
class RightConcat:public _GShom{
private:
  GShom left;
  GSDD right;
public:
  /* Constructor */
  RightConcat(const GShom &l,const GSDD &r,int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((RightConcat*)&h )->left && right==((RightConcat*)&h )->right;
  }
  size_t hash() const{
    return 47*left.hash()+19*right.hash();
  }

  bool
  skip_variable(int var) const
  {
      return get_concret(left)->skip_variable(var);
  }


  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(d)^right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Minus */
class Minus:public _GShom{
private:
  GShom left;
  GSDD right;
public:
  /* Constructor */
  Minus(const GShom &l,const GSDD &r,int ref=0):_GShom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Minus*)&h )->left && right==((Minus*)&h )->right;
  }
  size_t hash() const{
    return 5*left.hash()+61*right.hash();
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(d)-right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Fixpoint */
class Fixpoint:public _GShom{
private:
  GShom arg;
public:
  /* Constructor */
  Fixpoint(const GShom &a,int ref=0):_GShom(ref),arg(a){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return arg==((Fixpoint*)&h )->arg ;
  }
  size_t hash() const{
    return 17*arg.hash();
  }

  bool
  skip_variable(int var) const
  {
      return get_concret(arg)->skip_variable(var);
  }


  /* Eval */
	GSDD 
	eval(const GSDD &d) const
	{
	if( d == GSDD::null )
	{
		return GSDD::null;
	}
	else if( d == GSDD::one or d == GSDD::top )
	{
		return arg(d);
	}
	else
	{
		int variable = d.variable();

		GSDD d1 = d;
		GSDD d2 = d;

		// is it the fixpoint of an union ?
		if( typeid( *get_concret(arg) ) == typeid(Add) )
		{
			// Check if we have ( Id + F + G )* where F can be forwarded to the next variable
		
			// Rewrite ( Id + F + G )*
			// into ( O(Gn + Id) o (O(Fn + Id)*)* )* 
			Add* add = ((Add*)get_concret(arg));
			if( add->get_have_id() )
			{                           
				Add::partition partition = add->get_partition(variable);

				// operations that can be forwarded to the next variable
				GShom F_part = fixpoint(partition.F);

				GShom L_part = partition.has_local
							? localApply(fixpoint(GHom(partition.L->h)),variable)
							: GShom::id
							;
				
				do
				{
					d1 = d2;
					
					d2 = F_part(d2);
					d2 = L_part(d2);

					for( 	std::set<GShom>::const_iterator G_it = partition.G.begin();
							G_it != partition.G.end();
							++G_it) 
					{

						// d2 = F_part(d2);
						// d2 = L_part(d2);

						// apply local part
						// d2 = L_part(d2);
					  // chain application of Shom of this level
					  d2 = (*G_it) (d2) + d2;
					d2 = F_part(d2) + d2;
					
					}
				}
				while (d1 != d2);
				return d1;
			}
		}                                                                                               

		do
		{
			d1 = d2;
			d2 = arg(d2);
		}
		while (d1 != d2);

		return d1;
	}
}

  /* Memory Manager */
  void mark() const{
    arg.mark();
  }
};




} // end namespace H_Homomorphism

GSDD 
_GShom::eval_skip(const GSDD& d) const
{
    if( d == GSDD::null )
    {
        return GSDD::null;
    }
    else if( d == GSDD::one )
    {
        // basic case, mustn't call d.variable()
    }
    else if( d == GSDD::top )
    {
        return GSDD::top;
    }
    else if( this->skip_variable(d.variable()) )
    {
        GSDD::Valuation v;
		std::map<GSDD,DataSet *> res;

        for( GSDD::const_iterator it = d.begin();
            it != d.end();
            ++it)
        {
            GSDD son = GShom(this)(it->second);
            if( son != GSDD::null && !(it->first->empty()) )
            {
                // v.push_back(std::make_pair(it->first->newcopy(),son));
				square_union(res, son, it->first);
            }
        }

		v.reserve(res.size());  
	  	for (std::map<GSDD,DataSet *>::iterator it =res.begin() ;
				it!= res.end();
				++it)
		{
			v.push_back(std::make_pair(it->second,it->first));
		}
        
        if( v.empty() )
        {
            return GSDD::null;
        }
        else
        {
            return GSDD(d.variable(),v);
        }
        
    }
    
    return eval(d);
}

/*************************************************************************/
/*                         Class StrongShom                               */
/*************************************************************************/

/* Compare */
bool StrongShom::operator==(const _GShom &h) const{
  return typeid(*this)==typeid(h)?*this==*(StrongShom*)&h:false;
}


/* Eval */
GSDD StrongShom::eval(const GSDD &d)const{
  if(d==GSDD::null)
    return GSDD::null;
  else if(d==GSDD::one)
    return phiOne();
  else if(d==GSDD::top)
    return GSDD::top;
  else{
    int variable=d.variable();
    std::set<GSDD> s;

    for(GSDD::const_iterator vi=d.begin();vi!=d.end();++vi){
      s.insert(phi(variable,*vi->first)(vi->second));
    }
    return SDED::add(s);
  }
}




/*************************************************************************/
/*                         Class GShom                                    */
/*************************************************************************/

/* Constructor */
GShom::GShom(const StrongShom *h):concret(h){}

GShom::GShom(StrongShom *h):concret(canonical(h)){}

GShom::GShom(const GSDD& d):concret(canonical(new S_Homomorphism::Constant(d))){}

GShom::GShom(int var,const DataSet & val, const GShom &h) {
  if ( ! val.empty() ) {
    concret=  canonical(new S_Homomorphism::LeftConcat(GSDD(var,val),h));
  } else {
    concret = canonical(new S_Homomorphism::Constant(GSDD::null));
  }
}

/* Eval */
GSDD GShom::operator()(const GSDD &d) const{
  if(concret->immediat)
    return concret->eval(d);
  else
    return SDED::Shom(*this,d);
}

GSDD GShom::eval(const GSDD &d) const{
  return concret->eval_skip(d);
}

const GShom GShom::id(canonical(new S_Homomorphism::Identity(1)));

int GShom::refCounter() const{return concret->refCounter;}

/* Sum */

// GShom GShom::add(const set<GShom>& s){
//    return(new Sum(s));
// }


/* Memory Manager */
unsigned int GShom::statistics(){
  return canonical.size();
}

// Todo
void GShom::mark()const{
  if(!concret->marking){
    concret->marking=true;
    concret->mark();
  }
};

void GShom::garbage(){
  // mark phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
    if((*di)->refCounter!=0){
      (*di)->marking=true;
      (*di)->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!(*di)->marking){
      UniqueTable<_GShom>::Table::iterator ci=di;
      di++;
      _GShom *g=*ci;
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      di++;
    }
  }
}

/*************************************************************************/
/*                    Class Shom                                          */
/*************************************************************************/
/* Constructor */
Shom::Shom(const Shom &h):GShom(h.concret){
  concret->refCounter++;
}

Shom::Shom(const GShom &h):GShom(h.concret){
  concret->refCounter++;
}

Shom::Shom(const GSDD& d):GShom(d){
  concret->refCounter++;
}

Shom::Shom(int var,const DataSet &  val, const GShom &h):GShom(var,val,h){
  concret->refCounter++;
}

Shom::~Shom(){
  assert(concret->refCounter>0);
  concret->refCounter--;
}

/* Set */

Shom &Shom::operator=(const Shom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

Shom &Shom::operator=(const GShom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

/* Operations */
GShom fixpoint (const GShom &h) {
  return GShom(canonical(new S_Homomorphism::Fixpoint(h)));
}

GShom
// localApply(int target,const GHom & h)
localApply(const GHom & h, int target)
{
	return new S_Homomorphism::LocalApply(h,target);
}

GShom GShom::add(const std::set<GShom>& s)
{
	if( s.size() == 1 )
		return *(s.begin());
	else
  		return GShom(canonical(new S_Homomorphism::Add(s)));   
}

GShom operator&(const GShom &h1,const GShom &h2){
	
	if( h1 == GShom::id )
		return h2;

	if( h2 == GShom::id )
		return h1;

	if( typeid( *_GShom::get_concret(h1) ) == typeid(S_Homomorphism::LocalApply) 
		&& typeid( *_GShom::get_concret(h2) ) == typeid(S_Homomorphism::LocalApply) )
	{
		S_Homomorphism::LocalApply* lh1 = (S_Homomorphism::LocalApply*)(_GShom::get_concret(h1));
		S_Homomorphism::LocalApply* lh2 = (S_Homomorphism::LocalApply*)(_GShom::get_concret(h2));

		if( lh1->target == lh2->target )
		{
			return localApply(  lh1->h & lh2->h, lh1->target );
		}
	}
	
  	return GShom(canonical(new S_Homomorphism::Compose(h1,h2)));
}

GShom operator+(const GShom &h1,const GShom &h2){
  // if (h1 < h2) 
  //   return GShom(canonical(new S_Homomorphism::Add(h1,h2)));
  // else
 //   return GShom(canonical(new S_Homomorphism::Add(h2,h1)));

  std::set<GShom> s;
  s.insert(h1);
  s.insert(h2);

  return GShom(canonical(new S_Homomorphism::Add(s)));

}

GShom operator*(const GSDD &d,const GShom &h){
  return GShom(canonical(new S_Homomorphism::Mult(h,d)));
}

GShom operator*(const GShom &h,const GSDD &d){
  return GShom(canonical(new S_Homomorphism::Mult(h,d)));
}

GShom operator^(const GSDD &d,const GShom &h){
  return GShom(canonical(new S_Homomorphism::LeftConcat(d,h)));
}

GShom operator^(const GShom &h,const GSDD &d){
  return GShom(canonical(new S_Homomorphism::RightConcat(h,d)));
}

GShom operator-(const GShom &h,const GSDD &d){
  return GShom(canonical(new S_Homomorphism::Minus(h,d)));
}


GShom::GShom(const MyGShom* h) :
  concret(h)
{}

GShom::GShom(MyGShom* h) :
  concret(h)
{}

void GShom::pstats(bool reinit)
{
  std::cout << "*\nGSHom Stats : size unicity table = " <<  canonical.size() << std::endl;
}
