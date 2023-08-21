//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale.hpp>
#include <clocale>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <vector>
#ifndef BOOST_LOCALE_NO_WINAPI_BACKEND
#    include "../src/boost/locale/win32/lcid.hpp"
#endif
#include <boost/core/ignore_unused.hpp>

#ifdef BOOST_LOCALE_WITH_ICU
#    include <unicode/uversion.h>
#endif

#include "boostLocale/test/tools.hpp"
#include "boostLocale/test/unit_test.hpp"

const char* env(const char* s)
{
#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4996)
#endif
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    const char* r = getenv(s);
#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#ifdef _MSC_VER
#    pragma warning(pop)
#endif
    if(r)
        return r;
    return "";
}

bool has_win_locale(const std::string& locale_name)
{
#ifdef BOOST_LOCALE_NO_WINAPI_BACKEND
    boost::ignore_unused(locale_name);
    return false;
#else
    return boost::locale::impl_win::locale_to_lcid(locale_name) != 0;
#endif
}

void check_locales(const std::vector<const char*>& names)
{
    std::cout << std::setw(32) << "locale" << std::setw(7) << "POSIX" << std::setw(5) << "STD" << std::setw(5) << "WIN"
              << std::endl;
    for(const char* name : names) {
        std::cout << std::setw(32) << name;
        std::cout << std::setw(7) << (has_posix_locale(name) ? "Yes" : "No");
        std::cout << std::setw(5) << (has_std_locale(name) ? "Yes" : "No");
        std::cout << std::setw(5) << (has_win_locale(name) ? "Yes" : "No");
        std::cout << std::endl;
    }
}

void test_main(int /*argc*/, char** /*argv*/)
{
    std::cout << "- Backends: ";
#ifdef BOOST_LOCALE_WITH_ICU
    std::cout << "icu:" << U_ICU_VERSION << " ";
#endif
#ifndef BOOST_LOCALE_NO_STD_BACKEND
    std::cout << "std ";
#endif
#ifndef BOOST_LOCALE_NO_POSIX_BACKEND
    std::cout << "posix ";
#endif
#ifndef BOOST_LOCALE_NO_WINAPI_BACKEND
    std::cout << "winapi";
#endif
    std::cout << std::endl;
#ifdef BOOST_LOCALE_WITH_ICONV
    std::cout << "- With iconv\n";
#else
    std::cout << "- Without iconv\n";
#endif
    std::cout << "- Environment \n";
    std::cout << "  LANG=" << env("LANG") << std::endl;
    std::cout << "  LC_ALL=" << env("LC_ALL") << std::endl;
    std::cout << "  LC_CTYPE=" << env("LC_CTYPE") << std::endl;
    std::cout << "  TZ=" << env("TZ") << std::endl;

    const char* clocale = setlocale(LC_ALL, "");
    if(!clocale)
        clocale = "undetected"; // LCOV_EXCL_LINE
    std::cout << "- C locale: " << clocale << std::endl;

    try {
        std::locale loc("");
#if defined(BOOST_CLANG) && BOOST_CLANG_VERSION < 30800
        std::cout << "- C++ locale: n/a on Clang < 3.8\n";
#else
        std::cout << "- C++ locale: " << loc.name() << std::endl;
#endif
    } catch(const std::exception&) {
        std::cout << "- C++ locale: is not supported\n"; // LCOV_EXCL_LINE
    }

    const std::vector<const char*> locales_to_check = {
      "en_US.UTF-8",
      "en_US.ISO8859-1",
      "English_United States.1252",
      "he_IL.UTF-8",
      "he_IL.ISO8859-8",
      "Hebrew_Israel.1255",
      "ru_RU.UTF-8",
      "Russian_Russia.1251",
      "tr_TR.UTF-8",
      "Turkish_Turkey.1254",
      "ja_JP.UTF-8",
      "ja_JP.SJIS",
      "Japanese_Japan.932",
      "en_001.UTF-8",
      "en_150.UTF-8",
    };
    std::cout << "- Testing locales availability on the operation system:" << std::endl;
    check_locales(locales_to_check);
    std::cout << "--- Testing Japanese_Japan.932 is working: ";
    std::cout << (test_std_supports_SJIS_codecvt("Japanese_Japan.932") ? "Yes" : "No") << std::endl;

    std::cout << "- Testing timezone and time " << std::endl;
    {
        setlocale(LC_ALL, "C");
        time_t now = time(0);
        char buf[1024];
        strftime(buf, sizeof(buf), "%%c=%c; %%Z=%Z; %%z=%z", localtime_wrap(&now));
        std::cout << "  Local Time    :" << buf << std::endl;
        strftime(buf, sizeof(buf), "%%c=%c; %%Z=%Z; %%z=%z", gmtime_wrap(&now));
        std::cout << "  Universal Time:" << buf << std::endl;
    }
    std::cout << "- Boost.Locale's locale: ";
    boost::locale::generator gen;
    std::locale l = gen("");
    std::cout << (std::has_facet<boost::locale::info>(l) ? std::use_facet<boost::locale::info>(l).name() : "Unknown")
              << std::endl;
}

// boostinspect:noascii
