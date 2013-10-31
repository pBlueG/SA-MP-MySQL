
#ifndef BOOST_MPL_IS_PLACEHOLDER_HPP_INCLUDED
#define BOOST_MPL_IS_PLACEHOLDER_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: is_placeholder.hpp 86245 2013-10-11 23:17:48Z skelly $
// $Date: 2013-10-12 01:17:48 +0200 (Sa, 12. Okt 2013) $
// $Revision: 86245 $

#include <boost/mpl/arg_fwd.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/aux_/yes_no.hpp>
#include <boost/mpl/aux_/type_wrapper.hpp>
#include <boost/mpl/aux_/config/ctps.hpp>
#include <boost/mpl/aux_/config/static_constant.hpp>

namespace boost { namespace mpl {


template< typename T >
struct is_placeholder
    : bool_<false>
{
};

template< int N >
struct is_placeholder< arg<N> >
    : bool_<true>
{
};


}}

#endif // BOOST_MPL_IS_PLACEHOLDER_HPP_INCLUDED
