#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"
#include "CLog.h"

#include "misc.h"

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;


bool CMySQLQuery::Execute(MYSQL *mysql_connection)
{
	bool ret_val = false;
	char log_funcname[64];
	if (Unthreaded)
		sprintf(log_funcname, "CMySQLQuery::Execute");
	else
		sprintf(log_funcname, "CMySQLQuery::Execute[%s]", Callback.Name.c_str());

	CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "starting query execution");

	chrono::steady_clock::time_point query_exec = chrono::steady_clock::now();
	int query_error = mysql_real_query(mysql_connection, Query.c_str(), Query.length());
	chrono::steady_clock::duration query_exec_duration = chrono::steady_clock::now() - query_exec;
	
	if (query_error == 0)
	{
		unsigned int 
			query_exec_time_milli = static_cast<unsigned int>(chrono::duration_cast<chrono::milliseconds>(query_exec_duration).count()),
			query_exec_time_micro = static_cast<unsigned int>(chrono::duration_cast<chrono::microseconds>(query_exec_duration).count());

		CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "query was successfully executed within %d.%d milliseconds", query_exec_time_milli, query_exec_time_micro-(query_exec_time_milli*1000));

		MYSQL_RES *mysql_result = mysql_store_result(mysql_connection); //this has to be here

		//why should we process the result if it won't and can't be used?
		if (Unthreaded || Callback.Name.length() > 0)
		{
			if (StoreResult(mysql_connection, mysql_result) == false)
				CLog::Get()->LogFunction(LOG_ERROR, log_funcname, "an error occured while storing the result: (error #%d) \"%s\"", mysql_errno(mysql_connection), mysql_error(mysql_connection));
			else
			{
				Result->m_Query = Query;
				Result->m_ExecTime[UNIT_MILLISECONDS] = query_exec_time_milli;
				Result->m_ExecTime[UNIT_MICROSECONDS] = query_exec_time_micro;
			}
		}
		else  //no callback was specified
			CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "no callback specified, skipping result saving");

		if (mysql_result != NULL)
			mysql_free_result(mysql_result);

		ret_val = true;
	}
	else  //mysql_real_query failed
	{
		int error_id = mysql_errno(mysql_connection);
		string error_str(mysql_error(mysql_connection));

		CLog::Get()->LogFunction(LOG_ERROR, log_funcname, "(error #%d) %s", error_id, error_str.c_str());

		if (!Unthreaded)
		{
			//forward OnQueryError(error_id, error[], callback[], query[], connectionHandle);
			//recycle these structures, change some data

			Orm.Object = NULL;
			Orm.Type = 0;

			while (Callback.Params.size() > 0)
				Callback.Params.pop();


			Callback.Params.push(static_cast<cell>(error_id));
			Callback.Params.push(error_str);
			Callback.Params.push(Callback.Name);
			Callback.Params.push(Query);
			Callback.Params.push(static_cast<cell>(Handle->GetID()));

			Callback.Name = "OnQueryError";

			CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "error will be triggered in OnQueryError");
		}
		ret_val = false;
	}

	if(Unthreaded == false) //decrease counter only if threaded query
		Handle->DecreaseQueryCounter();
	return ret_val;
}


bool CMySQLQuery::StoreResult(MYSQL *mysql_connection, MYSQL_RES *mysql_result)
{
	if (mysql_result != NULL)
	{
		MYSQL_FIELD *mysql_field;
		MYSQL_ROW mysql_row;

		CMySQLResult *result_ptr = Result = new CMySQLResult;

		Result->m_WarningCount = mysql_warning_count(mysql_connection);

		const my_ulonglong num_rows = Result->m_Rows = mysql_num_rows(mysql_result);
		const unsigned int num_fields = Result->m_Fields = mysql_num_fields(mysql_result);

		Result->m_FieldNames.reserve(Result->m_Fields + 1);


		size_t row_data_size = 0;
		while (mysql_field = mysql_fetch_field(mysql_result))
		{
			Result->m_FieldNames.push_back(mysql_field->name);
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
		char ***mem_data = result_ptr->m_Data = static_cast<char ***>(malloc(mem_size));
		char **mem_offset = reinterpret_cast<char **>(&mem_data[num_rows]);

		for (size_t r = 0; r != num_rows; ++r)
		{
			mysql_row = mysql_fetch_row(mysql_result);

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
		return true;
	}
	else if (mysql_field_count(mysql_connection) == 0) //query is non-SELECT query
	{
		Result = new CMySQLResult;
		Result->m_WarningCount = mysql_warning_count(mysql_connection);
		Result->m_AffectedRows = mysql_affected_rows(mysql_connection);
		Result->m_InsertID = mysql_insert_id(mysql_connection);
		return true;
	}
	else //error
	{
		//we clear the callback name and forward it to the callback handler
		//the callback handler free's all memory but doesn't call the callback because there's no callback name
		Callback.Name.clear();
		return false;
	}
}
