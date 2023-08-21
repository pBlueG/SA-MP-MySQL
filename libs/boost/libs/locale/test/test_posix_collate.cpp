//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/conversion.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/locale/localization_backend.hpp>
#include "boostLocale/test/tools.hpp"
#include "boostLocale/test/unit_test.hpp"
#include <iomanip>
#include <iostream>

int get_sign(int x)
{
    if(x < 0)
        return -1;
    else if(x == 0)
        return 0;
    return 1;
}

template<typename CharType>
void test_one(const std::locale& l, std::string ia, std::string ib, int diff)
{
    std::basic_string<CharType> a = to_correct_string<CharType>(ia, l);
    std::basic_string<CharType> b = to_correct_string<CharType>(ib, l);
    if(diff < 0) {
        TEST(l(a, b));
        TEST(!l(b, a));
    } else if(diff == 0) {
        TEST(!l(a, b));
        TEST(!l(b, a));
    } else {
        TEST(!l(a, b));
        TEST(l(b, a));
    }

    const std::collate<CharType>& col = std::use_facet<std::collate<CharType>>(l);

    TEST_EQ(diff, col.compare(a.c_str(), a.c_str() + a.size(), b.c_str(), b.c_str() + b.size()));
    TEST_EQ(
      diff,
      get_sign(col.transform(a.c_str(), a.c_str() + a.size()).compare(col.transform(b.c_str(), b.c_str() + b.size()))));
    if(diff == 0) {
        TEST_EQ(col.hash(a.c_str(), a.c_str() + a.size()), col.hash(b.c_str(), b.c_str() + b.size()));
    }
}

template<typename CharType>
void test_char()
{
    boost::locale::generator gen;

    std::cout << "- Testing at least C" << std::endl;
    std::locale l = gen("C.UTF-8");
    test_one<CharType>(l, "a", "b", -1);
    test_one<CharType>(l, "a", "a", 0);

#if !defined(__APPLE__) && !defined(__FreeBSD__)
    for(const std::string locale_name : {"en_US.UTF-8", "en_US.ISO8859-1"}) {
        if(!has_posix_locale(locale_name))
            std::cout << "- " << locale_name << " not supported, skipping" << std::endl;
        else {
            std::cout << "- Testing " << locale_name << std::endl;
            l = gen(locale_name);
            test_one<CharType>(l, "a", "ç", -1);
            test_one<CharType>(l, "ç", "d", -1);
        }
    }
#else
    std::cout << "- Collation is broken on this OS C standard library, skipping\n";
#endif
}

BOOST_LOCALE_DISABLE_UNREACHABLE_CODE_WARNING
void test_main(int /*argc*/, char** /*argv*/)
{
#ifdef BOOST_LOCALE_NO_POSIX_BACKEND
    std::cout << "POSIX Backend is not build... Skipping\n";
    return;
#endif

    boost::locale::localization_backend_manager mgr = boost::locale::localization_backend_manager::global();
    mgr.select("posix");
    boost::locale::localization_backend_manager::global(mgr);

    std::cout << "Testing char" << std::endl;
    test_char<char>();
    std::cout << "Testing wchar_t" << std::endl;
    test_char<wchar_t>();
}

// boostinspect:noascii
