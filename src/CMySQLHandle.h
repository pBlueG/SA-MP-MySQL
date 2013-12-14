#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H


#include <string>
#include <boost/thread/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <queue>
#include <boost/atomic.hpp>

using std::string;
using boost::unordered_map;
using boost::unordered_set;
using boost::thread;
using boost::tuple;
using boost::function;
using std::queue;
using boost::atomic;
namespace this_thread = boost::this_thread;


#include "main.h"

#include "CMySQLResult.h"


class CMySQLQuery;
class CMySQLConnection;


#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	CLog::Get()->LogFunction(LOG_ERROR, #function, "invalid connection handle (ID = %d)", id)


class CMySQLHandle 
{
public:


	//freezes the thread until all pending queries are executed
	void WaitForQueryExec();

	//returns main MySQL connection
	inline CMySQLConnection *GetMainConnection() const 
	{
		return m_MainConnection;
	}

	void ExecuteOnConnections(void (CMySQLConnection::*func)());

	inline void QueueQuery(function< CMySQLQuery (CMySQLConnection*) > func, bool use_pool = false)
	{
		m_QueryQueue.push(boost::move(boost::make_tuple(boost::move(func), use_pool)));
	}
	

	//checks if handle exists by id
	static inline bool IsValid(unsigned int id) 
	{
		return (SQLHandle.find(id) != SQLHandle.end());
	}


	//fabric function
	static CMySQLHandle *Create(string host, string user, string pass, string db, size_t port, size_t pool_size, bool reconnect);
	//delete function, call this instead of delete operator!
	void Destroy();
	//returns MySQL handle by id
	static inline CMySQLHandle *GetHandle(unsigned int cid) 
	{
		return SQLHandle.at(cid);
	}
	//returns connection id
	inline unsigned int GetID() const 
	{
		return m_MyID;
	}
	//returns number of unprocessed queries
	inline unsigned int GetUnprocessedQueryCount() const 
	{
		return m_QueryCounter;
	}
	inline void DecreaseQueryCounter()
	{
		m_QueryCounter--;
	}


	void SetActiveResult(CMySQLResult *result);
	
	unsigned int SaveActiveResult();
	bool DeleteSavedResult(unsigned int resultid);
	bool SetActiveResult(unsigned int resultid);
	bool IsValidResult(unsigned int resultid);
	inline CMySQLResult *GetActiveResult() 
	{
		return m_ActiveResult;
	}
	inline bool IsActiveResultSaved() const 
	{
		return m_ActiveResultID > 0 ? true : false;
	}


	void ExecThreadStashFunc();

	static void ClearAll();

	static inline CMySQLHandle *GetActiveHandle()
	{
		return ActiveHandle;
	}

private:
	CMySQLHandle(unsigned int id);
	~CMySQLHandle();

	static CMySQLHandle *ActiveHandle;
	static unordered_map<unsigned int, CMySQLHandle *> SQLHandle;

	
	queue< tuple< function<CMySQLQuery(CMySQLConnection*)>, bool> > m_QueryQueue;
	thread m_QueryStashThread;
	atomic<bool> m_QueryThreadRunning;
	atomic<unsigned int> m_QueryCounter;

	unordered_map<unsigned int, CMySQLResult*> m_SavedResults;

	CMySQLResult *m_ActiveResult;
	unsigned int m_ActiveResultID; //ID of stored result; 0 if not stored yet

	unsigned int m_MyID;

	CMySQLConnection *m_MainConnection; //only used in main thread
	CMySQLConnection *m_ThreadConnection; //for normal threaded queries
	unordered_set<CMySQLConnection*> m_ConnectionPool;
};


struct CMySQLOptions
{
	CMySQLOptions() :
		DuplicateConnections(false)
	{}
	bool DuplicateConnections;
};
extern struct CMySQLOptions MySQLOptions;


enum E_DATATYPE
{
	DATATYPE_INT,
	DATATYPE_FLOAT,
	DATATYPE_STRING
};

enum E_MYSQL_OPTION	
{
	DUPLICATE_CONNECTIONS
};


#endif // INC_CMYSQLHANDLE_H
