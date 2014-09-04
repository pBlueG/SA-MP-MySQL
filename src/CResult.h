#pragma once

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;


class CResult 
{
	friend class CQuery;
public: //constructor / destructor
	CResult() = default;
	~CResult()
	{
		if (m_Data != nullptr)
			free(m_Data);
	}


private: //variables
	unsigned int m_Fields = 0;
	my_ulonglong m_Rows = 0;

	char ***m_Data = nullptr;
	vector<string> m_FieldNames;

	my_ulonglong
		m_InsertId = 0,
		m_AffectedRows = 0;

	unsigned int m_WarningCount = 0;


public: //functions
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


	inline my_ulonglong InsertId() const 
	{
		return m_InsertId;
	}

	inline my_ulonglong AffectedRows() const 
	{
		return m_AffectedRows;
	}

	inline unsigned int WarningCount() const 
	{
		return m_WarningCount;
	}

};
