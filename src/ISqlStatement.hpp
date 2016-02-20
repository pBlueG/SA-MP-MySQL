#pragma once

#include <string>
#include <functional>

using std::string;
using std::function;

#include "types.hpp"
#include "mysql.hpp"


class ISqlStatement
{
public:
	using Callback_t = function<void(ResultSet_t result)>;
	using ErrorCallback_t = function<void(unsigned int, string)>;

public:
	virtual bool Execute(MYSQL *connection) = 0;

	inline void OnExecutionFinished(Callback_t &&cb)
	{
		m_Callback = std::move(cb);
	}
	inline void CallCallback()
	{
		if (m_Callback)
			m_Callback(m_Result);
	}

	inline void OnError(ErrorCallback_t &&cb)
	{
		m_ErrorCallback = std::move(cb);
	}
	inline void CallErrorCallback(unsigned int errorid, string error)
	{
		if (m_ErrorCallback)
			m_ErrorCallback(errorid, std::move(error));
	}

private:
	Callback_t m_Callback;
	ErrorCallback_t m_ErrorCallback;

protected:
	ResultSet_t m_Result = nullptr;

};
