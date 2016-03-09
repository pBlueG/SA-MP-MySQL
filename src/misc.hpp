#pragma once

#include <string>
#include <type_traits>

#pragma warning (push)
#pragma warning (disable: 4244 4018 4348)

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>

using std::string;
namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;


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

template<typename T>
bool ConvertStrToData(const char *src, T &dest)
{
	return src != nullptr && qi::parse(src, src + strlen(src),
		typename std::conditional<
			std::is_floating_point<T>::value,
				qi::real_parser<T>,
				qi::int_parser<T>
		>::type(),
		dest);
}


template<typename T, unsigned int B = 10U>
bool ConvertDataToStr(T src, string &dest)
{
	return karma::generate(std::back_inserter(dest), 
		typename std::conditional<
			std::is_floating_point<T>::value,
				karma::real_generator<T>,
				typename std::conditional<
					std::is_signed<T>::value,
						karma::int_generator<T, B>, 
						karma::uint_generator<T, B>
				>::type
		>::type(),
		src);
}

template<> //bool specialization
inline bool ConvertStrToData(const string &src, bool &dest)
{
	return qi::parse(src.begin(), src.end(), qi::bool_, dest);
}

template<> //bool specialization
inline bool ConvertDataToStr(bool src, string &dest)
{
	return karma::generate(std::back_inserter(dest), karma::bool_, src);
}


#pragma warning (pop)
