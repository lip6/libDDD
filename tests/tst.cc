#include <iostream>
#include <cassert>
#include <fstream>
#include <stdlib.h>
#include "dot_representation.hh"

using namespace std;

#include "DDD.h"
#include "DED.h"

char* vn[]= {"T", "0", "1", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
  "M", "N", "O", "P", "Q", "R", "S", "U", "V", "W", "X", "Y", "Z"};


void initName()
{
  for (int i = -1; i < 26; ++i)
    DDD::varName(i, vn[i + 1]);
}

class eof
{
public:
  eof() { }
};

const char* next_tok(const char* cstr)
{
  if (! *cstr)
    throw eof();
  cstr++;
  while ((*cstr == ' ' || *cstr == '\t') && *cstr)
    cstr++;
  return cstr;
}


DDD* parse_rec (const char *cstr)
{
  DDD*          ddd = 0;
  DDD*          ddd_rec = 0;
  char          node;
  char          val;

  while ((*cstr == ' ' || *cstr == '\t') && *cstr)
    cstr++;
  node = *cstr;
  if (node < 'A' || node > 'Z')
    exit(4);
  node = node - 'A' + 2;
  cstr = next_tok(cstr);
  val = *cstr;
  if (val < '0' || val > '9')
    exit(4);
  val -= '0';
  cstr = next_tok(cstr);
  if (*cstr != ',' && *cstr)
    exit(1);
  try
  {
    cstr = next_tok(cstr);
    ddd_rec = parse_rec(cstr);
  }
  catch (eof)
  {
    ;
  }
  if (ddd_rec)
    ddd = new DDD(node, val, *ddd_rec);
  else
    ddd = new DDD(node, val);
  return ddd;
}


DDD* parse_file (std::string file)
{
  DDD*          ddd = 0;
  DDD*          ddd_tmp = 0;
  std::string   str;
  std::ifstream fi;
  
  fi.open(file.c_str());
  if (fi.is_open() != true)
    exit(2);
  while (fi.eof() == false)
  {
    std::getline(fi, str);
    if (fi.eof() == true)
      return ddd;
    if (str == "[1]")
        ddd_tmp = new DDD(DDD::one);
    else
      if (str == "[0]")
        ddd_tmp = new DDD(DDD::null);
      else
        if ("[T]" == str)
        ddd_tmp = new DDD(DDD::top);
        else 
          ddd_tmp = parse_rec(str.c_str());
    assert(ddd_tmp != 0);
    if (ddd)
      *ddd = *ddd + *ddd_tmp;
    else
      ddd = ddd_tmp;
  }
  return ddd;
}

char* remove_slash(char *str)
{
  char* ret = (char *)malloc (strlen(str));
  unsigned int   i, j;

  for (i = strlen(str) - 1; i >= 0; --i)
  {
    if (str[i] == '/')
      break;
  }
  ++i;
  for (j = 0; i < strlen(str); ++i, ++j)
  {
    ret[j] = str[i];
  }
  ret[j] = 0;
  return ret;
}


int main(int argc, char **argv){
  // Define a name for each variable
  initName();
  if (argc < 4)
    exit(3);
    
  DDD*          ddd1;
  DDD*          ddd2;
  DDD           ddd_res;

  ddd1 = parse_file(argv[1]);
  ddd2 = parse_file(argv[2]);
  if (!ddd1 || !ddd2)
    return 1;
//  export of tests into dot format
  xxx::dot::dot_export(std::string(argv[1]) + ".dot", xxx::data_set(new xxx::ddd_data(*ddd1)), std::string("ddd1"));
  xxx::dot::dot_export(std::string(argv[2]) + ".dot", xxx::data_set(new xxx::ddd_data(*ddd2)), std::string("ddd2"));
//  commented lines are to show only wether ddd-making is ok or not
//  cout << *ddd1 << endl;
//  cout << *ddd2 << endl;
  if (argv[3][0] == '+')
    cout << (ddd_res = (*ddd1 + *ddd2));
  if (argv[3][0] == '*')
    cout << (ddd_res = (*ddd1 * *ddd2));
  if (argv[3][0] == '^')
    cout << (ddd_res = (*ddd1 ^ *ddd2));
  if (argv[3][0] == '-')
    cout << (ddd_res = (*ddd1 - *ddd2));
//  export of tests into dot format
  xxx::dot::dot_export(std::string(argv[1]) + argv[3] + remove_slash(argv[2]) + ".dot", xxx::data_set(new xxx::ddd_data(ddd_res)) , std::string("ddd_res"));
  return 0;
}
