//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/collator.hpp>
#include <boost/locale/generator.hpp>
#include "boostLocale/test/tools.hpp"
#include "boostLocale/test/unit_test.hpp"
#include <iomanip>
#include <iostream>

template<typename Char>
void test_comp(std::locale l, std::basic_string<Char> left, std::basic_string<Char> right, int ilevel, int expected)
{
    typedef std::basic_string<Char> string_type;
    boost::locale::collate_level level = static_cast<boost::locale::collate_level>(ilevel);
    TEST_EQ(boost::locale::comparator<Char>(l, level)(left, right), expected < 0);
    if(ilevel == 4) {
        const std::collate<Char>& coll = std::use_facet<std::collate<Char>>(l);
        string_type lt = coll.transform(left.c_str(), left.c_str() + left.size());
        string_type rt = coll.transform(right.c_str(), right.c_str() + right.size());
        if(expected < 0)
            TEST_LT(lt, rt);
        else if(expected == 0) {
            TEST_EQ(lt, rt);
        } else
            TEST_GT(lt, rt);
        long lh = coll.hash(left.c_str(), left.c_str() + left.size());
        long rh = coll.hash(right.c_str(), right.c_str() + right.size());
        if(expected == 0)
            TEST_EQ(lh, rh);
        else
            TEST_NE(lh, rh);
    }
    const boost::locale::collator<Char>& coll = std::use_facet<boost::locale::collator<Char>>(l);
    string_type lt = coll.transform(level, left.c_str(), left.c_str() + left.size());
    TEST_EQ(lt, coll.transform(level, left));
    string_type rt = coll.transform(level, right.c_str(), right.c_str() + right.size());
    TEST_EQ(rt, coll.transform(level, right));
    if(expected < 0)
        TEST_LT(lt, rt);
    else if(expected == 0)
        TEST_EQ(lt, rt);
    else
        TEST_GT(lt, rt);
    long lh = coll.hash(level, left.c_str(), left.c_str() + left.size());
    TEST_EQ(lh, coll.hash(level, left));
    long rh = coll.hash(level, right.c_str(), right.c_str() + right.size());
    TEST_EQ(rh, coll.hash(level, right));
    if(expected == 0)
        TEST_EQ(lh, rh);
    else
        TEST_NE(lh, rh);
}

#define TEST_COMP(c, _l, _r) test_comp<c>(l, _l, _r, level, expected)

void compare(std::string left, std::string right, int level, int expected)
{
    boost::locale::generator gen;
    std::locale l = gen("en_US.UTF-8");
    if(level == 4)
        TEST_EQ(l(left, right), (expected < 0));
    TEST_COMP(char, left, right);
    TEST_COMP(wchar_t, to<wchar_t>(left), to<wchar_t>(right));
#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
    TEST_COMP(char16_t, to<char16_t>(left), to<char16_t>(right));
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
    TEST_COMP(char32_t, to<char32_t>(left), to<char32_t>(right));
#endif
    l = gen("en_US.ISO8859-1");
    if(level == 4)
        TEST_EQ(l(to<char>(left), to<char>(right)), (expected < 0));
    TEST_COMP(char, to<char>(left), to<char>(right));
}

void test_collate()
{
    int primary = 0, secondary = 1, tertiary = 2, quaternary = 3, identical = 4;
    int le = -1, gt = 1, eq = 0;

    compare("a", "A", primary, eq);
    compare("a", "A", secondary, eq);
    compare("A", "a", tertiary, gt);
    compare("a", "A", tertiary, le);
    compare("a", "A", quaternary, le);
    compare("A", "a", quaternary, gt);
    compare("a", "A", identical, le);
    compare("A", "a", identical, gt);
    compare("a", "ä", primary, eq);    //  a , ä
    compare("a", "ä", secondary, le);  //  a , ä
    compare("ä", "a", secondary, gt);  //  a , ä
    compare("a", "ä", quaternary, le); //  a , ä
    compare("ä", "a", quaternary, gt); //  a , ä
    compare("a", "ä", identical, le);  //  a , ä
    compare("ä", "a", identical, gt);  //  a , ä
}

BOOST_LOCALE_DISABLE_UNREACHABLE_CODE_WARNING
void test_main(int /*argc*/, char** /*argv*/)
{
#ifndef BOOST_LOCALE_WITH_ICU
    std::cout << "ICU is not build... Skipping\n";
    return;
#endif
    test_collate();
}

// boostinspect:noascii
