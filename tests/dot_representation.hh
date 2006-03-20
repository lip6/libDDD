#ifndef DOT_REPRESENTATION__
#define DOT_REPRESENTATION__

// libDDD headers :
#include <DataSet.h>
#include <DDD.h>
#include <SDD.h>

// STD headers :
#include <string>
#include <ostream>

//#include "operation.hh"
#include <boost/shared_ptr.hpp>

namespace xxx
{
  class base_data
  {
  public:
    virtual ~base_data() {}
  };
  class value_data : public base_data
  {
  public:
    int value;
    value_data(int x) :
      value(x)
    {}
  };
  class ddd_data : public base_data,
                   public DDD
  {
  public:
    ddd_data(const DDD& ddd) :
      base_data(),
      DDD(ddd)
    {}
    ddd_data(const GDDD& ddd) :
      base_data(),
      DDD(ddd)
    {}
  };
  class sdd_data : public base_data,
                   public SDD
  {
  public:
    sdd_data(const SDD& sdd) :
      base_data(),
      SDD(sdd)
    {}
    sdd_data(const GSDD& sdd) :
      base_data(),
      SDD(sdd)
    {}
  };
  typedef boost::shared_ptr<base_data> data_set;
  bool operator==(const data_set& l, const data_set& r);

  namespace dot
  {

    void dot_export(const std::string& file,
		    const data_set& dd,
		    const std::string& name);

    void dot_export(std::ostream& stream,
		    const data_set& dd,
		    const std::string& name);

    unsigned int count(const data_set& dd);

    long double states(const data_set& dd);

/*    void dot_export(const std::string& file,
		    const operation& op,
		    const std::string& name);

    void dot_export(const std::ostream& stream,
		    const operation& op,
		    const std::string& name);
*/
  }
}

#endif // DOT_REPRESENTATION__
