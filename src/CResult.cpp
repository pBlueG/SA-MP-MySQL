#include "CResult.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


CResult *CResult::Create(MYSQL *connection)
{
	if (connection == nullptr)
		return nullptr;

	//TODO: mysql_error
	return new CResult(mysql_store_result(connection), 
		mysql_insert_id(connection),
		mysql_affected_rows(connection),
		mysql_warning_count(connection));
}

CResult::~CResult()
{
	if (m_Result != nullptr)
		mysql_free_result(m_Result);
}


my_ulonglong CResult::GetRowCount() const
{
	return mysql_num_rows(m_Result);
}

unsigned int CResult::GetFieldCount() const
{
	return mysql_num_fields(m_Result);
}

const string CResult::GetFieldName(unsigned int idx)
{
	auto it = m_FieldNames.left.find(idx);
	if (it == m_FieldNames.left.end())
	{
		string field_name(mysql_fetch_field_direct(m_Result, idx)->name);
		m_FieldNames.insert(bimap<unsigned int, string>::value_type(idx, field_name));
		return field_name;
	}
	else
		return it->second;
}

const string CResult::GetRowData(unsigned int row_idx, unsigned int field_idx)
{
	if (row_idx < GetRowCount() && field_idx < GetFieldCount())
	{
		auto it_row = m_RowData.find(row_idx);
		if (it_row == m_RowData.end())
		{
			if ((row_idx - m_LastRowPos) != 1)
				mysql_data_seek(m_Result, row_idx);
			it_row = m_RowData.emplace(row_idx, mysql_fetch_row(m_Result)).first;
			m_LastRowPos = row_idx;
		}
		return it_row->second[field_idx];
	}
	return string();
	//return (row < m_Rows && fieldidx < m_Fields) ? m_Data[row][fieldidx] : nullptr;
}

const string CResult::GetRowDataByName(unsigned int row_idx, const string &field)
{
	return string();
}
