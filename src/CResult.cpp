#include "CResult.h"

#include <cstring>

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


CResult::~CResult()
{
	if (m_Data != nullptr)
		free(m_Data);
}

bool CResult::GetFieldName(unsigned int idx, string &dest) const
{
	if (idx < m_Fields)
	{
		dest = m_FieldNames.at(idx);
		return true;
	}
	return false;
}

bool CResult::GetRowData(unsigned int row, unsigned int fieldidx, string &dest) const
{
	if (row < m_Rows && fieldidx < m_Fields)
	{
		dest = m_Data[row][fieldidx];
		return true;
	}
	return false;
}

bool CResult::GetRowDataByName(unsigned int row, const string &field, string &dest) const
{
	if(row >= m_Rows || m_Fields == 0)
		return false;
	
	if (field.empty())
		return false;
	

	for (unsigned int i = 0; i < m_Fields; ++i)
	{
		if (m_FieldNames.at(i).compare(field) == 0)
		{
			dest = m_Data[row][i];
			return true;
		}
	}
	
	return false;
}

CResultSet::~CResultSet()
{
	for (auto *r : m_Results)
		delete r;
}

CResultSet::Type_t CResultSet::Create(MYSQL *connection)
{
	if (connection == nullptr)
		return nullptr;


	CResultSet *resultset = nullptr;
	MYSQL_RES *raw_result = mysql_store_result(connection);

	if (raw_result == nullptr)
	{
		if (mysql_field_count(connection) == 0) //query is non-SELECT query
		{
			resultset = new CResultSet;

			resultset->m_WarningCount = mysql_warning_count(connection);
			resultset->m_AffectedRows = mysql_affected_rows(connection);
			resultset->m_InsertId = mysql_insert_id(connection);
		}
		else
		{
			//TODO: mysql_errno
		}
	}
	else
	{
		resultset = new CResultSet;
		resultset->m_WarningCount = mysql_warning_count(connection);

		do
		{
			CResult *result = new CResult;

			resultset->m_Results.push_back(result);

			MYSQL_FIELD *mysql_field;
			MYSQL_ROW mysql_row;

			const my_ulonglong num_rows = result->m_Rows = mysql_num_rows(raw_result);
			const unsigned int num_fields = result->m_Fields = mysql_num_fields(raw_result);

			result->m_FieldNames.reserve(num_fields + 1);


			size_t row_data_size = 0;
			while (mysql_field = mysql_fetch_field(raw_result))
			{
				result->m_FieldNames.push_back(mysql_field->name);
				row_data_size += mysql_field->max_length + 1;
			}


			size_t
				mem_head_size = sizeof(char **) * static_cast<size_t>(num_rows),
				mem_row_size = (sizeof(char *) * (num_fields + 1)) + ((row_data_size)* sizeof(char));
			//+1 because there is another value in memory pointing to somewhere
			//mem_row_size has to be a multiple of 8
			while (mem_row_size % 8 != 0)
				mem_row_size++;

			const size_t mem_size = mem_head_size + static_cast<size_t>(num_rows)* mem_row_size;
			char ***mem_data = result->m_Data = static_cast<char ***>(malloc(mem_size));
			char **mem_offset = reinterpret_cast<char **>(&mem_data[num_rows]);

			//TODO: check if mem_data == nullptr

			for (size_t r = 0; r != num_rows; ++r)
			{
				mysql_row = mysql_fetch_row(raw_result);

				//copy mysql result data to our location
				mem_data[r] = mem_offset;
				mem_offset += mem_row_size / sizeof(char **);
				memcpy(mem_data[r], mysql_row, mem_row_size);

				//correct the pointers of the copied mysql result data
				for (size_t f = 0; f != num_fields; ++f)
				{
					if (mysql_row[f] == NULL)
						continue;
					size_t dist = mysql_row[f] - reinterpret_cast<char *>(mysql_row);
					mem_data[r][f] = reinterpret_cast<char *>(mem_data[r]) + dist;
				}
			}

			mysql_free_result(raw_result);
		} while (mysql_next_result(connection) == 0 && (raw_result = mysql_store_result(connection)));
	}
	return CResultSet::Type_t(resultset);
}



CResultSet::Id_t CResultSetManager::StoreActiveResultSet()
{
	if (m_ActiveResultSet == false)
		return 0;

	CResultSet::Id_t id = 1;
	while (m_StoredResults.find(id) != m_StoredResults.end())
		id++;

	m_StoredResults.emplace(id, m_ActiveResultSet);
	return id;
}

bool CResultSetManager::DeleteResultSet(CResultSet::Id_t id)
{
	if (IsValidResultSet(id) == false)
		return false;

	if (GetResultSet(id) == GetActiveResultSet())
		SetActiveResultSet(nullptr);

	return m_StoredResults.erase(id) == 1;
}
