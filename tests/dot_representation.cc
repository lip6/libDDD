#include <dot_representation.hh>

// libDDD headers :
#include <DDD.h>
#include <SDD.h>

// STL headers :
#include <ext/hash_map>
#include <set>
#include <map>
#include <limits>

// STD headers :
#include <fstream>

namespace xxx
{


namespace dot
{

  struct ddd_hash
  {
    size_t operator()(const DDD& ddd) const
    {return ddd.set_hash();}
  };
  struct ddd_equal
  {
    bool operator()(const DDD& ddd1, const DDD& ddd2) const
    {return ddd1 == ddd2;}
  };
  struct sdd_hash
  {
    size_t operator()(const SDD& sdd) const
    {return sdd.set_hash();}
  };
  struct sdd_equal
  {
    bool operator()(const SDD& sdd1, const SDD& sdd2) const
    {return sdd1 == sdd2;}
  };

  unsigned int
  dot_export(DDD ddd,
             unsigned int& last,
             __gnu_cxx::hash_map<DDD, unsigned int, ddd_hash, ddd_equal>& ddd_unicity,
             __gnu_cxx::hash_map<unsigned int, std::map<int, unsigned int> >& ddd_successors,
             __gnu_cxx::hash_map<unsigned int, int>& variables)
  {
    if (ddd_unicity.find(ddd) != ddd_unicity.end())
      return ddd_unicity[ddd]; // node is already in table
    unsigned int id = ddd_unicity[ddd] = last++;
    variables[id] = ddd.variable();
    for (DDD::const_iterator succ = ddd.begin();
         succ != ddd.end();
         ++succ)
      ddd_successors[id][succ->first] =
        dot_export(DDD(succ->second), last, ddd_unicity, ddd_successors, variables);
    return id;
  }

  unsigned int
  dot_export(SDD sdd,
             unsigned int& last,
             unsigned int depth,
             __gnu_cxx::hash_map<DDD, unsigned int, ddd_hash, ddd_equal>& ddd_unicity,
             __gnu_cxx::hash_map<SDD, std::pair<unsigned int, unsigned int>, sdd_hash, sdd_equal>& sdd_unicity,
             __gnu_cxx::hash_map<unsigned int, std::map<int, unsigned int> >& ddd_successors,
             __gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> >& sdd_successors,
             __gnu_cxx::hash_map<unsigned int, int>& variables)
  {
    unsigned int id;
    if (sdd_unicity.find(sdd) != sdd_unicity.end())
      {
        sdd_unicity[sdd].second = std::max(sdd_unicity[sdd].second, depth);
        id = sdd_unicity[sdd].first; // node is already in table
      }
    else
      {
        id = last++;
        if (sdd != SDD::one)
          {
            sdd_unicity[sdd].first = id;
            sdd_unicity[sdd].second = depth;
            variables[id] = sdd.variable();
          }
      }
    if (sdd.begin() != sdd.end())
      if (dynamic_cast<DDD*>(sdd.begin()->first) != NULL)
        // arcs are valued with DDDs
        for (SDD::const_iterator succ = sdd.begin();
             succ != sdd.end();
             ++succ)
          {
            DDD* ddd = dynamic_cast<DDD*>(succ->first);
            sdd_successors[id][dot_export(*ddd, last, ddd_unicity, ddd_successors, variables)] =
              dot_export(succ->second, last, depth, ddd_unicity, sdd_unicity, ddd_successors, sdd_successors, variables);
          }
      else if (dynamic_cast<SDD*>(sdd.begin()->first) != NULL)
        // arcs are valued with SDDs
        for (SDD::const_iterator succ = sdd.begin();
             succ != sdd.end();
             ++succ)
          {
            SDD* sdd = dynamic_cast<SDD*>(succ->first);
            sdd_successors[id][dot_export(*sdd, last, depth+1, ddd_unicity, sdd_unicity, ddd_successors, sdd_successors, variables)] =
              dot_export(succ->second, last, depth, ddd_unicity, sdd_unicity, ddd_successors, sdd_successors, variables);
          }
    return id;
  }

