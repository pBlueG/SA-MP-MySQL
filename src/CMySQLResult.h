#pragma once
#ifndef INC_CMYSQLRESULT_H
#define INC_CMYSQLRESULT_H


#include <vector>
#include <string>

using std::vector;
using std::string;

#ifdef WIN32
	#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"


class CMySQLResult 
{
public:
	friend class CMySQLQuery;
	
	CMySQLResult();
	~CMySQLResult();

	inline my_ulonglong GetRowCount() const 
	{
		return m_Rows;
	}

	inline unsigned int GetFieldCount() const 
	{
		return m_Fields;
	}

	void GetFieldName(unsigned int idx, char **dest);
	void GetRowData(unsigned int row, unsigned int fieldidx, char **dest);
	void GetRowDataByName(unsigned int row, const char *field, char **dest);


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

private:
	unsigned int m_Fields;
	my_ulonglong m_Rows;

	vector< vector<string> > m_Data;
	vector<string> m_FieldNames;

	my_ulonglong 
		m_InsertID, 
		m_AffectedRows;

	unsigned int m_WarningCount;
};


#endif // INC_CMYSQLRESULT_H
