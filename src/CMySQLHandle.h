#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H


#include <string>
#include <boost/unordered_map.hpp>
#include <set>
#include <boost/function.hpp>
#include <boost/atomic.hpp>

using std::string;
using boost::unordered_map;
using std::set;
using boost::function;
using boost::atomic;


class CMySQLQuery;
class CMySQLConnection;
class CMySQLResult;


#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	CLog::Get()->LogFunction(LOG_ERROR, function, "invalid connection handle (id: %d)", id)


class CMySQLHandle 
{
public:
	//returns main MySQL connection
	inline CMySQLConnection *GetMainConnection() const 
	{
		return m_MainConnection;
	}
	void ExecuteOnConnections(function<void (CMySQLConnection *)> func);
	void QueueQuery(CMySQLQuery *query, bool use_pool = false);


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

	void SetActiveResult(CMySQLResult *result);
	

	unsigned int SaveActiveResult();
	bool DeleteSavedResult(unsigned int resultid);
	bool SetActiveResult(unsigned int resultid);
	inline bool IsValidResult(unsigned int resultid)
	{
		return (resultid != 0 && m_SavedResults.find(resultid) != m_SavedResults.end());
	}
	inline CMySQLResult *GetActiveResult() 
	{
		return m_ActiveResult;
	}
	inline bool IsActiveResultSaved() const 
	{
		return m_ActiveResultID > 0 ? true : false;
	}

	inline void DecreaseQueryCounter()
	{
		m_QueryCounter--;
	}

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


	atomic<unsigned int> m_QueryCounter;

	unordered_map<unsigned int, CMySQLResult*> m_SavedResults;

	CMySQLResult *m_ActiveResult;
	unsigned int m_ActiveResultID; //ID of stored result; 0 if not stored yet

	unsigned int m_MyID;

	CMySQLConnection *m_MainConnection; //only used in main thread
	CMySQLConnection *m_ThreadConnection; //for normal threaded queries
	set<CMySQLConnection *> m_ConnectionPool;
	set<CMySQLConnection *>::iterator m_CurrentConPoolPos;
};


struct CMySQLOptions
{
	CMySQLOptions() :
		DuplicateConnections(false),
		Log_TruncateData(true)
	{}
	bool DuplicateConnections;
	bool Log_TruncateData;
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
	DUPLICATE_CONNECTIONS,
	LOG_TRUNCATE_DATA
};


#endif // INC_CMYSQLHANDLE_H
