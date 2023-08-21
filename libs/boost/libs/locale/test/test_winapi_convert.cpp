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

template<typename CharType>
void test_one(const std::locale& l, std::string src, std::string tgtl, std::string tgtu)
{
    TEST_EQ(boost::locale::to_upper(to_correct_string<CharType>(src, l), l), to_correct_string<CharType>(tgtu, l));
    TEST_EQ(boost::locale::to_lower(to_correct_string<CharType>(src, l), l), to_correct_string<CharType>(tgtl, l));
    TEST_EQ(boost::locale::fold_case(to_correct_string<CharType>(src, l), l), to_correct_string<CharType>(tgtl, l));
}

template<typename CharType>
void test_char()
{
    boost::locale::generator gen;

    std::cout << "- Testing at least C" << std::endl;

    std::locale l = gen("C.UTF-8");

    test_one<CharType>(l, "Hello World i", "hello world i", "HELLO WORLD I");

    std::string name = "en_US.UTF-8";

    std::cout << "- Testing " << name << std::endl;
    l = gen(name);
    test_one<CharType>(l, "Façade", "façade", "FAÇADE");

    name = "tr_TR.UTF-8";
    std::cout << "Testing " << name << std::endl;
    test_one<CharType>(gen(name), "i", "i", "İ");
}

template<typename Char>
void test_normc(std::basic_string<Char> orig, std::basic_string<Char> normal, boost::locale::norm_type type)
{
    std::locale l = boost::locale::generator().generate("en_US.UTF-8");
    TEST_EQ(boost::locale::normalize(orig, type, l), normal);
    TEST_EQ(boost::locale::normalize(orig.c_str(), type, l), normal);
    TEST_EQ(boost::locale::normalize(orig.c_str(), orig.c_str() + orig.size(), type, l), normal);
}

void test_norm(std::string orig, std::string normal, boost::locale::norm_type type)
{
    test_normc<char>(orig, normal, type);
    test_normc<wchar_t>(to<wchar_t>(orig), to<wchar_t>(normal), type);
}

void test_main(int /*argc*/, char** /*argv*/)
{
#ifdef BOOST_LOCALE_NO_WINAPI_BACKEND
    std::cout << "WinAPI Backend is not build... Skipping\n";
    return;
#endif
    boost::locale::localization_backend_manager mgr = boost::locale::localization_backend_manager::global();
    mgr.select("winapi");
    boost::locale::localization_backend_manager::global(mgr);

    std::cout << "Testing char" << std::endl;
    test_char<char>();
    std::cout << "Testing wchar_t" << std::endl;
    test_char<wchar_t>();

    std::cout << "Testing Unicode normalization" << std::endl;
    test_norm("\xEF\xAC\x81", "\xEF\xAC\x81", boost::locale::norm_nfd); /// ligature fi
    test_norm("\xEF\xAC\x81", "\xEF\xAC\x81", boost::locale::norm_nfc);
    test_norm("\xEF\xAC\x81", "fi", boost::locale::norm_nfkd);
    test_norm("\xEF\xAC\x81", "fi", boost::locale::norm_nfkc);
    test_norm("ä", "ä", boost::locale::norm_nfd); // ä to a and accent
    test_norm("ä", "ä", boost::locale::norm_nfc);
}

// boostinspect:noascii
