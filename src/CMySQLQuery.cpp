#pragma once

#include <cstdio>


#include "CMySQLQuery.h"
#include "CMySQLHandle.h"
#include "CCallback.h"
//#include "COrm.h"
#include "CLog.h"

#include "misc.h"


CMySQLQuery CMySQLQuery::Create(
	string query, CMySQLConnection *connection,
	string cbname, stack<boost::variant<cell, string>> cbparams)
{
	CMySQLQuery QueryObj;

	QueryObj.Connection = connection;
	QueryObj.Query = std::move(query);
	QueryObj.Callback.Name = std::move(cbname);
	QueryObj.Callback.Params = std::move(cbparams);


	MYSQL *mysql_connection = QueryObj.Connection->GetMySQLPointer();
	if (connection != NULL)
	{
		mysql_thread_init();


		char log_funcname[128];
		sprintf(log_funcname, "CMySQLQuery::Execute[%s]", QueryObj.Callback.Name.c_str());

		CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "starting query execution");


		if (mysql_real_query(mysql_connection, QueryObj.Query.c_str(), QueryObj.Query.length()) == 0)
		{
			CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "query was successful");

			MYSQL_RES *mysql_result = mysql_store_result(mysql_connection); //this has to be here

			//why should we process the result if it won't and can't be used?
			if (QueryObj.Callback.Name.length() > 0)
			{
				if (mysql_result != NULL)
				{
					MYSQL_FIELD *mysql_field;
					MYSQL_ROW mysql_row;

					QueryObj.Result = new CMySQLResult;

					QueryObj.Result->m_WarningCount = mysql_warning_count(mysql_connection);
								 
					QueryObj.Result->m_Rows = mysql_num_rows(mysql_result);
					QueryObj.Result->m_Fields = mysql_num_fields(mysql_result);
								  
					QueryObj.Result->m_Data.reserve((unsigned int)QueryObj.Result->m_Rows + 1);
					QueryObj.Result->m_FieldNames.reserve(QueryObj.Result->m_Fields + 1);


					while (mysql_field = mysql_fetch_field(mysql_result))
						QueryObj.Result->m_FieldNames.push_back(mysql_field->name);


					while (mysql_row = mysql_fetch_row(mysql_result))
					{
						vector<string> Row;
						Row.reserve(QueryObj.Result->m_Fields + 1);
						for (unsigned int a = 0; a != QueryObj.Result->m_Fields; ++a)
							Row.push_back(mysql_row[a] == NULL ? "NULL" : mysql_row[a]);

						QueryObj.Result->m_Data.push_back(std::move(Row));
					}

				}
				else if (mysql_field_count(mysql_connection) == 0) //query is non-SELECT query
				{
					QueryObj.Result = new CMySQLResult;
					QueryObj.Result->m_WarningCount = mysql_warning_count(mysql_connection);
					QueryObj.Result->m_AffectedRows = mysql_affected_rows(mysql_connection);
					QueryObj.Result->m_InsertID = mysql_insert_id(mysql_connection);
				}
				else //error
				{
					int ErrorID = mysql_errno(mysql_connection);
					string ErrorString(mysql_error(mysql_connection));

					CLog::Get()->LogFunction(LOG_ERROR, log_funcname, "an error occured while storing the result: (error #%d) \"%s\"", ErrorID, ErrorString.c_str());

					//we clear the callback name and forward it to the callback handler
					//the callback handler free's all memory but doesn't call the callback because there's no callback name
					QueryObj.Callback.Name.clear();
				}
			}
			else  //no callback was specified
			{
				CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "no callback specified, skipping result saving");
			}

			if (mysql_result != NULL)
				mysql_free_result(mysql_result);
		}
		else  //mysql_real_query failed
		{
			int error_id = mysql_errno(mysql_connection);
			string error_str(mysql_error(mysql_connection));

			CLog::Get()->LogFunction(LOG_ERROR, log_funcname, "(error #%d) %s", error_id, error_str.c_str());


			if (QueryObj.Connection->GetAutoReconnect() && error_id == 2006)
			{
				CLog::Get()->LogFunction(LOG_WARNING, log_funcname, "lost connection, reconnecting..");

				MYSQL_RES *mysql_res;
				if ((mysql_res = mysql_store_result(mysql_connection)) != NULL)
					mysql_free_result(mysql_res);

				QueryObj.Connection->Disconnect();
				QueryObj.Connection->Connect();
			}

			if (QueryObj.Callback.Name.size() > 0)
			{
				//forward OnQueryError(error_id, error[], callback[], query[], connectionHandle);
				//recycle these structures, change some data
				
				//OrmObject = NULL;
				//OrmQueryType = 0;

				while (QueryObj.Callback.Params.size() > 0)
					QueryObj.Callback.Params.pop();


				QueryObj.Callback.Params.push(static_cast<cell>(error_id));
				QueryObj.Callback.Params.push(error_str);
				QueryObj.Callback.Params.push(QueryObj.Callback.Name);
				QueryObj.Callback.Params.push(QueryObj.Query);
				QueryObj.Callback.Params.push(static_cast<cell>(1337)); //TODO: THIS IS IMPORTANT!!!!!!

				QueryObj.Callback.Name = "OnQueryError";

				CLog::Get()->LogFunction(LOG_DEBUG, log_funcname, "error will be triggered in OnQueryError");
			}
		}
		mysql_thread_end();
	}
	connection->ToggleState(false);

	return QueryObj;
}

CMySQLQuery::~CMySQLQuery()
{
	//delete Result;
}