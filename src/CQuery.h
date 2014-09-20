#pragma once

#include <string>
#include <functional>

using std::string;
using std::function;

class CResult;
typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;


class CQuery
{
private: //constructor / deconstructor
	CQuery(string &&query) :
		m_Query(query)
	{ }
	~CQuery() = default;


private: //variables
	string m_Query;
	function<void(const CResult *result)> m_Callback;
	CResult *m_Result = nullptr;


private: //functions
	CResult *StoreResult(MYSQL *connection, MYSQL_RES *raw_result);


public:
	bool Execute(MYSQL *connection);
	inline void OnExecutionFinished(decltype(m_Callback) && cb)
	{
		m_Callback = cb;
	}


public: //factory function
	static CQuery *Create(string &&query)
	{
		return new CQuery(std::move(query));
	}

};
