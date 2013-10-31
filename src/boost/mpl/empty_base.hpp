
#ifndef BOOST_MPL_EMPTY_BASE_HPP_INCLUDED
#define BOOST_MPL_EMPTY_BASE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: empty_base.hpp 85956 2013-09-26 13:05:50Z skelly $
// $Date: 2013-09-26 15:05:50 +0200 (Do, 26. Sep 2013) $
// $Revision: 85956 $

#include <boost/mpl/bool.hpp>
#include <boost/mpl/aux_/config/msvc.hpp>
#include <boost/mpl/aux_/config/workaround.hpp>

#include <boost/type_traits/is_empty.hpp>

// should be always the last #include directive
#include <boost/type_traits/detail/bool_trait_def.hpp>

namespace boost { namespace mpl {

// empty base class, guaranteed to have no members; inheritance from
// 'empty_base' through the 'inherit' metafunction is a no-op - see 
// "mpl/inherit.hpp> header for the details
struct empty_base {};

template< typename T >
struct is_empty_base
    : false_
{
};

template<>
struct is_empty_base<empty_base>
    : true_
{
};

}}

namespace boost {
BOOST_TT_AUX_BOOL_TRAIT_SPEC1(is_empty, mpl::empty_base, true)
}

#include <boost/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_MPL_EMPTY_BASE_HPP_INCLUDED
