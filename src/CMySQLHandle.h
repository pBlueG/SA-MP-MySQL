#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H


#include <string>
#include <thread>
#include <future>
#include <unordered_map>
#include <forward_list>
#include <tuple>
#include <functional>
#include <queue>
#include <atomic>

using std::string;
using std::unordered_map;
using std::forward_list;
using std::thread;
using std::future;
using std::tuple;
using std::function;
using std::queue;
using std::atomic;
namespace this_thread = std::this_thread;


#ifdef WIN32
	#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"

#include "main.h"

#include "CMySQLResult.h"


class CMySQLQuery;


#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	CLog::Get()->LogFunction(LOG_ERROR, #function, "invalid connection handle (ID = %d)", id)


class CMySQLConnection 
{
public:
	static CMySQLConnection *Create(string &host, string &user, string &passwd, string &db, size_t port, bool auto_reconnect);
	void Destroy();

	//(dis)connect to the MySQL server
	void Connect();
	void Disconnect();

	//escape a string to dest
	void EscapeString(const char *src, string &dest);

	inline void ToggleState(bool toggle)
	{
		m_State = toggle;
	}
	inline bool GetState() const
	{
		return m_State;
	}

	inline MYSQL *GetMySQLPointer() const
	{
		return m_Connection;
	}

	inline bool GetAutoReconnect() const 
	{
		return m_AutoReconnect;
	}
	inline bool IsConnected() const 
	{
		return m_IsConnected;
	}

	inline bool operator==(CMySQLConnection &rhs)
	{
		return (rhs.m_Host.compare(m_Host) == 0 && rhs.m_User.compare(m_User) == 0 && rhs.m_Database.compare(m_Database) == 0 && rhs.m_Passw.compare(m_Passw) == 0);
	}

private:
	CMySQLConnection(string &host, string &user, string &passw, string &db, size_t port, bool auto_reconnect)
		:	m_Host(host),
			m_User(user),
			m_Passw(passw),
			m_Database(db),
			m_Port(port),

			m_IsConnected(false),
			m_AutoReconnect(auto_reconnect),

			m_Connection(NULL),

			m_State(false)
	{ }
	~CMySQLConnection()
	{ }


	//MySQL server login values
	string
		m_Host,
		m_User,
		m_Passw,
		m_Database;
	size_t m_Port;

	//connection status
	bool m_IsConnected;

	//automatic reconnect
	bool m_AutoReconnect;

	//internal MYSQL pointer
	MYSQL *m_Connection;

	bool m_State;
};


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

	void ExecuteOnConnectionPool(void (CMySQLConnection::*func)());

	inline void QueueQuery(function<CMySQLQuery(CMySQLConnection*)> &&func)
	{
		m_QueryQueue.push(std::move(func));
	}
	

	//checks if handle exists by id
	static inline bool IsValid(int id) 
	{
		return (SQLHandle.find(id) != SQLHandle.end());
	}


	//fabric function
	static CMySQLHandle *Create(string host, string user, string pass, string db, size_t port, size_t pool_size, bool reconnect);
	//delete function, call this instead of delete operator!
	void Destroy();
	//returns MySQL handle by id
	static inline CMySQLHandle *GetHandle(int cid) 
	{
		return SQLHandle.at(cid);
	}
	//returns connection id
	inline int GetID() const 
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
	
	int SaveActiveResult();
	bool DeleteSavedResult(int resultid);
	bool SetActiveResult(int resultid);
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

	static CMySQLHandle *ActiveHandle;
private:
	CMySQLHandle(int id);
	~CMySQLHandle();

	static unordered_map<int, CMySQLHandle *> SQLHandle;
	
	queue< function<CMySQLQuery(CMySQLConnection*)> > m_QueryQueue;
	thread m_QueryStashThread;
	atomic<bool> m_QueryThreadRunning;
	atomic<unsigned int> m_QueryCounter;

	unordered_map<int, CMySQLResult*> m_SavedResults;

	CMySQLResult *m_ActiveResult;
	int m_ActiveResultID; //ID of stored result; 0 if not stored yet

	int m_MyID;

	CMySQLConnection *m_MainConnection; //only used in main thread
	forward_list<CMySQLConnection*> m_ConnectionPool;
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
