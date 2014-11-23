#pragma once

#include "CSingleton.h"

#include <string>
#include <unordered_map>
#include <boost/bimap.hpp>

using std::string;
using std::unordered_map;
using boost::bimap;

typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;
typedef unsigned long long my_ulonglong;
typedef char** MYSQL_ROW;


class CResult 
{
	friend class CQuery;
public: //constructor / destructor
	CResult(MYSQL_RES *result, my_ulonglong insert_id, my_ulonglong aff_rows, 
		unsigned int warning_count) :
		m_Result(result), m_InsertId(insert_id), m_AffectedRows(aff_rows),
		m_WarningCount(warning_count)
	{ }
	~CResult();

private: //variables
	MYSQL_RES *m_Result = nullptr;

	my_ulonglong
		m_InsertId = 0,
		m_AffectedRows = 0;
	unsigned int m_WarningCount = 0;

	unsigned int m_Fields = 0;
	my_ulonglong m_Rows = 0;

	bimap<unsigned int, string> m_FieldNames;

	//<Row_Index, Row>
	unordered_map<unsigned int, MYSQL_ROW> m_RowData;
	unsigned int m_LastRowPos = 0;

public: //functions
	my_ulonglong GetRowCount() const;
	unsigned int GetFieldCount() const;

	const string GetFieldName(unsigned int idx);
	const string GetRowData(unsigned int row_idx, unsigned int field_idx);
	const string GetRowDataByName(unsigned int row_idx, const string &field);


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

public: //factory function
	static CResult *Create(MYSQL *connection);

};

class CResultManager : public CSingleton<CResultManager>
{
	friend class CSingleton<CResultManager>;
private:
	CResultManager() = default;
	~CResultManager() = default;

private:
	CResult *m_ActiveResult;

public:
	inline void SetActiveResult(CResult *result)
	{
		m_ActiveResult = result;
	}
	inline CResult *GetActiveResult()
	{
		return m_ActiveResult;
	}

};
