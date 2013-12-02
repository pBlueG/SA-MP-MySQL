#pragma once

#include "misc.h"

#include <cstring>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>

using namespace boost::spirit;


bool ConvertStrToInt(const char *src, int &dest) 
{
	if (src == NULL)
		return false;

	const char 
		*first_iter = src,
		*last_iter = first_iter+strlen(src);

	return qi::parse(first_iter, last_iter, qi::int_, dest);
}

bool ConvertStrToFloat(const char *src, float &dest) 
{
	if (src == NULL)
		return false;

	const char 
		*first_iter(src),
		*last_iter(first_iter+strlen(src));

	return qi::parse(first_iter, last_iter, qi::double_, dest);
}


template<unsigned int B> //B = base/radix
bool ConvertIntToStr(int src, char *dest) 
{
	const bool ReturnVal = karma::generate(dest, karma::int_generator<int, B>(), src);
	*dest = 0;
	return ReturnVal;
}
//instantiate templates
template bool ConvertIntToStr<16>(int src, char *dest);
template bool ConvertIntToStr<10>(int src, char *dest);
template bool ConvertIntToStr<2>(int src, char *dest);

bool ConvertIntToStr(int src, char *dest) 
{
	if (dest == NULL)
		return false;

	const bool success = karma::generate(dest, karma::int_generator<int>(), src);
	*dest = 0;
	return success;
}

bool ConvertFloatToStr(float src, char *dest) 
{
	if (dest == NULL)
		return false;

	const bool success = karma::generate(dest, double_, src);
	*dest = 0;
	return success;
}


void amx_SetCString(AMX* amx, cell param, const char *str, int len) 
{
	cell *dest = NULL;
	amx_GetAddr(amx, param, &dest);
	amx_SetString(dest, str, 0, 0, (len > 0) ? len : (strlen(str)+1));
}