#pragma once

#include <string>

using std::string;


template<typename T>
class CError
{
	using ErrorType = typename T::Error;
public:
	CError() :
		m_Type(ErrorType::NONE)
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

private:
	ErrorType m_Type;
	string m_Message;
};