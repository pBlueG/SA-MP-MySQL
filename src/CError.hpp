#pragma once

#include <string>
#include <fmt/format.h>

using std::string;


template<typename T>
class CError
{
	using ErrorType = typename T::Error;
public:
	CError() :
		m_Type(ErrorType::NONE)
	{ }
	CError(ErrorType type, string &&msg) :
		m_Type(type),
		m_Message(std::move(msg))
	{ }
	template<typename... Args>
	CError(ErrorType type, string &&format, Args &&...args) :
		m_Type(type),
		m_Message(fmt::format(format, std::forward<Args>(args)...))
	{ }
	~CError() = default;

	operator bool() const
	{
		return m_Type != ErrorType::NONE;
	}

	const string &msg() const
	{
		return m_Message;
	}
	const ErrorType type() const
	{
		return m_Type;
	}
	const string &module() const
	{
		return T::ModuleName;
	}

	void set(ErrorType type, string &&msg)
	{
		m_Type = type;
		m_Message.assign(std::move(msg));
	}
	template<typename... Args>
	void set(ErrorType type, string &&format, Args &&...args)
	{
		m_Type = type;
		m_Message.assign(fmt::format(format, std::forward<Args>(args)...));
	}

private:
	ErrorType m_Type;
	string m_Message;
};
