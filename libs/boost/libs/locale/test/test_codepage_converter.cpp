//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/util.hpp>
#ifdef BOOST_LOCALE_WITH_ICU
#    include "../src/boost/locale/icu/codecvt.hpp"
#endif
#include "../src/boost/locale/shared/iconv_codecvt.hpp"

#include <cstring>
#include <iostream>

#include "boostLocale/test/tools.hpp"
#include "boostLocale/test/unit_test.hpp"

constexpr auto illegal = boost::locale::util::base_converter::illegal;
constexpr auto incomplete = boost::locale::util::base_converter::incomplete;

bool test_to(boost::locale::util::base_converter& cvt, const char* s, unsigned codepoint)
{
    size_t len = strlen(s);
    const char* end = s + len;
    return cvt.to_unicode(s, end) == codepoint;
}

bool test_from(boost::locale::util::base_converter& cvt, unsigned codepoint, const char* str)
{
    char buf[32] = {0};
    unsigned res = cvt.from_unicode(codepoint, buf, buf + sizeof(buf));
    if(res == boost::locale::util::base_converter::illegal) {
        return str == 0;
    } else {
        return str != 0 && strlen(str) == res && memcmp(str, buf, res) == 0;
    }
}

bool test_incomplete(boost::locale::util::base_converter& cvt, unsigned codepoint, int len)
{
    char buf[32] = {0};
    unsigned res = cvt.from_unicode(codepoint, buf, buf + len);
    return res == boost::locale::util::base_converter::incomplete;
}

#define TEST_TO(str, codepoint) TEST(test_to(*cvt, str, codepoint))
#define TEST_FROM(str, codepoint) TEST(test_from(*cvt, codepoint, str))
#define TEST_INC(codepoint, len) TEST(test_incomplete(*cvt, codepoint, len))

void test_shiftjis(std::unique_ptr<boost::locale::util::base_converter>& cvt)
{
    std::cout << "- Correct" << std::endl;
    TEST_TO("a", 'a');
    TEST_TO("X", 'X');
    TEST_TO("\xCB", 0xFF8b);     // half width katakana Hi ヒ
    TEST_TO("\x83\x71", 0x30d2); // Full width katakana Hi ヒ
    TEST_TO("\x82\xd0", 0x3072); // Full width hiragana Hi ひ

    TEST_FROM("a", 'a');
    TEST_FROM("X", 'X');
    TEST_FROM("\xCB", 0xFF8b);     // half width katakana Hi ヒ
    TEST_FROM("\x83\x71", 0x30d2); // Full width katakana Hi ヒ
    TEST_FROM("\x82\xd0", 0x3072); // Full width hiragana Hi ひ

    std::cout << "- Illegal/incomplete" << std::endl;

    TEST_TO("\xa0", illegal);
    TEST_TO("\x82", incomplete);
    TEST_TO("\x83\xf0", illegal);

    TEST_INC(0x30d2, 1); // Full width katakana Hi ヒ
    TEST_INC(0x3072, 1); // Full width hiragana Hi ひ

    TEST_FROM(0, 0x5e9); // Hebrew ש not in ShiftJIS
}

