//
// Copyright (c) 2015 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/utf8_codecvt.hpp>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <locale>
#include <memory.h>
#include <wchar.h>
#define BOOST_LOCALE_ERROR_LIMIT -1
#include "boostLocale/test/unit_test.hpp"

#if defined(BOOST_MSVC) && BOOST_MSVC < 1700
#    pragma warning(disable : 4428) // universal-character-name encountered in source
#endif

static const char* utf8_name =
  "\xf0\x9d\x92\x9e-\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82-\xE3\x82\x84\xE3\x81\x82.txt";
static const wchar_t* wide_name = L"\U0001D49E-\u043F\u0440\u0438\u0432\u0435\u0442-\u3084\u3042.txt";

const char* res(std::codecvt_base::result r)
{
    switch(r) {
        case std::codecvt_base::ok: return "ok";
        case std::codecvt_base::partial: return "partial";
        case std::codecvt_base::error: return "error";
        case std::codecvt_base::noconv: return "noconv";
        default: return "error";
    }
}

typedef std::codecvt<wchar_t, char, std::mbstate_t> cvt_type;

void test_codecvt_in_n_m(const cvt_type& cvt, int n, int m)
{
    const wchar_t* wptr = wide_name;
    const size_t wlen = wcslen(wide_name);
    const size_t u8len = strlen(utf8_name);
    const char* from = utf8_name;
    const char* end = from;
    const char* real_end = utf8_name + u8len;
    const char* from_next = from;
    std::mbstate_t mb = std::mbstate_t();
    while(from_next < real_end) {
        if(from == end) {
            end = from + n;
            if(end > real_end)
                end = real_end;
        }

        wchar_t buf[128];
        wchar_t* to = buf;
        wchar_t* to_end = to + m;
        wchar_t* to_next = to;

        std::mbstate_t mb2 = mb;
        std::codecvt_base::result r = cvt.in(mb, from, end, from_next, to, to_end, to_next);

        int count = cvt.length(mb2, from, end, to_end - to);
#ifndef BOOST_LOCALE_DO_LENGTH_MBSTATE_CONST
        TEST_EQ(memcmp(&mb, &mb2, sizeof(mb)), 0);
        if(count != from_next - from) {
            std::cout << count << " " << from_next - from << std::endl;
        }
        TEST_EQ(count, from_next - from);
#else
        TEST_EQ(count, to_next - to);
#endif

        if(r == cvt_type::partial) {
            end += n;
            if(end > real_end)
                end = real_end;
        } else
            TEST_EQ(r, cvt_type::ok);
        while(to != to_next) {
            TEST_EQ(*wptr, *to);
            wptr++;
            to++;
        }
        to = to_next;
        from = from_next;
    }
    TEST(wptr == wide_name + wlen);
    TEST(from == real_end);
}

void test_codecvt_out_n_m(const cvt_type& cvt, int n, int m)
{
    const char* nptr = utf8_name;
    const size_t wlen = wcslen(wide_name);
    const size_t u8len = strlen(utf8_name);

    std::mbstate_t mb = std::mbstate_t();

    const wchar_t* from_next = wide_name;
    const wchar_t* real_from_end = wide_name + wlen;

    char buf[256];
    char* to = buf;
    char* to_next = to;
    char* to_end = to + n;
    char* real_to_end = buf + sizeof(buf);

    while(from_next < real_from_end) {
        const wchar_t* from = from_next;
        const wchar_t* from_end = from + m;
        if(from_end > real_from_end)
            from_end = real_from_end;
        if(to_end == to) {
            to_end = to + n;
        }

        std::codecvt_base::result r = cvt.out(mb, from, from_end, from_next, to, to_end, to_next);
        if(r == cvt_type::partial) {
            // If those are equal, then "partial" probably means: Need more input
            // Otherwise "Need more output"
            if(from_next != from_end) {
                TEST_LT(to_end - to_next, cvt.max_length());
                to_end += n;
                if(to_end > real_to_end)
                    to_end = real_to_end;
            }
        } else {
            TEST_EQ(r, cvt_type::ok);
        }

        while(to != to_next) {
            TEST_EQ(*nptr, *to);
            nptr++;
            to++;
        }
        from = from_next;
    }
    TEST(nptr == utf8_name + u8len);
    TEST(from_next == real_from_end);
    TEST_EQ(cvt.unshift(mb, to, to + n, to_next), cvt_type::ok);
    TEST(to_next == to);
}

void test_codecvt_conv()
{
    std::cout << "Conversions " << std::endl;
    std::locale l(std::locale::classic(), new boost::locale::utf8_codecvt<wchar_t>());

    const cvt_type& cvt = std::use_facet<cvt_type>(l);

    TEST_EQ(cvt.max_length(), 4);

    for(int i = 1; i <= (int)strlen(utf8_name) + 1; i++) {
        for(int j = 1; j <= (int)wcslen(wide_name) + 1; j++) {
            try {
                test_codecvt_in_n_m(cvt, i, j);
                test_codecvt_out_n_m(cvt, i, j);
            } catch(...) {
                std::cerr << "Wlen=" << j << " Nlen=" << i << std::endl;
                throw;
            }
        }
    }
}

