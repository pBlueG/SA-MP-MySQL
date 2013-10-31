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
	string query, const CMySQLConnection *connection,
	string cbname, string cbformat, stack<string> cbparams)
{
	CMySQLQuery QueryObj(connection);

	QueryObj.Query = boost::move(query);
	QueryObj.Callback.Name = boost::move(cbname);
	QueryObj.Callback.Format = boost::move(cbformat);
	QueryObj.Callback.Params = boost::move(cbparams);


	MYSQL *ConnPtr = const_cast<MYSQL*>(QueryObj.Connection->GetMySQLPointer()); //don't try this at home kids!
	if (ConnPtr != nullptr)
	{
		mysql_thread_init();
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


					QueryObj.Result.m_WarningCount = mysql_warning_count(ConnPtr);

					QueryObj.Result.m_Rows = mysql_num_rows(SQLResult);
					QueryObj.Result.m_Fields = mysql_num_fields(SQLResult);
					
					QueryObj.Result.m_Data.reserve((unsigned int)QueryObj.Result.m_Rows + 1);
					QueryObj.Result.m_FieldNames.reserve(QueryObj.Result.m_Fields + 1);


					while ((SQLField = mysql_fetch_field(SQLResult)))
						QueryObj.Result.m_FieldNames.push_back(SQLField->name);


					while (SQLRow = mysql_fetch_row(SQLResult))
					{
						std::vector< vector<string> >::iterator It = QueryObj.Result.m_Data.insert(QueryObj.Result.m_Data.end(), vector<string>());
						It->reserve(QueryObj.Result.m_Fields + 1);

						for (unsigned int a = 0; a < QueryObj.Result.m_Fields; ++a)
							It->push_back(!SQLRow[a] ? "NULL" : SQLRow[a]);
					}

				}
				else if (mysql_field_count(ConnPtr) == 0) //query is non-SELECT query
				{
					QueryObj.Result.m_WarningCount = mysql_warning_count(ConnPtr);
					QueryObj.Result.m_AffectedRows = mysql_affected_rows(ConnPtr);
					QueryObj.Result.m_InsertID = mysql_insert_id(ConnPtr);
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

				char
					strErrorID[12],
					strConnID[12];
				ConvertIntToStr(ErrorID, strErrorID);
				//ConvertIntToStr(ConnHandle->GetID(), strConnID); //TODO: THIS IS IMPORTANT!!!!!!

				QueryObj.Callback.Params.push(strErrorID);
				QueryObj.Callback.Params.push(ErrorString);
				QueryObj.Callback.Params.push(QueryObj.Callback.Name);
				QueryObj.Callback.Params.push(QueryObj.Query);
				QueryObj.Callback.Params.push(strConnID);

				QueryObj.Callback.Name = "OnQueryError";
				QueryObj.Callback.Format = "dsssd";

				//CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "error will be triggered in OnQueryError");
			}
		}
		mysql_thread_end();
	}

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
/*
CMySQLQuery::CMySQLQuery()  :
	Threaded(true),

	ConnHandle(NULL),
	Connection(NULL),
	Result(NULL),
	Callback(NULL),

	OrmObject(NULL),
	OrmQueryType(0)
{ 
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLQuery::CMySQLQuery()", "constructor called");
}

CMySQLQuery::~CMySQLQuery() {
	delete Result;
	delete Callback;

	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLQuery::~CMySQLQuery()", "deconstructor called");
}

CMySQLQuery *CMySQLQuery::Create(
	const char *query, CMySQLHandle *connhandle, 
	const char *cbname, const char *cbformat, 
	bool threaded ,
	COrm *ormobject , unsigned short orm_querytype)
{
	if(connhandle == NULL) 
	{
		CLog::Get()->LogFunction(LOG_ERROR, "CMySQLQuery::Create", "no connection handle specified");
		return (CMySQLQuery *)(NULL);
	}

	if(query == NULL && ormobject == NULL) 
	{
		CLog::Get()->LogFunction(LOG_ERROR, "CMySQLQuery::Create", "no query and orm object specified");
		return (CMySQLQuery *)(NULL);
	}
	

	CMySQLQuery *Query = new CMySQLQuery;
	CCallback *Callback = new CCallback;

	if(ormobject != NULL) 
	{
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLQuery::Create", "starting query generation");
		switch(orm_querytype) 
		{
		case ORM_QUERYTYPE_SELECT:
			ormobject->GenerateSelectQuery(Query->Query);
			break;
		case ORM_QUERYTYPE_UPDATE:
			ormobject->GenerateUpdateQuery(Query->Query);
			break;
		case ORM_QUERYTYPE_INSERT:
			ormobject->GenerateInsertQuery(Query->Query);
			break;
		case ORM_QUERYTYPE_DELETE:
			ormobject->GenerateDeleteQuery(Query->Query);
			break;
		case ORM_QUERYTYPE_SAVE:
			orm_querytype = ormobject->GenerateSaveQuery(Query->Query);
		}

		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLQuery::Create", "query successful generated");
	}
	else 
	{
		if(query != NULL)
			Query->Query.assign(query);
	}

	if(cbname != NULL) 
	{
		Callback->Name.assign(cbname);
		if(cbformat != NULL)
			Callback->ParamFormat.assign(cbformat);
	}

	Query->Threaded = threaded;
	Query->ConnHandle = connhandle; 
	Query->Connection = threaded == true ? connhandle->GetQueryConnection() : connhandle->GetMainConnection(); 
	Query->Callback = Callback;
	Query->OrmObject = ormobject;
	Query->OrmQueryType = orm_querytype;

	if(Query->Callback->Name.find("FJ37DH3JG") != -1) 
	{
		Query->Callback->IsInline = true;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLQuery::Create", "inline function detected");
	}

	return Query;
}

void CMySQLQuery::Destroy() 
{
	delete this;
}

void CMySQLQuery::Execute() 
{
	char LogFuncBuf[128];
	sprintf(LogFuncBuf, "CMySQLQuery::Execute[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
	
	CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "starting query execution");

	Result = NULL;
	MYSQL *ConnPtr = Connection->GetMySQLPointer();

	if(ConnPtr != NULL) 
	{
		if (mysql_real_query(ConnPtr, Query.c_str(), Query.length()) == 0) 
		{
			CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "query was successful");

			MYSQL_RES *SQLResult = mysql_store_result(ConnPtr); //this has to be here

			//why should we process the result if it won't and can't be used?
			if(Threaded == false || Callback->Name.length() > 0 || (OrmObject != NULL && (OrmQueryType == ORM_QUERYTYPE_SELECT || OrmQueryType == ORM_QUERYTYPE_INSERT))) 
			{ 
				if (SQLResult != NULL) 
				{
					MYSQL_FIELD *SQLField;
					MYSQL_ROW SQLRow;

					Result = new CMySQLResult;

					Result->m_WarningCount = mysql_warning_count(ConnPtr);

					Result->m_Rows = mysql_num_rows(SQLResult);
					Result->m_Fields = mysql_num_fields(SQLResult);

					Result->m_Data.reserve((unsigned int)Result->m_Rows+1);
					Result->m_FieldNames.reserve(Result->m_Fields+1);


					while ((SQLField = mysql_fetch_field(SQLResult)))
						Result->m_FieldNames.push_back(SQLField->name);
					
				
					while (SQLRow = mysql_fetch_row(SQLResult)) 
					{
						std::vector< vector<string> >::iterator It = Result->m_Data.insert(Result->m_Data.end(), vector<string>());
						It->reserve(Result->m_Fields+1);
					
						for (unsigned int a = 0; a < Result->m_Fields; ++a)
							It->push_back(!SQLRow[a] ? "NULL" : SQLRow[a]);
					}

				}
				else if(mysql_field_count(ConnPtr) == 0) //query is non-SELECT query
				{
					Result = new CMySQLResult;
				
					Result->m_WarningCount = mysql_warning_count(ConnPtr);
					Result->m_AffectedRows = mysql_affected_rows(ConnPtr);
					Result->m_InsertID = mysql_insert_id(ConnPtr); 
				}
				else //error
				{
					int ErrorID = mysql_errno(ConnPtr);
					string ErrorString(mysql_error(ConnPtr));

					CLog::Get()->LogFunction(LOG_ERROR, LogFuncBuf, "an error occured while storing the result: (error #%d) \"%s\"", ErrorID, ErrorString.c_str());
					
					//we clear the callback name and forward it to the callback handler
					//the callback handler free's all memory but doesn't call the callback because there's no callback name
					Callback->Name.clear(); 
				}
			}
			else  //no callback was specified
				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "no callback specified, skipping result saving");

			if(SQLResult != NULL)
				mysql_free_result(SQLResult);
		}
		else  //mysql_real_query failed
		{
			int ErrorID = mysql_errno(ConnPtr);
			string ErrorString(mysql_error(ConnPtr));

			CLog::Get()->LogFunction(LOG_ERROR, LogFuncBuf, "(error #%d) %s", ErrorID, ErrorString.c_str());
			
			
			if(Connection->GetAutoReconnect() && ErrorID == 2006) 
			{
				CLog::Get()->LogFunction(LOG_WARNING, LogFuncBuf, "lost connection, reconnecting..");

				MYSQL_RES *SQLRes;
				if ((SQLRes = mysql_store_result(ConnPtr)) != NULL)
					mysql_free_result(SQLRes);
				
				Connection->Disconnect();
				Connection->Connect();
			}

			if(Threaded == true) 
			{
				//forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);
				//recycle these structures, change some data
				OrmObject = NULL;
				OrmQueryType = 0;

				while(Callback->Parameters.size() > 0)
					Callback->Parameters.pop();

				char 
					strErrorID[12], 
					strConnID[12];
				ConvertIntToStr(ErrorID, strErrorID);
				ConvertIntToStr(ConnHandle->GetID(), strConnID);

				Callback->Parameters.push(strErrorID);
				Callback->Parameters.push(ErrorString);
				Callback->Parameters.push(Callback->Name);
				Callback->Parameters.push(Query);
				Callback->Parameters.push(strConnID);

				Callback->Name = "OnQueryError";
				Callback->ParamFormat = "dsssd";

				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "error will be triggered in OnQueryError");
			}
		}
	}

	if(Threaded == true) 
	{
		//the query gets passed to the callback handler in any case
		//if query successful, it calls the callback and free's memory
		//if not it only free's the memory
		CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "data being passed to ProcessCallbacks()");
		CCallback::AddQueryToQueue(this);
	}
}
*/