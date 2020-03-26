#include "CResult.hpp"
#include "CLog.hpp"

#include <cstring>

#include "mysql.hpp"


CResult::~CResult()
{
	if (m_Data != nullptr)
		free(m_Data);
}

bool CResult::GetFieldName(unsigned int idx, string &dest) const
{
	if (idx < m_NumFields)
	{
		dest = m_Fields.at(idx).Name;
		return true;
	}
	return false;
}

bool CResult::GetFieldType(unsigned int idx, enum_field_types &dest) const
{
	if (idx < m_NumFields)
	{
		dest = m_Fields.at(idx).Type;
		return true;
	}
	return false;
}

bool CResult::GetRowData(unsigned int row,
						 unsigned int fieldidx, const char **dest) const
{
	if (row < m_NumRows && fieldidx < m_NumFields)
	{
		*dest = m_Data[row][fieldidx];
		return true;
	}
	return false;
}

bool CResult::GetRowDataByName(unsigned int row,
							   const string &field, const char **dest) const
{
	if (row >= m_NumRows)
		return false;

	if (field.empty())
		return false;


	for (unsigned int i = 0; i != m_NumFields; ++i)
	{
		if (m_Fields.at(i).Name.compare(field) == 0)
		{
			*dest = m_Data[row][i];
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

ResultSet_t CResultSet::Create(MYSQL *connection,
							   default_clock::duration &exec_time,
							   string query_str)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CResultSet::Create(connection={}, query_str='{}')",
					 static_cast<const void *>(connection), query_str);

	if (connection == nullptr)
		return nullptr;


	ResultSet_t resultset = ResultSet_t(new CResultSet);
	CLog::Get()->Log(LogLevel::DEBUG, "created new resultset '{}'",
					 static_cast<const void *>(resultset.get()));
	bool error = false;
	MYSQL_RES *raw_result = nullptr;
	do
	{
		raw_result = mysql_store_result(connection);
		CLog::Get()->Log(LogLevel::DEBUG, "fetched MySQL result '{}'",
						 static_cast<const void*>(raw_result));

		if (raw_result == nullptr) //result empty: non-SELECT-type query or error
		{
			if (mysql_field_count(connection) == 0) //query is non-SELECT-type query
			{
				CResult *result = new CResult;

				resultset->m_Results.push_back(result);

				result->m_WarningCount = mysql_warning_count(connection);
				result->m_AffectedRows = mysql_affected_rows(connection);
				result->m_InsertId = mysql_insert_id(connection);
			}
			else //error
			{
				error = true;
				break;
			}
		}
		else //SELECT-type query
		{
			CResult *result = new CResult;

			resultset->m_Results.push_back(result);

			result->m_WarningCount = mysql_warning_count(connection);

			MYSQL_FIELD *mysql_field;
			MYSQL_ROW mysql_row;

			const my_ulonglong num_rows
				= result->m_NumRows = mysql_num_rows(raw_result);
			const unsigned int num_fields
				= result->m_NumFields = mysql_num_fields(raw_result);

			result->m_Fields.reserve(num_fields + 1);


			size_t row_data_size = 0;
			while ((mysql_field = mysql_fetch_field(raw_result)))
			{
				result->m_Fields.push_back({ mysql_field->name, mysql_field->type });
				row_data_size += mysql_field->max_length + 1;
			}


			size_t
				mem_head_size = sizeof(char **) * static_cast<size_t>(num_rows),
				mem_row_size = (sizeof(char *) * (num_fields + 1)) + ((row_data_size) * sizeof(char));
			//+ 1 because there is another value in memory pointing to behind the last MySQL field

			//mem_row_size has to be a multiple of 8
			mem_row_size += 8 - (mem_row_size % 8);

			const size_t mem_size = mem_head_size + static_cast<size_t>(num_rows) * mem_row_size;
			char ***mem_data = result->m_Data = static_cast<char ***>(malloc(mem_size));
			if (mem_data == nullptr) //error while allocating memory
			{
				error = true;
				break;
			}

			CLog::Get()->Log(LogLevel::DEBUG,
							 "allocated {} bytes for PAWN result", mem_size);

			char **mem_offset = reinterpret_cast<char **>(&mem_data[num_rows]);
			for (size_t r = 0; r != num_rows; ++r)
			{
				mysql_row = mysql_fetch_row(raw_result);

				//copy mysql result data to our location
				mem_data[r] = mem_offset;
				mem_offset += mem_row_size / sizeof(char **);
				size_t copy_size = mysql_row[num_fields] - reinterpret_cast<char *>(mysql_row);
				memcpy(mem_data[r], mysql_row, copy_size);

				//correct the pointers of the copied mysql result data
				for (size_t f = 0; f != num_fields; ++f)
				{
					if (mysql_row[f] == nullptr)
						continue;
					size_t dist = mysql_row[f] - reinterpret_cast<char *>(mysql_row);
					mem_data[r][f] = reinterpret_cast<char *>(mem_data[r]) + dist;
				}
				//useless field we had to copy
				//set it to nullptr to avoid invalid memory access errors
				//(very unlikely to happen in first place though)
				mem_data[r][num_fields] = nullptr;
			}

			mysql_free_result(raw_result);
			raw_result = nullptr;
		}
	}
	while (mysql_next_result(connection) == 0);

	if (error)
	{
		mysql_free_result(raw_result);

		// go through all results to avoid "out of sync" error
		while (mysql_next_result(connection) == 0)
			mysql_free_result(mysql_store_result(connection));
	}
	resultset->m_ExecTimeMilli = static_cast<unsigned int>(
		std::chrono::duration_cast<std::chrono::milliseconds>(exec_time).count());
	resultset->m_ExecTimeMicro = static_cast<unsigned int>(
		std::chrono::duration_cast<std::chrono::microseconds>(exec_time).count());

	resultset->m_ExecQuery = std::move(query_str);

	return resultset;
}

ResultSet_t CResultSet::Merge(const std::vector<ResultSet_t> &results)
{
	ResultSet_t resultset = ResultSet_t(new CResultSet);
	for (ResultSet_t rset : results)
	{
		if (rset)
		{
			for (Result_t r : rset->m_Results)
				resultset->m_Results.push_back(r);

			rset->m_Results.clear();
		}
	}
	return resultset;
}



ResultSetId_t CResultSetManager::StoreActiveResultSet()
{
	if (m_ActiveResultSet == nullptr)
		return 0;

	ResultSetId_t id = 1;
	while (m_StoredResults.find(id) != m_StoredResults.end())
		id++;

	m_StoredResults.emplace(id, m_ActiveResultSet);
	return id;
}

bool CResultSetManager::DeleteResultSet(ResultSetId_t id)
{
	if (IsValidResultSet(id) == false)
		return false;

	if (GetResultSet(id) == GetActiveResultSet())
		SetActiveResultSet(nullptr);

	return m_StoredResults.erase(id) == 1;
}