void test_codecvt_err()
{
    std::cout << "Errors " << std::endl;
    std::locale l(std::locale::classic(), new boost::locale::utf8_codecvt<wchar_t>());

    const cvt_type& cvt = std::use_facet<cvt_type>(l);

    std::cout << "- UTF-8" << std::endl;
    {
        wchar_t buf[2];
        wchar_t* to = buf;
        wchar_t* to_end = buf + 2;
        wchar_t* to_next = to;
        const char* err_utf = "1\xFF\xFF";
        {
            std::mbstate_t mb = std::mbstate_t();
            const char* from = err_utf;
            const char* from_end = from + strlen(from);
            const char* from_next = from;
            to_next = to;
            TEST_EQ(cvt.in(mb, from, from_end, from_next, to, to_end, to_next), cvt_type::error);
            TEST(from_next == from + 1);
            TEST(to_next == to + 1);
            TEST_EQ(*to, '1');
        }
        err_utf++;
        {
            std::mbstate_t mb = std::mbstate_t();
            const char* from = err_utf;
            const char* from_end = from + strlen(from);
            const char* from_next = from;
            TEST_EQ(cvt.in(mb, from, from_end, from_next, to, to_end, to_next), cvt_type::error);
            TEST(from_next == from);
            TEST(to_next == to);
        }
    }
    std::cout << "- Trailing UTF-16 surrogate" << std::endl;
    {
        char buf[4] = {};
        char* const to = buf;
        char* const to_end = buf + 4;
        char* to_next = to;
        const wchar_t* err_utf = L"\xD800"; // Trailing UTF-16 surrogate
        std::mbstate_t mb = std::mbstate_t();
        const wchar_t* from = err_utf;
        const wchar_t* from_end = from + 1;
        const wchar_t* from_next = from;
        cvt_type::result res = cvt.out(mb, from, from_end, from_next, to, to_end, to_next);
        BOOST_LOCALE_START_CONST_CONDITION
        if(sizeof(wchar_t) == 2) {
            BOOST_LOCALE_END_CONST_CONDITION
            TEST(res == cvt_type::partial);
            TEST(from_next == from_end);
            TEST(to_next == to);
            TEST(buf[0] == 0);
        } else {
            // surrogate is invalid
            TEST(res == cvt_type::error);
            TEST(from_next == from);
            TEST(to_next == to);
        }
    }

    std::cout << "- UTF-16/32" << std::endl;
    {
        char buf[32];
        char* to = buf;
        char* to_end = buf + 32;
        char* to_next = to;
        wchar_t err_buf[3] = {'1', 0xDC9E, 0}; // second value is invalid for UTF-16 and 32
        const wchar_t* err_utf = err_buf;
        {
            std::mbstate_t mb = std::mbstate_t();
            const wchar_t* from = err_utf;
            const wchar_t* from_end = from + wcslen(from);
            const wchar_t* from_next = from;
            TEST_EQ(cvt.out(mb, from, from_end, from_next, to, to_end, to_next), cvt_type::error);
            TEST(from_next == from + 1);
            TEST(to_next == to + 1);
            TEST_EQ(*to, '1');
        }
        err_utf++;
        {
            std::mbstate_t mb = std::mbstate_t();
            const wchar_t* from = err_utf;
            const wchar_t* from_end = from + wcslen(from);
            const wchar_t* from_next = from;
            to_next = to;
            TEST_EQ(cvt.out(mb, from, from_end, from_next, to, to_end, to_next), cvt_type::error);
            TEST(from_next == from);
            TEST(to_next == to);
        }
    }
}

void test_char_char()
{
    std::cout << "Char-char specialization" << std::endl;
    std::locale l(std::locale::classic(), new boost::locale::utf8_codecvt<char>());
    const std::codecvt<char, char, std::mbstate_t>& cvt = std::use_facet<std::codecvt<char, char, std::mbstate_t>>(l);
    std::mbstate_t mb = std::mbstate_t();
    const char* from = "a";
    const char* from_end = from + 1;
    const char* from_next = from;
    char buf[2];
    char* to = buf;
    char* to_end = buf + 1;
    char* to_next = to;
    TEST(cvt.always_noconv());
    TEST_EQ(cvt.in(mb, from, from_end, from_next, to, to_end, to_next), cvt_type::noconv);
    TEST(from_next == from);
    TEST(to_next == to);
    TEST_EQ(cvt.out(mb, from, from_end, from_next, to, to_end, to_next), cvt_type::noconv);
    TEST(from_next == from);
    TEST(to_next == to);
    TEST_EQ(cvt.encoding(), 1);
    TEST_EQ(cvt.max_length(), 1);
}

void test_main(int /*argc*/, char** /*argv*/)
{
    test_codecvt_conv();
    test_codecvt_err();
    test_char_char();
}
///
