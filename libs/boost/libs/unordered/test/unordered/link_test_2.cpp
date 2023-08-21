
// Copyright 2006-2009 Daniel James.
// Copyright 2022-2023 Christian Mazakas.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/unordered.hpp"

#ifdef BOOST_UNORDERED_FOA_TESTS
void foo(boost::unordered_flat_set<int>& x1,
  boost::unordered_flat_map<int, int>& x2, boost::unordered_node_set<int>& x3,
  boost::unordered_node_map<int, int>& x4)
{
#if BOOST_WORKAROUND(BOOST_CODEGEARC, BOOST_TESTED_AT(0x0613))
  struct dummy
  {
    boost::unordered_flat_set<int> x1;
    boost::unordered_flat_map<int, int> x2;
  };
#endif

  x1.insert(1);
  x2[2] = 2;
  x3.insert(3);
  x4.insert(std::make_pair(4, 5));
}
#else
void foo(boost::unordered_set<int>& x1, boost::unordered_map<int, int>& x2,
  boost::unordered_multiset<int>& x3, boost::unordered_multimap<int, int>& x4)
{
#if BOOST_WORKAROUND(BOOST_CODEGEARC, BOOST_TESTED_AT(0x0613))
  struct dummy
  {
    boost::unordered_set<int> x1;
    boost::unordered_map<int, int> x2;
    boost::unordered_multiset<int> x3;
    boost::unordered_multimap<int, int> x4;
  };
#endif

  x1.insert(1);
  x2[2] = 2;
  x3.insert(3);
  x4.insert(std::make_pair(4, 5));
}
#endif