void test_main(int /*argc*/, char** /*argv*/)
{
    using namespace boost::locale::util;

    std::cout << "Test UTF-8\n";
    std::cout << "- From UTF-8" << std::endl;

    std::unique_ptr<base_converter> cvt = create_utf8_converter();

    TEST(cvt.get());
    TEST(cvt->is_thread_safe());
    TEST_EQ(cvt->max_len(), 4);

    std::cout << "-- Correct" << std::endl;

    TEST_TO("\x7f", 0x7f);
    TEST_TO("\xC2\x80", 0x80);
    TEST_TO("\xdf\xBF", 0x7FF);
    TEST_TO("\xe0\xa0\x80", 0x800);
    TEST_TO("\xef\xbf\xbf", 0xFFFF);
    TEST_TO("\xf0\x90\x80\x80", 0x10000);
    TEST_TO("\xf4\x8f\xbf\xbf", 0x10FFFF);

    std::cout << "-- Too big" << std::endl;
    TEST_TO("\xf4\x9f\x80\x80", illegal);         //  11 0000
    TEST_TO("\xfb\xbf\xbf\xbf", illegal);         // 3FF FFFF
    TEST_TO("\xf8\x90\x80\x80\x80", illegal);     // 400 0000
    TEST_TO("\xfd\xbf\xbf\xbf\xbf\xbf", illegal); // 7fff ffff

    std::cout << "-- Invalid trail" << std::endl;
    TEST_TO("\xC2\x7F", illegal);
    TEST_TO("\xdf\x7F", illegal);
    TEST_TO("\xe0\x7F\x80", illegal);
    TEST_TO("\xef\xbf\x7F", illegal);
    TEST_TO("\xe0\x7F\x80", illegal);
    TEST_TO("\xef\xbf\x7F", illegal);
    TEST_TO("\xf0\x7F\x80\x80", illegal);
    TEST_TO("\xf4\x7f\xbf\xbf", illegal);
    TEST_TO("\xf0\x90\x7F\x80", illegal);
    TEST_TO("\xf4\x8f\x7F\xbf", illegal);
    TEST_TO("\xf0\x90\x80\x7F", illegal);
    TEST_TO("\xf4\x8f\xbf\x7F", illegal);

    std::cout << "-- Invalid length" << std::endl;

    /// Test that this actually works
    TEST_TO(make2(0x80), 0x80);
    TEST_TO(make2(0x7ff), 0x7ff);

    TEST_TO(make3(0x800), 0x800);
    TEST_TO(make3(0xffff), 0xffff);

    TEST_TO(make4(0x10000), 0x10000);
    TEST_TO(make4(0x10ffff), 0x10ffff);

    TEST_TO(make4(0x110000), illegal);
    TEST_TO(make4(0x1fffff), illegal);

    TEST_TO(make2(0), illegal);
    TEST_TO(make3(0), illegal);
    TEST_TO(make4(0), illegal);
    TEST_TO(make2(0x7f), illegal);
    TEST_TO(make3(0x7f), illegal);
    TEST_TO(make4(0x7f), illegal);

    TEST_TO(make3(0x80), illegal);
    TEST_TO(make4(0x80), illegal);
    TEST_TO(make3(0x7ff), illegal);
    TEST_TO(make4(0x7ff), illegal);

    TEST_TO(make4(0x8000), illegal);
    TEST_TO(make4(0xffff), illegal);

    std::cout << "-- Invalid surrogate" << std::endl;

    TEST_TO(make3(0xD800), illegal);
    TEST_TO(make3(0xDBFF), illegal);
    TEST_TO(make3(0xDC00), illegal);
    TEST_TO(make3(0xDFFF), illegal);

    TEST_TO(make4(0xD800), illegal);
    TEST_TO(make4(0xDBFF), illegal);
    TEST_TO(make4(0xDC00), illegal);
    TEST_TO(make4(0xDFFF), illegal);

    std::cout << "-- Incomplete" << std::endl;

    TEST_TO("\x80", illegal);
    TEST_TO("\xC2", incomplete);

    TEST_TO("\xdf", incomplete);

    TEST_TO("\xe0", incomplete);
    TEST_TO("\xe0\xa0", incomplete);

    TEST_TO("\xef\xbf", incomplete);
    TEST_TO("\xef", incomplete);

    TEST_TO("\xf0\x90\x80", incomplete);
    TEST_TO("\xf0\x90", incomplete);
    TEST_TO("\xf0", incomplete);

    TEST_TO("\xf4\x8f\xbf", incomplete);
    TEST_TO("\xf4\x8f", incomplete);
    TEST_TO("\xf4", incomplete);

    std::cout << "- To UTF-8\n";

    std::cout << "-- Test correct" << std::endl;

    TEST_FROM("\x7f", 0x7f);
    TEST_FROM("\xC2\x80", 0x80);
    TEST_FROM("\xdf\xBF", 0x7FF);
    TEST_INC(0x7FF, 1);
    TEST_FROM("\xe0\xa0\x80", 0x800);
    TEST_INC(0x800, 2);
    TEST_INC(0x800, 1);
    TEST_FROM("\xef\xbf\xbf", 0xFFFF);
    TEST_INC(0x10000, 3);
    TEST_INC(0x10000, 2);
    TEST_INC(0x10000, 1);
    TEST_FROM("\xf0\x90\x80\x80", 0x10000);
    TEST_FROM("\xf4\x8f\xbf\xbf", 0x10FFFF);

    std::cout << "-- Test no surrogate " << std::endl;

    TEST_FROM(0, 0xD800);
    TEST_FROM(0, 0xDBFF);
    TEST_FROM(0, 0xDC00);
    TEST_FROM(0, 0xDFFF);

    std::cout << "-- Test invalid " << std::endl;

    TEST_FROM(0, 0x110000);
    TEST_FROM(0, 0x1FFFFF);

    std::cout << "Test windows-1255" << std::endl;

    cvt = create_simple_converter("windows-1255");

    TEST(cvt.get());
    TEST(cvt->is_thread_safe());
    TEST_EQ(cvt->max_len(), 1);

    std::cout << "- From 1255" << std::endl;

    TEST_TO("\xa4", 0x20aa);
    TEST_TO("\xe0", 0x05d0);
    TEST_TO("\xc4", 0x5b4);
    TEST_TO("\xfb", illegal);
    TEST_TO("\xdd", illegal);
    TEST_TO("\xff", illegal);
    TEST_TO("\xfe", 0x200f);

    std::cout << "- To 1255" << std::endl;

    TEST_FROM("\xa4", 0x20aa);
    TEST_FROM("\xe0", 0x05d0);
    TEST_FROM("\xc4", 0x5b4);
    TEST_FROM("\xfe", 0x200f);

    TEST_FROM(0, 0xe4);
    TEST_FROM(0, 0xd0);

#ifdef BOOST_LOCALE_WITH_ICU
    std::cout << "Testing Shift-JIS using ICU/uconv" << std::endl;

    cvt = boost::locale::impl_icu::create_uconv_converter("Shift-JIS");
    TEST_REQUIRE(cvt);
    test_shiftjis(cvt);
#endif

    std::cout << "Testing Shift-JIS using POSIX/iconv" << std::endl;

    cvt = boost::locale::create_iconv_converter("Shift-JIS");
#ifndef BOOST_LOCALE_WITH_ICONV
    TEST(!cvt);
#endif
    if(cvt)
        test_shiftjis(cvt);
    else {
#ifdef BOOST_LOCALE_WITH_ICONV
        std::cout << "- Shift-JIS is not supported!" << std::endl;
#endif
    }
}

// boostinspect:noascii
