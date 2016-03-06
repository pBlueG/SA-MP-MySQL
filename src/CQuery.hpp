#pragma once

#include <string>
#include <functional>

using std::string;
using std::function;

#include "CLog.hpp"
#include "types.hpp"
#include "mysql.hpp"


class CQuery
{
public: //constructor / deconstructor
	CQuery(string &&query) :
		m_Query(query),
		m_DbgInfo(CDebugInfoManager::Get()->GetCurrentInfo())
	{ }
	~CQuery() = default;

private: //variables
	string m_Query;
	function<void(ResultSet_t result)> m_Callback;
	function<void(unsigned int, string)> m_ErrorCallback;
	ResultSet_t m_Result = nullptr;
	const DebugInfo m_DbgInfo;

public: //functions
	bool Execute(MYSQL *connection);
	ResultSet_t GetResult()
	{
		return m_Result;
	}

	inline void OnExecutionFinished(decltype(m_Callback) &&cb)
	{
		m_Callback = std::move(cb);
	}
	inline void CallCallback()
	{
		if (m_Callback)
			m_Callback(m_Result);
	}

	inline void OnError(decltype(m_ErrorCallback) &&cb)
	{
		m_ErrorCallback = std::move(cb);
	}
	inline void CallErrorCallback(unsigned int errorid, string error)
	{
		if (m_ErrorCallback)
			m_ErrorCallback(errorid, std::move(error));
	}

public: //factory function
	static inline Query_t Create(string query)
	{
		return std::make_shared<CQuery>(std::move(query));
	}

};
