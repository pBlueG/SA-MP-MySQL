//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

// Test that header file is self-contained.
#include <boost/url/segments_view.hpp>

#include <boost/url/parse.hpp>
#include <boost/url/parse_path.hpp>
#include <boost/url/url.hpp>
#include <boost/static_assert.hpp>
#include <boost/core/ignore_unused.hpp>

#include "test_suite.hpp"

#include <sstream>

#ifdef assert
#undef assert
#endif
#define assert BOOST_TEST

namespace boost {
namespace urls {

BOOST_STATIC_ASSERT(
    std::is_default_constructible<
        segments_view>::value);

BOOST_STATIC_ASSERT(
    std::is_copy_constructible<
        segments_view>::value);

BOOST_STATIC_ASSERT(
    std::is_copy_assignable<
        segments_view>::value);

BOOST_STATIC_ASSERT(
    std::is_default_constructible<
        segments_view::iterator>::value);

struct segments_view_test
{
    void
    testSpecialMembers()
    {
        // segments_view()
        {
            segments_view ps;
            BOOST_TEST(ps.empty());
            BOOST_TEST(! ps.is_absolute());
            BOOST_TEST_EQ(ps.buffer(), "");
            BOOST_TEST_EQ(ps.size(), 0);
        }

        // segments_view(segments_view)
        {
            segments_view ps0 =
                parse_path("/path/to/file.txt").value();
            segments_view ps1(ps0);
            BOOST_TEST_EQ(
                ps0.buffer().data(),
                ps1.buffer().data());
        }

        // segments_view(string_view)
        {
            try
            {
                string_view s = "/path/to/file.txt";
                segments_view ps(s);
                BOOST_TEST_PASS();
                BOOST_TEST_EQ(
                    ps.buffer().data(), s.data());
                BOOST_TEST_EQ(ps.buffer(), s);
            }
            catch(std::exception const&)
            {
                BOOST_TEST_FAIL();
            }

            // reserved character
            BOOST_TEST_THROWS(segments_view("?"), system_error);

            // invalid percent-escape
            BOOST_TEST_THROWS(segments_view("%"), system_error);
            BOOST_TEST_THROWS(segments_view("%F"), system_error);
            BOOST_TEST_THROWS(segments_view("%FX"), system_error);
            BOOST_TEST_THROWS(segments_view("%%"), system_error);
            BOOST_TEST_THROWS(segments_view("FA%"), system_error);
        }

        // operator=(segments_view)
        {
            segments_view ps0("/path/to/file.txt");
            segments_view ps1("/index.htm");
            ps0 = ps1;
            BOOST_TEST_EQ(
                ps0.buffer().data(),
                ps1.buffer().data());
        }

        // ostream
        {
            segments_view ps = parse_path(
                "/path/to/file.txt").value();
            std::stringstream ss;
            ss << ps;
            BOOST_TEST_EQ(ss.str(),
                "/path/to/file.txt");
        }
    }

    void
    testJavadocs()
    {
        // {class}
        {
    url_view u( "/path/to/file.txt" );

    segments_view ps = u.segments();

    assert( ps.buffer().data() == u.buffer().data() );

    ignore_unused(ps);
        }
    }

    void
    run()
    {
        testSpecialMembers();
        testJavadocs();
    }
};

TEST_SUITE(
    segments_view_test,
    "boost.url.segments_view");

} // urls
} // boost
