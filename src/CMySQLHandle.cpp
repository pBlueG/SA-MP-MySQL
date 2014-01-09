#pragma once

#include "CLog.h"

#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"
#include "CMySQLConnection.h"
#include "CCallback.h"

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;


unordered_map<unsigned int, CMySQLHandle *> CMySQLHandle::SQLHandle;
CMySQLHandle *CMySQLHandle::ActiveHandle = NULL;
CMySQLOptions MySQLOptions;


CMySQLHandle::CMySQLHandle(unsigned int id) : 
	m_MyID(id),
	
	m_ActiveResult(NULL),
	m_ActiveResultID(0),
	
	m_MainConnection(NULL),
	m_ThreadConnection(NULL),

	m_QueryCounter(0),
	m_QueryThreadRunning(true),
	m_QueryStashThread(boost::bind(&CMySQLHandle::ExecThreadStashFunc, this))
{
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::CMySQLHandle", "constructor called");
}

CMySQLHandle::~CMySQLHandle() 
{
	for (unordered_map<unsigned int, CMySQLResult*>::iterator it = m_SavedResults.begin(), end = m_SavedResults.end(); it != end; it++)
		delete it->second;
	
	ExecuteOnConnections(&CMySQLConnection::Destroy);

	m_QueryThreadRunning = false;
	m_QueryStashThread.join();

	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::~CMySQLHandle", "deconstructor called");
}

void CMySQLHandle::WaitForQueryExec() 
{
	while (m_QueryCounter != 0)
		this_thread::sleep_for(chrono::milliseconds(5));
}

CMySQLHandle *CMySQLHandle::Create(string host, string user, string pass, string db, size_t port, size_t pool_size, bool reconnect) 
{
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", "creating new connection..");

	CMySQLHandle *handle = NULL;
	CMySQLConnection *main_connection = CMySQLConnection::Create(host, user, pass, db, port, reconnect);

	if (MySQLOptions.DuplicateConnections == false && SQLHandle.size() > 0) 
	{
		//code used for checking duplicate connections
		for(unordered_map<unsigned int, CMySQLHandle*>::iterator i = SQLHandle.begin(), end = SQLHandle.end(); i != end; ++i) 
		{
			CMySQLConnection *Connection = i->second->m_MainConnection;
			if((*Connection) == (*main_connection))
			{
				CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::Create", "connection already exists");
				handle = i->second;
				break;
			}
		}
	}

	if(handle == NULL) 
	{
		unsigned int id = 1;
		if(SQLHandle.size() > 0) 
		{
			unordered_map<unsigned int, CMySQLHandle*>::iterator itHandle = SQLHandle.begin();
			do 
			{
				id = itHandle->first+1;
				++itHandle;
			} while(SQLHandle.find(id) != SQLHandle.end());
		}


		handle = new CMySQLHandle(id);

		//init connections
		handle->m_MainConnection = main_connection;
		handle->m_ThreadConnection = CMySQLConnection::Create(host, user, pass, db, port, reconnect, handle->m_QueryCounter, id);

		for (size_t i = 0; i < pool_size; ++i)
			handle->m_ConnectionPool.insert(CMySQLConnection::Create(host, user, pass, db, port, reconnect, handle->m_QueryCounter, id));

		SQLHandle.insert( unordered_map<unsigned int, CMySQLHandle*>::value_type(id, handle) );

		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", "connection created with id = %d", id);
	}
	return handle;
}

void CMySQLHandle::Destroy() 
{
	if (ActiveHandle == this)
		ActiveHandle = NULL;

	SQLHandle.erase(m_MyID);
	delete this;
}

void CMySQLHandle::ExecuteOnConnections(void (CMySQLConnection::*func)())
{
	if (m_MainConnection != NULL)
		(m_MainConnection->*func)();
	
	if (m_ThreadConnection != NULL)
		(m_ThreadConnection->*func)();
	
	
	for(unordered_set<CMySQLConnection*>::iterator c = m_ConnectionPool.begin(), end = m_ConnectionPool.end(); c != end; ++c)
		((*c)->*func)();
}

