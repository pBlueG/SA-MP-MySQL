#pragma once
#ifndef INC_CMYSQLRESULT_H
#define INC_CMYSQLRESULT_H


#include <vector>
#include <string>

using std::vector;
using std::string;

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif
#include "CLog.h"


class CMySQLResult 
{
public:
	friend class CMySQLQuery;


	inline my_ulonglong GetRowCount() const 
	{
		return m_Rows;
	}

	inline unsigned int GetFieldCount() const 
	{
		return m_Fields;
	}

	const char *GetFieldName(unsigned int idx);
	const char *GetRowData(unsigned int row, unsigned int fieldidx);
	const char *GetRowDataByName(unsigned int row, const char *field);


	inline my_ulonglong InsertID() const 
	{
		return m_InsertID;
	}

	inline my_ulonglong AffectedRows() const 
	{
		return m_AffectedRows;
	}

	inline unsigned int WarningCount() const 
	{
		return m_WarningCount;
	}


	inline unsigned int GetQueryExecutionTime(unsigned int unit) const
	{
		return m_ExecTime[unit];
	}

	inline const string &GetQueryString() const
	{
		return m_Query;
	}


	CMySQLResult();
	~CMySQLResult();
private:

	unsigned int m_Fields;
	my_ulonglong m_Rows;

	char ***m_Data;
	vector<string> m_FieldNames;

	my_ulonglong 
		m_InsertID, 
		m_AffectedRows;

	unsigned int m_WarningCount;

	string m_Query;
	unsigned int m_ExecTime[2];
};

enum E_EXECTIME_UNIT
{
	UNIT_MILLISECONDS,
	UNIT_MICROSECONDS
};


#endif // INC_CMYSQLRESULT_H
