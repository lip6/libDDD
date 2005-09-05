// Copyright (C) 2004, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
//
// This file is part of libDDD.
// Written by Alexandre Duret-Lutz, Alexandre.Duret-Lutz@lip6.fr

#ifndef DDD_MISC_HASHFUNC_HH
#define DDD_MISC_HASHFUNC_HH
//# include <cstddef>

namespace ddd
{
  /// \addtogroup hash_funcs Hashing functions
  /// \ingroup misc_tools
  /// @{

  /// \brief Thomas Wang's 32 bit hash function.
  ///
  /// Hash an integer amongst the integers.
  /// http://www.concentric.net/~Ttwang/tech/inthash.htm
  inline size_t
  wang32_hash(size_t key)
  {
    // We assume that size_t has at least 32bits.
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
  }

  /// \brief Knuth's Multiplicative hash function.
  ///
  /// This function is suitable for hashing values whose
  /// high order bits do not vary much (ex. addresses of
  /// memory objects).  Prefer spot::wang32_hash() otherwise.
  /// http://www.concentric.net/~Ttwang/tech/addrhash.htm
  inline size_t
  knuth32_hash(size_t key)
  {
    // 2654435761 is the golden ratio of 2^32.  The right shift of 3
    // bits assumes that all objects are aligned on a 8 byte boundary.
    return (key >> 3) * 2654435761U;
  }
  /// @}
}

#endif // DDD_MISC_HASHFUNC_HH
