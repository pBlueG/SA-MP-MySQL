#pragma once

#include <string>
using std::string;


template<typename T>
bool ConvertStrToData(const char *src, T &dest);

template<typename T>
bool ConvertDataToStr(T src, string &dest);
