#pragma once

#include <string>
#include <functional>

using std::string;
using std::function;

#include "CLog.hpp"
#include "types.hpp"
#include "ISqlStatement.hpp"



class CQuery : public ISqlStatement
{
public: //constructor / deconstructor
	CQuery(string &&query) :
		m_Query(query),
		m_DbgInfo(CDebugInfoManager::Get()->GetCurrentInfo())
	{ }
	~CQuery() = default;

private: //variables
	string m_Query;
	const DebugInfo m_DbgInfo;

public: //functions
	bool Execute(MYSQL *connection);
	ResultSet_t GetResult()
	{
		return m_Result;
	}

	

public: //factory function
	static inline Query_t Create(string query)
	{
		return std::make_shared<CQuery>(std::move(query));
	}

};
