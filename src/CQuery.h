#pragma once

#include <string>
#include <functional>

using std::string;
using std::function;

#include "Types.h"


class CQuery
{
public: //constructor / deconstructor
	CQuery(string &&query) :
		m_Query(query)
	{ }
	~CQuery() = default;

private: //variables
	string m_Query;
	function<void(ResultSet_t result)> m_Callback;
	ResultSet_t m_Result = nullptr;

public: //functions
	bool Execute(MYSQL *connection);
	inline void OnExecutionFinished(decltype(m_Callback) &&cb)
	{
		m_Callback = cb;
	}
	inline void CallCallback()
	{
		if (m_Callback)
			m_Callback(m_Result);
	}

public: //factory function
	static inline Query_t Create(string &&query)
	{
		return std::make_shared<CQuery>(std::move(query));
	}

};
