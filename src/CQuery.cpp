#include "CQuery.h"
#include "CResult.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


bool CQuery::Execute(MYSQL *connection)
{
	if (mysql_real_query(connection, m_Query.c_str(), m_Query.length()) != 0)
		return false;
	

	MYSQL_RES *raw_result = mysql_store_result(connection);

	m_Result = StoreResult(connection, raw_result);
	bool ret_val = m_Result != nullptr;
	
	if (raw_result != NULL)
		mysql_free_result(raw_result);

	return ret_val;
}


CResult *CQuery::StoreResult(MYSQL *connection, MYSQL_RES *raw_result)
{
	CResult *result = nullptr;
	if (raw_result != NULL)
	{
		MYSQL_FIELD *mysql_field;
		MYSQL_ROW mysql_row;

		result = new CResult;

		result->m_WarningCount = mysql_warning_count(connection);

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
			mem_row_size = (sizeof(char *) * (num_fields + 1)) + ((row_data_size) * sizeof(char));
		//+1 because there is another value in memory pointing to somewhere
		//mem_row_size has to be a multiple of 8
		while (mem_row_size % 8 != 0)
			mem_row_size++;

		const size_t mem_size = mem_head_size + static_cast<size_t>(num_rows) * mem_row_size;
		char ***mem_data = result->m_Data = static_cast<char ***>(malloc(mem_size));
		char **mem_offset = reinterpret_cast<char **>(&mem_data[num_rows]);

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
				if(mysql_row[f] == NULL)
					continue;
				size_t dist = mysql_row[f] - reinterpret_cast<char *>(mysql_row);
				mem_data[r][f] = reinterpret_cast<char *>(mem_data[r]) + dist;
			}
		}
	}
	else if (mysql_field_count(connection) == 0) //query is non-SELECT query
	{
		result = new CResult;
		result->m_WarningCount = mysql_warning_count(connection);
		result->m_AffectedRows = mysql_affected_rows(connection);
		result->m_InsertId = mysql_insert_id(connection);
	}
	
	return result;
}
