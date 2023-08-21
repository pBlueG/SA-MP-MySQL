//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/encoding.hpp>
#include <boost/locale/formatting.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/localization_backend.hpp>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "boostLocale/test/tools.hpp"
#include "boostLocale/test/unit_test.hpp"

template<typename CharType, typename RefCharType>
void test_by_char(const std::locale& l, const std::locale& lreal)
{
    typedef std::basic_stringstream<CharType> ss_type;
    typedef std::basic_stringstream<RefCharType> ss_ref_type;

    using namespace boost::locale;

    {
        std::cout << "- Testing as::posix" << std::endl;
        ss_type ss;
        ss.imbue(l);

        TEST(ss << 1045.45);
        double n;
        TEST(ss >> n);
        TEST_EQ(n, 1045.45);
        TEST_EQ(ss.str(), ascii_to<CharType>("1045.45"));
    }

    {
        std::cout << "- Testing as::number" << std::endl;
        ss_type ss;
        ss.imbue(l);

        TEST(ss << as::number);
        TEST(ss << 1045.45);
        double n;
        TEST(ss >> n);
        TEST_EQ(n, 1045.45);

        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);

        TEST(ss_ref << 1045.45);

        TEST_EQ(to_utf8(ss.str()), to_utf8(ss_ref.str()));
    }

    {
        std::cout << "- Testing as::currency national " << std::endl;

        bool bad_parsing = false;
        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);
        ss_ref << std::showbase;
        std::use_facet<std::money_put<RefCharType>>(lreal).put(ss_ref, false, ss_ref, RefCharType(' '), 104334);
        { // workaround MSVC library issues
            std::ios_base::iostate err = std::ios_base::iostate();
            typename std::money_get<RefCharType>::iter_type end;
            long double tmp;
            std::use_facet<std::money_get<RefCharType>>(lreal).get(ss_ref, end, false, ss_ref, err, tmp);
            if(err & std::ios_base::failbit) {
                std::cout << "-- Looks like standard library does not support parsing well" << std::endl;
                bad_parsing = true;
            }
        }

        ss_type ss;
        ss.imbue(l);

        TEST(ss << as::currency);
        TEST(ss << 1043.34);
        if(!bad_parsing) {
            double v1;
            TEST(ss >> v1);
            TEST_EQ(v1, 1043.34);
        }

        TEST_EQ(to_utf8(ss.str()), to_utf8(ss_ref.str()));
    }

    {
        std::cout << "- Testing as::currency iso" << std::endl;
        ss_type ss;
        ss.imbue(l);

        ss << as::currency << as::currency_iso;
        TEST(ss << 1043.34);
        double v1;
        TEST(ss >> v1);
        TEST_EQ(v1, 1043.34);

        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);
        ss_ref << std::showbase;
        std::use_facet<std::money_put<RefCharType>>(lreal).put(ss_ref, true, ss_ref, RefCharType(' '), 104334);

        TEST_EQ(to_utf8(ss.str()), to_utf8(ss_ref.str()));
    }

    {
        std::cout << "- Testing as::date/time" << std::endl;
        ss_type ss;
        ss.imbue(l);

        time_t a_date = 3600 * 24 * (31 + 4); // Feb 5th
        time_t a_time = 3600 * 15 + 60 * 33;  // 15:33:05
        time_t a_timesec = 13;
        time_t a_datetime = a_date + a_time + a_timesec;

        ss << as::time_zone("GMT");

        ss << as::date << a_datetime << CharType('\n');
        ss << as::time << a_datetime << CharType('\n');
        ss << as::datetime << a_datetime << CharType('\n');
        ss << as::time_zone("GMT+01:00");
        ss << as::ftime(ascii_to<CharType>("%H")) << a_datetime << CharType('\n');
        ss << as::time_zone("GMT+00:15");
        ss << as::ftime(ascii_to<CharType>("%M")) << a_datetime << CharType('\n');

        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);

        std::basic_string<RefCharType> rfmt(ascii_to<RefCharType>("%x\n%X\n%c\n16\n48\n"));

        std::tm tm = *gmtime_wrap(&a_datetime);

        std::use_facet<std::time_put<RefCharType>>(lreal)
          .put(ss_ref, ss_ref, RefCharType(' '), &tm, rfmt.c_str(), rfmt.c_str() + rfmt.size());
        if(ss_ref.str() == rfmt)
            std::cout << "-- Standard library failed to format the datetime value" << std::endl;
        else
            TEST_EQ(to_utf8(ss.str()), to_utf8(ss_ref.str()));
    }
}

void test_main(int /*argc*/, char** /*argv*/)
{
#ifdef BOOST_LOCALE_NO_STD_BACKEND
    std::cout << "STD Backend is not build... Skipping\n";
    return;
#endif

    boost::locale::localization_backend_manager mgr = boost::locale::localization_backend_manager::global();
    mgr.select("std");
    boost::locale::localization_backend_manager::global(mgr);
    boost::locale::generator gen;
    for(const std::string lName : {"en_US.UTF-8", "en_US.ISO8859-1", "he_IL.UTF-8", "he_IL.ISO8859-8"}) {
        std::cout << lName << " locale" << std::endl;
        std::string real_name;
        std::string name = get_std_name(lName, &real_name);
        if(name.empty()) {
            std::cout << lName << " not supported" << std::endl;
        } else {
            std::cout << "\tstd name: " << name << std::endl;
            std::locale l1 = gen(name);
            std::cout << "\treal name: " << real_name << std::endl;
            std::locale l2(real_name.c_str());
            if(lName.find(".UTF-8") != std::string::npos) {
                std::cout << "\tUTF-8" << std::endl;
                if(name == real_name)
                    test_by_char<char, char>(l1, l2);
                else
                    test_by_char<char, wchar_t>(l1, l2);
            } else {
                std::cout << "\tchar" << std::endl;
                test_by_char<char, char>(l1, l2);
            }

            std::cout << "\tWide UTF-" << sizeof(wchar_t) * 8 << std::endl;
            test_by_char<wchar_t, wchar_t>(l1, l2);

#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
            std::cout << "\tchar16 UTF-16" << std::endl;
            test_by_char<char16_t, char16_t>(l1, l2);
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
            std::cout << "\tchar32 UTF-32" << std::endl;
            test_by_char<char32_t, char32_t>(l1, l2);
#endif
        }
    }
    {
        std::cout << "Testing UTF-8 punct workaround" << std::endl;
        std::string real_name;
        std::string name = get_std_name("ru_RU.UTF-8", &real_name);
        if(name.empty()) {
            std::cout << "- No Russian locale" << std::endl;
        } else if(name != real_name) {
            std::cout << "- Not having UTF-8 locale, no need for workaround" << std::endl;
        } else {
            std::locale l1 = gen(name), l2(real_name.c_str());
            bool fails = false;
            try {
                std::ostringstream ss;
                ss.imbue(l2);
                ss << 12345.45;
                boost::locale::conv::from_utf<char>(ss.str(), "windows-1251", boost::locale::conv::stop);
                fails = false;
            } catch(...) {
                fails = true;
            }

            if(!fails) {
                std::cout << "- No invalid UTF. No need to check" << std::endl;
            } else {
                std::ostringstream ss;
                ss.imbue(l1);
                ss << std::setprecision(10);
                ss << boost::locale::as::number << 12345.45;
                TEST(ss.str() == "12 345,45" || ss.str() == "12345,45");
            }
        }
    }
}

// boostinspect:noascii
