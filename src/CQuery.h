#pragma once

#include <string>
#include <functional>
#include <memory>

using std::string;
using std::function;
using std::shared_ptr;

class CResult;
typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;


class CQuery
{
public: //type definitions
	using Type_t = shared_ptr<CQuery>;

public: //constructor / deconstructor
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
	static inline CQuery::Type_t Create(string &&query)
	{
		return std::make_shared<CQuery>(std::move(query));
	}

};
