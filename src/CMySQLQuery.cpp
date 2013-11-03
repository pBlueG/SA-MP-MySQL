#pragma once

#include <cstdio>


#include "CMySQLQuery.h"
#include "CMySQLHandle.h"
#include "CCallback.h"
//#include "COrm.h"
#include "CLog.h"

#include "misc.h"

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>


CMySQLQuery CMySQLQuery::Create(
	string query, CMySQLConnection *connection,
	string cbname, stack<boost::variant<cell, string>> cbparams)
{
	CMySQLQuery QueryObj;

	QueryObj.Connection = connection;
	QueryObj.Query = boost::move(query);
	QueryObj.Callback.Name = boost::move(cbname);
	QueryObj.Callback.Params = boost::move(cbparams);


	MYSQL *ConnPtr = QueryObj.Connection->GetMySQLPointer();
	if (ConnPtr != NULL)
	{
		mysql_thread_init();


		char LogFuncBuf[128];
		sprintf(LogFuncBuf, "CMySQLQuery::Execute[%s]", QueryObj.Callback.Name.c_str());

		CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "starting query execution");


		if (mysql_real_query(ConnPtr, QueryObj.Query.c_str(), QueryObj.Query.length()) == 0)
		{
			CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "query was successful");

			MYSQL_RES *SQLResult = mysql_store_result(ConnPtr); //this has to be here

			//why should we process the result if it won't and can't be used?
			if (QueryObj.Callback.Name.length() > 0)
			{
				if (SQLResult != NULL)
				{
					MYSQL_FIELD *SQLField;
					MYSQL_ROW SQLRow;

					QueryObj.Result = new CMySQLResult;

					QueryObj.Result->m_WarningCount = mysql_warning_count(ConnPtr);
								 
					QueryObj.Result->m_Rows = mysql_num_rows(SQLResult);
					QueryObj.Result->m_Fields = mysql_num_fields(SQLResult);
								  
					QueryObj.Result->m_Data.reserve((unsigned int)QueryObj.Result->m_Rows + 1);
					QueryObj.Result->m_FieldNames.reserve(QueryObj.Result->m_Fields + 1);


					while (SQLField = mysql_fetch_field(SQLResult))
						QueryObj.Result->m_FieldNames.push_back(SQLField->name);


					while (SQLRow = mysql_fetch_row(SQLResult))
					{
						vector<string> Row;
						Row.reserve(QueryObj.Result->m_Fields + 1);
						for (unsigned int a = 0; a != QueryObj.Result->m_Fields; ++a)
							Row.push_back(SQLRow[a] == NULL ? "NULL" : SQLRow[a]);

						QueryObj.Result->m_Data.push_back(std::move(Row));
					}

				}
				else if (mysql_field_count(ConnPtr) == 0) //query is non-SELECT query
				{
					QueryObj.Result = new CMySQLResult;
					QueryObj.Result->m_WarningCount = mysql_warning_count(ConnPtr);
					QueryObj.Result->m_AffectedRows = mysql_affected_rows(ConnPtr);
					QueryObj.Result->m_InsertID = mysql_insert_id(ConnPtr);
				}
				else //error
				{
					int ErrorID = mysql_errno(ConnPtr);
					string ErrorString(mysql_error(ConnPtr));

					CLog::Get()->LogFunction(LOG_ERROR, LogFuncBuf, "an error occured while storing the result: (error #%d) \"%s\"", ErrorID, ErrorString.c_str());

					//we clear the callback name and forward it to the callback handler
					//the callback handler free's all memory but doesn't call the callback because there's no callback name
					QueryObj.Callback.Name.clear();
				}
			}
			else  //no callback was specified
			{
				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "no callback specified, skipping result saving");
			}

			if (SQLResult != NULL)
				mysql_free_result(SQLResult);
		}
		else  //mysql_real_query failed
		{
			int ErrorID = mysql_errno(ConnPtr);
			string ErrorString(mysql_error(ConnPtr));

			CLog::Get()->LogFunction(LOG_ERROR, LogFuncBuf, "(error #%d) %s", ErrorID, ErrorString.c_str());


			if (QueryObj.Connection->GetAutoReconnect() && ErrorID == 2006)
			{
				CLog::Get()->LogFunction(LOG_WARNING, LogFuncBuf, "lost connection, reconnecting..");

				MYSQL_RES *SQLRes;
				if ((SQLRes = mysql_store_result(ConnPtr)) != NULL)
					mysql_free_result(SQLRes);

				QueryObj.Connection->Disconnect();
				QueryObj.Connection->Connect();
			}

			if (QueryObj.Callback.Name.size() > 0)
			{
				//forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);
				//recycle these structures, change some data
				
				//OrmObject = NULL;
				//OrmQueryType = 0;

				while (QueryObj.Callback.Params.size() > 0)
					QueryObj.Callback.Params.pop();


				QueryObj.Callback.Params.push(static_cast<cell>(ErrorID));
				QueryObj.Callback.Params.push(ErrorString);
				QueryObj.Callback.Params.push(QueryObj.Callback.Name);
				QueryObj.Callback.Params.push(QueryObj.Query);
				QueryObj.Callback.Params.push(static_cast<cell>(1337)); //TODO: THIS IS IMPORTANT!!!!!!

				QueryObj.Callback.Name = "OnQueryError";

				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "error will be triggered in OnQueryError");
			}
		}
		mysql_thread_end();
	}
	connection->ToggleState(false);
	/*if (Threaded == true)
	{
		//the query gets passed to the callback handler in any case
		//if query successful, it calls the callback and free's memory
		//if not it only free's the memory
		CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "data being passed to ProcessCallbacks()");
		CCallback::AddQueryToQueue(this);
	}*/


	return QueryObj;
}

CMySQLQuery::~CMySQLQuery()
{
	//delete Result;
}