  void
  get_successors(unsigned int id,
                 std::set<unsigned int>& result,
                 const __gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> >& sdd_successors)
  {
    // TODO : assert find != end()
    __gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> >::const_iterator i =
      sdd_successors.find(id);
    if (i != sdd_successors.end())
      {
        result.insert(id);
        const std::map<unsigned int, unsigned int>& succs = i->second;
        for (std::map<unsigned int, unsigned int>::const_iterator succ = succs.begin();
             succ != succs.end();
             ++succ)
          get_successors(succ->second, result, sdd_successors);
      }
  }

  /*
  bool operator==(const std::pair<unsigned int, unsigned int>& x1,
                  const std::pair<unsigned int, unsigned int>& x2)
  {
    return x1.first == x2.first
      && x1.second == x2.second;
  }
  */

  void
  dot_export(std::ostream& output,
             const __gnu_cxx::hash_map<DDD, unsigned int, ddd_hash, ddd_equal>& ddd_unicity,
             const __gnu_cxx::hash_map<SDD, std::pair<unsigned int, unsigned int>, sdd_hash, sdd_equal>& sdd_unicity,
             const __gnu_cxx::hash_map<unsigned int, std::map<int, unsigned int> >& ddd_successors,
             const __gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> >& sdd_successors,
             const __gnu_cxx::hash_map<unsigned int, int>& variables)
  {
    // export DDDs in a cluster :
    output << "subgraph clusterDDD {"
	     << std::endl
	     << "\trankdir=LR"
	     << std::endl;
      for (__gnu_cxx::hash_map<DDD, unsigned int, ddd_hash, ddd_equal>::const_iterator ddd  =
	     ddd_unicity.begin();
	   ddd != ddd_unicity.end();
	   ++ddd)
	{
	  __gnu_cxx::hash_map<unsigned int, std::map<int, unsigned int> >::const_iterator succ
	    = ddd_successors.find(ddd->second);
	  if ((succ == ddd_successors.end())
	      || (succ->second.empty()))
	    output << "\t"
		   << ddd->second
		   << " [ label=\""
		   << DDD::getvarName(ddd->first.variable())
		   << "\", shape=\"box\" ];"
		   << std::endl;
	  else
	    output << "\t"
		   << ddd->second
		   << " [ label=\""
		   << DDD::getvarName(ddd->first.variable())
		   << "\", shape=\"circle\" ];"
		   << std::endl;
	}
      output << "}" << std::endl;
      // export SDDs :
      // order by depth
      std::map<unsigned int, std::map<unsigned int, const SDD> > depths;
      for (__gnu_cxx::hash_map<SDD, std::pair<unsigned int, unsigned int>, sdd_hash, sdd_equal>::const_iterator sdd =
	     sdd_unicity.begin();
	   sdd != sdd_unicity.end();
	   ++sdd)
	depths[sdd->second.second].insert(std::make_pair(sdd->second.first, sdd->first));
      // build reverse of predecessors
      std::map<unsigned int, std::set<unsigned int> > predecessors;
      for (__gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> >::const_iterator i = sdd_successors.begin();
	   i != sdd_successors.end();
	   ++i)
	{
	  predecessors[i->first];
	  for (std::map<unsigned int, unsigned int>::const_iterator j = i->second.begin();
	       j != i->second.end();
	       ++j)
	    predecessors[j->second].insert(i->first);
	}
      // get roots
      std::set<unsigned int> roots;
      for (std::map<unsigned int, std::set<unsigned int> >::const_iterator pred = predecessors.begin();
	   pred != predecessors.end();
	   ++pred)
	if (pred->second.empty())
	  roots.insert(pred->first);
      __gnu_cxx::hash_set<unsigned int> points;
      // for each depth, create clusters
      for (std::map<unsigned int, std::map<unsigned int, const SDD> >::const_iterator depth =
	     depths.begin();
	   depth != depths.end();
	   ++depth)
	{
	  for (std::map<unsigned int, const SDD>::const_iterator node = depth->second.begin();
	       node != depth->second.end();
	       ++node)
	    if (roots.find(node->first) != roots.end())
	      {
		output << "subgraph cluster" << node->first << " {"
		       << std::endl
		       << "\trankdir=LR"
		       << std::endl;
		std::set<unsigned int> succs;
		get_successors(node->first, succs, sdd_successors);
		for (std::set<unsigned int>::const_iterator i = succs.begin();
		     i != succs.end();
		     ++i)
		  {
		    output << "\t"
			   << *i
			   << " [ label=\""
			   << variables.find(*i)->second
			   << "\", shape=\"circle\" ];"
			   << std::endl;
		    if (! predecessors[*i].empty())
		      {
			output << "\t"
			       << std::numeric_limits<unsigned int>::max() - *i
			       << " [ label=\"\", shape=\"point\" ];"
			       << std::endl;
			points.insert(std::numeric_limits<unsigned int>::max() - *i);
		      }
		  }
		output << "}"
		       << std::endl;
	      }
	}
      // output arcs valued with integers :
      for (__gnu_cxx::hash_map<unsigned int, std::map<int, unsigned int> >::const_iterator arc =
	     ddd_successors.begin();
	   arc != ddd_successors.end();
	   ++arc)
	for (std::map<int, unsigned int>::const_iterator destination = arc->second.begin();
	     destination != arc->second.end();
	     ++destination)
	  output << "\t"
		 << arc->first
		 << " -> "
		 << destination->second
		 << " [ label=\""
		 << destination->first
		 << "\", weight=1 ];"
		 << std::endl;
      // output arcs valued with nodes :
      std::set< std::pair<unsigned int, unsigned int> > found_arcs;
      for (__gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> >::const_iterator arc =
	     sdd_successors.begin();
	   arc != sdd_successors.end();
	   ++arc)
	for (std::map<unsigned int, unsigned int>::const_iterator destination = arc->second.begin();
	     destination != arc->second.end();
	     ++destination)
	  {
	    unsigned int point = std::numeric_limits<unsigned int>::max() - destination->second;
	    if (points.find(point) == points.end())
	      {
		output << "\t"
		       << point
		       << " [ label=\"\", shape=\"point\" ];"
		       << std::endl;
		points.insert(point);
	      }
	    if (sdd_successors.find(destination->second) == sdd_successors.end())
	      output << "\t"
		     << arc->first
		     << " -> "
		     << point
		     << " [ weight=1 ];"
		     << std::endl
		     << "\t"
		     << point
		     << " -> "
		     << destination->first
		     << " [ style=\"dotted\", weight=1 ];"
		     << std::endl;
	    else
	      {
		output << "\t"
		       << arc->first
		       << " -> "
		       << point
		       << " [ weight=255 ];"
		       << std::endl;
		if (found_arcs.find(std::make_pair(point, destination->second)) == found_arcs.end())
		  {
		    output << "\t"
			   << point
			   << " -> "
			   << destination->second
			   << " [ weight=255 ];"
			   << std::endl;
		    found_arcs.insert(std::make_pair(point, destination->second));
		  }
		output << "\t"
		       << point
		       << " -> "
		       << destination->first
		       << " [ style=\"dotted\", weight=1 ];"
		       << std::endl;
	      }
	  }
    }