unsigned int CMySQLHandle::SaveActiveResult() 
{
	if(m_ActiveResult != NULL) 
	{
		if(m_ActiveResultID != 0) //if active cache was already saved
		{
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::SaveActiveResult", "active cache was already saved");
			return m_ActiveResultID; //return the id of already saved cache
		}
		else 
		{
			unsigned int id = 1;
			if(!m_SavedResults.empty()) 
			{
				unordered_map<unsigned int, CMySQLResult*>::iterator itHandle = m_SavedResults.begin();
				do 
				{
					id = itHandle->first+1;
					++itHandle;
				} 
				while(m_SavedResults.find(id) != m_SavedResults.end()); //TODO: benchmark + eventually faster lookup
			}

			m_ActiveResultID = id;
			m_SavedResults.insert( unordered_map<unsigned int, CMySQLResult*>::value_type(id, m_ActiveResult) );
			
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SaveActiveResult", "cache saved with id = %d", id);
			return id; 
		}
	}
	
	return 0;
}

bool CMySQLHandle::DeleteSavedResult(unsigned int resultid) 
{
	if (IsValidResult(resultid))
	{
		CMySQLResult *ResultHandle = m_SavedResults.at(resultid);
		if(m_ActiveResult == ResultHandle) 
		{
			m_ActiveResult = NULL;
			m_ActiveResultID = 0;
			ActiveHandle = NULL;
		}
		delete ResultHandle;
		m_SavedResults.erase(resultid);
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::DeleteSavedResult", "result deleted");
		return true;
	}
	else
	{
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::DeleteSavedResult", "invalid result id ('%d')", resultid);
		return false;
	}
}

bool CMySQLHandle::SetActiveResult(unsigned int resultid) 
{
	if(resultid != 0) 
	{
		if(m_SavedResults.find(resultid) != m_SavedResults.end()) 
		{
			CMySQLResult *cResult = m_SavedResults.at(resultid);
			if(cResult != NULL) 
			{
				if(m_ActiveResult != NULL)
					if (m_ActiveResultID == 0) //if cache not saved
						delete m_ActiveResult; //delete unsaved cache
				
				m_ActiveResult = cResult; //set new active cache
				m_ActiveResultID = resultid; //new active cache was stored previously
				ActiveHandle = this;
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "result is now active");
			}
		}
		else
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::SetActiveResult", "result not found");
	}
	else 
	{
		if (m_ActiveResultID == 0) //if cache not saved
			delete m_ActiveResult; //delete unsaved cache
		m_ActiveResult = NULL;
		m_ActiveResultID = 0;
		ActiveHandle = NULL;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "invalid result id specified, setting active result to zero");
	}
	return true;
}

void CMySQLHandle::ClearAll()
{
	for(unordered_map<unsigned int, CMySQLHandle *>::iterator i = SQLHandle.begin(); i != SQLHandle.end(); ++i)
		i->second->Destroy();
	
	SQLHandle.clear();
}

void CMySQLHandle::SetActiveResult(CMySQLResult *result)
{
	m_ActiveResult = result;
	m_ActiveResultID = 0;
	if (result != NULL)
		ActiveHandle = this;
	else
		ActiveHandle = NULL;
}


void CMySQLHandle::ExecThreadStashFunc()
{
	m_QueryThreadRunning = true;
	while (m_QueryThreadRunning)
	{
		while (!m_QueryQueue.empty())
		{
			tuple< function<CMySQLQuery(CMySQLConnection*)>, bool > &query_data = m_QueryQueue.front();
			const bool use_pool = boost::get<1>(query_data);
			function<CMySQLQuery(CMySQLConnection*)> QueryFunc(boost::move(boost::get<0>(query_data)));
			
			m_QueryQueue.pop();
			
			CMySQLConnection *connection = NULL;
			do
			{
				if (use_pool == false)
				{
					if (m_ThreadConnection->IsInUse == false)
					{
						m_ThreadConnection->IsInUse = true;
						connection = m_ThreadConnection;
					}
				}
				else
				{
					for (unordered_set<CMySQLConnection*>::iterator c = m_ConnectionPool.begin(), end = m_ConnectionPool.end(); c != end; ++c)
					{
						if ((*c)->IsInUse == false)
						{
							(*c)->IsInUse = true;
							connection = (*c);
							break;
						}
					}
				}

				if (connection == NULL)
					this_thread::sleep_for(chrono::milliseconds(5));
			} 
			while (connection == NULL);

			m_QueryCounter++;
			shared_future<CMySQLQuery> fut = boost::async(boost::launch::async, boost::bind(QueryFunc, connection));
			CCallback::Get()->AddQueryToQueue(boost::move(fut), this);
		}
		this_thread::sleep_for(chrono::milliseconds(5));
	}
}
