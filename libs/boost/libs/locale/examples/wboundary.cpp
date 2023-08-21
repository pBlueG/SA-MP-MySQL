//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

//
// ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
//
// BIG FAT WARNING FOR Microsoft Visual Studio Users
//
// YOU NEED TO CONVERT THIS SOURCE FILE ENCODING TO UTF-8 WITH BOM ENCODING.
//
// Unfortunately MSVC understands that the source code is encoded as
// UTF-8 only if you add useless BOM in the beginning.
//
// So, before you compile "wide" examples with MSVC, please convert them to text
// files with BOM. There are two very simple ways to do it:
//
// 1. Open file with Notepad and save it from there. It would convert
//    it to file with BOM.
// 2. In Visual Studio go File->Advances Save Options... and select
//    Unicode (UTF-8  with signature) Codepage 65001
//
// Note: once converted to UTF-8 with BOM, this source code would not
// compile with other compilers, because no-one uses BOM with UTF-8 today
// because it is absolutely meaningless in context of UTF-8.
//
// ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
//
#include <boost/locale.hpp>
#include <ctime>
#include <iostream>

int main()
{
    using namespace boost::locale;

    // Create system default locale
    generator gen;
    std::locale loc = gen("");
    // We need the boundary facet, currently only available via ICU
    if(!std::has_facet<boundary::boundary_indexing<wchar_t>>(loc)) {
        std::cout << "boundary detection not implemented in this environment\n";
        return 0;
    }
    std::locale::global(loc);
    std::wcout.imbue(loc);

    // This is needed to prevent the C stdio library from
    // converting strings to narrow on some platforms
    std::ios_base::sync_with_stdio(false);

    std::wstring text = L"Hello World! あにま! Linux2.6 and Windows7 is word and number. שָלוֹם עוֹלָם!";

    std::wcout << text << std::endl;

    boundary::wssegment_index index(boundary::word, text.begin(), text.end());
    boundary::wssegment_index::iterator p, e;

    for(p = index.begin(), e = index.end(); p != e; ++p) {
        std::wcout << L"Part [" << *p << L"] has ";
        if(p->rule() & boundary::word_number)
            std::wcout << L"number(s) ";
        if(p->rule() & boundary::word_letter)
            std::wcout << L"letter(s) ";
        if(p->rule() & boundary::word_kana)
            std::wcout << L"kana character(s) ";
        if(p->rule() & boundary::word_ideo)
            std::wcout << L"ideographic character(s) ";
        if(p->rule() & boundary::word_none)
            std::wcout << L"no word characters";
        std::wcout << std::endl;
    }

    index.map(boundary::character, text.begin(), text.end());

    for(p = index.begin(), e = index.end(); p != e; ++p) {
        std::wcout << L"|" << *p;
    }
    std::wcout << L"|\n\n";

    index.map(boundary::line, text.begin(), text.end());

    for(p = index.begin(), e = index.end(); p != e; ++p) {
        std::wcout << L"|" << *p;
    }
    std::wcout << L"|\n\n";

    index.map(boundary::sentence, text.begin(), text.end());

    for(p = index.begin(), e = index.end(); p != e; ++p) {
        std::wcout << L"|" << *p;
    }
    std::wcout << "|\n\n";
}

// boostinspect:noascii