    void count(const DataSet* s,
	       __gnu_cxx::hash_set<DDD, ddd_hash, ddd_equal>& ddd_unicity,
	       __gnu_cxx::hash_set<SDD, sdd_hash, sdd_equal>& sdd_unicity)
    {
      const DDD* ddd = dynamic_cast<const DDD*>(s);
      if (ddd != NULL)
	{
	  if (ddd_unicity.find(*ddd) == ddd_unicity.end())
	    {
	      ddd_unicity.insert(*ddd);
	      for (DDD::const_iterator i = ddd->begin();
		   i != ddd->end();
		   ++i)
		{
		  DDD tmp(i->second);
		  count(&tmp, ddd_unicity, sdd_unicity);
		}
	    }
	  return;
	}
      const SDD* sdd = dynamic_cast<const SDD*>(s);
      //  if (sdd != NULL)
      if (sdd_unicity.find(*sdd) == sdd_unicity.end())
	{
	  sdd_unicity.insert(*sdd);
	  for (SDD::const_iterator i = sdd->begin();
	       i != sdd->end();
	       ++i)
	    {
	      SDD tmp(i->second);
	      count(&tmp, ddd_unicity, sdd_unicity);
	      count(i->first, ddd_unicity, sdd_unicity);
	    }
	}
      return;
    }

