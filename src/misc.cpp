#pragma warning (disable: 4244) //conversion from `long` to `float`, possible loss of data

#include "misc.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <type_traits>

using namespace boost::spirit;


template<typename T>
bool ConvertStrToData(const string &src, T &dest)
{
	return qi::parse(src.begin(), src.end(),
		typename std::conditional<
			std::is_floating_point<T>::value, 
				qi::real_parser<T>, 
				qi::int_parser<T>
		>::type(), 
		dest);
}

template bool ConvertStrToData(const string &src, int &dest);
template bool ConvertStrToData(const string &src, unsigned int &dest);
template bool ConvertStrToData(const string &src, short &dest);
template bool ConvertStrToData(const string &src, unsigned short &dest);
template bool ConvertStrToData(const string &src, char &dest);
template bool ConvertStrToData(const string &src, unsigned char &dest);
template bool ConvertStrToData(const string &src, long long &dest);
template bool ConvertStrToData(const string &src, unsigned long long &dest);
template bool ConvertStrToData(const string &src, float &dest);
template bool ConvertStrToData(const string &src, double &dest);



template<typename T>
bool ConvertDataToStr(T src, string &dest)
{
	return karma::generate(std::back_inserter(dest), 
		typename std::conditional<
			std::is_floating_point<T>::value,
				karma::real_generator<T>,
				std::conditional<
					std::is_signed<T>::value,
						karma::int_generator<T>, 
						karma::uint_generator<T>
				>::type
		>::type(),
		src);
}
template bool ConvertDataToStr(int src, string &dest);
template bool ConvertDataToStr(unsigned int src, string &dest);
template bool ConvertDataToStr(short src, string &dest);
template bool ConvertDataToStr(unsigned short src, string &dest);
template bool ConvertDataToStr(char src, string &dest);
template bool ConvertDataToStr(unsigned char src, string &dest);
template bool ConvertDataToStr(long long src, string &dest);
template bool ConvertDataToStr(unsigned long long src, string &dest);
template bool ConvertDataToStr(float src, string &dest);
template bool ConvertDataToStr(double src, string &dest);