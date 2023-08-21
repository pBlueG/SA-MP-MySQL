//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale.hpp>
#include <ctime>
#include <iostream>
#include <set>
#include <string>

using namespace boost::locale;

int main(int argc, char** argv)
{
    if(argc != 2) {
        std::cerr << "Usage backend locale\n";
        return 1;
    }
    boost::locale::localization_backend_manager mgr = boost::locale::localization_backend_manager::global();
    mgr.select(argv[1]);
    generator gen(mgr);
    std::locale::global(gen(argv[2]));
    /// Set global locale to requested

    for(int i = 0; i < 100000; i++) {
        std::ostringstream ss;
        for(int j = 0; j < 5; j++) {
            ss << boost::locale::as::datetime << std::time(0) << " " << boost::locale::as::number << 13456.345 << "\n";
        }
        if(i == 0)
            std::cout << ss.str() << std::endl;
    }
}