    unsigned int count(const data_set& dd)
    {
      const value_data* value = dynamic_cast<const value_data*>(dd.get());
      const ddd_data* ddd = dynamic_cast<const ddd_data*>(dd.get());
      const sdd_data* sdd = dynamic_cast<const sdd_data*>(dd.get());
      if (value != NULL)
	return 1;
      else if (ddd != NULL)
	{
	  __gnu_cxx::hash_set<DDD, ddd_hash, ddd_equal> ddd_unicity;
	  __gnu_cxx::hash_set<SDD, sdd_hash, sdd_equal> sdd_unicity;
	  count(ddd, ddd_unicity, sdd_unicity);
	  return ddd_unicity.size() + sdd_unicity.size();
	}
      else if (sdd != NULL)
	{
	  __gnu_cxx::hash_set<DDD, ddd_hash, ddd_equal> ddd_unicity;
	  __gnu_cxx::hash_set<SDD, sdd_hash, sdd_equal> sdd_unicity;
	  count(sdd, ddd_unicity, sdd_unicity);
	  return ddd_unicity.size() + sdd_unicity.size();
	}
      else
	assert(false);
    }

    long double states(const data_set& dd)
    {
      const value_data* value = dynamic_cast<const value_data*>(dd.get());
      const ddd_data* ddd = dynamic_cast<const ddd_data*>(dd.get());
      const sdd_data* sdd = dynamic_cast<const sdd_data*>(dd.get());
      if (value != NULL)
	return 1;
      else if (ddd != NULL)
	return ddd->nbStates();
      else if (sdd != NULL)
	return sdd->nbStates();
      else
	assert(false);
    }

    void dot_export(const std::string& file, const data_set& dd, const std::string& name)
    {
      std::ofstream output(file.c_str(), std::ios::trunc);
      dot_export(output, dd, name);
    }

    void dot_export(std::ostream& stream, const data_set& dd, const std::string& name)
    {
      stream << "digraph " << name << " {" << std::endl
	     << "\trankdir=LR" << std::endl;
      __gnu_cxx::hash_map<DDD, unsigned int, ddd_hash, ddd_equal> ddd_unicity;
      __gnu_cxx::hash_map<SDD, std::pair<unsigned int, unsigned int>, sdd_hash, sdd_equal> sdd_unicity;
      __gnu_cxx::hash_map<unsigned int, std::map<int, unsigned int> > ddd_successors;
      __gnu_cxx::hash_map<unsigned int, std::map<unsigned int, unsigned int> > sdd_successors;
      __gnu_cxx::hash_map<unsigned int, int> variables;
      unsigned int last = std::numeric_limits<unsigned int>::min();
      const value_data* value = dynamic_cast<const value_data*>(dd.get());
      const DDD* ddd = dynamic_cast<const DDD*>(dd.get());
      const SDD* sdd = dynamic_cast<const SDD*>(dd.get());
      if (value != NULL)
	stream << "\t" << value->value << std::endl;
      else if (ddd != NULL)
	dot_export(*ddd, last, ddd_unicity, ddd_successors, variables);
      else if (sdd != NULL)
	dot_export(*sdd, last, 0, ddd_unicity, sdd_unicity, ddd_successors, sdd_successors, variables);
      else
	assert(false);
      dot_export(stream, ddd_unicity, sdd_unicity, ddd_successors, sdd_successors, variables);
      stream << "}" << std::endl;
    }

  }
}
