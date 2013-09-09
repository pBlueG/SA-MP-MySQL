#pragma once
#ifndef INC_MISC_H
#define INC_MISC_H


#include "main.h"

bool ConvertStrToInt(const char *src, int &dest);
bool ConvertStrToFloat(const char *src, float &dest);

template<unsigned int B> //B = base/radix
bool ConvertIntToStr(int src, char *dest);
bool ConvertIntToStr(int src, char *dest); //no-template version
bool ConvertFloatToStr(float src, char *dest);


void amx_SetCString(AMX* amx, cell param, const char *str, int len = 0);


#endif // INC_MISC_H
