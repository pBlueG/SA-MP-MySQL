#pragma once

#include <string>

using std::string;

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

	CResult *m_Result = nullptr;


private: //functions
	CResult *StoreResult(MYSQL *connection, MYSQL_RES *raw_result);


public:
	bool Execute(MYSQL *connection);

	const CResult *GetResult() const
	{
		return m_Result;
	}


public: //factory function
	static CQuery *Create(string &&query)
	{
		return new CQuery(std::move(query));
	}

};
