#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H


#include <string>
#include <boost/unordered_map.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

using std::string;
using boost::unordered_map;

#ifdef WIN32
	#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"

#include "main.h"


class CMySQLResult;
class CMySQLQuery;


#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	CLog::Get()->LogFunction(LOG_ERROR, #function, "invalid connection handle (ID = %d)", id), 0


class CMySQLConnection {
public:
	friend class CMySQLHandle;

	//(dis)connect to the MySQL server
	void Connect();
	void Disconnect();

	//escape a string to dest
	void EscapeString(const char *src, string &dest);

	inline MYSQL *GetMySQLPointer() {
		return m_Connection;
	}

	inline bool GetAutoReconnect() const {
		return m_AutoReconnect;
	}

	CMySQLConnection(string host, string user, string passw, string db, unsigned int port, bool auto_reconnect)
		:	m_Host(host),
			m_User(user),
			m_Passw(passw),
			m_Database(db),
			m_Port(port),

			m_IsConnected(false),
			m_AutoReconnect(auto_reconnect),

			m_Connection(NULL)
	{ }
	~CMySQLConnection()
	{ }
private:

	//MySQL server login values
	string
		m_Host,
		m_User,
		m_Passw,
		m_Database;
	unsigned int m_Port;

	//connection status
	bool m_IsConnected;

	//automatic reconnect
	bool m_AutoReconnect;

	//internal MYSQL pointer
	MYSQL *m_Connection;
};

class CMySQLHandle {
public:
	//freezes the thread until all pending queries are executed
	void WaitForQueryExec();

	//returns main MySQL connection
	inline CMySQLConnection *GetMainConnection() const {
		return m_MainConnection;
	}

	//returns MySQL connection for unthreaded queries
	inline CMySQLConnection *GetQueryConnection() const {
		return m_QueryConnection;
	}
	
	//checks if handle exists by id
	static inline bool IsValid(int id) {
		return (SQLHandle.find(id) != SQLHandle.end());
	}

	//schedules query
	inline bool ScheduleQuery(CMySQLQuery *query) {
		return m_QueryQueue.push(query);
	}
	//process queries
	void ProcessQueries();

	//fabric function
	static CMySQLHandle *Create(string host, string user, string pass, string db, size_t port, bool reconnect);
	//delete function, call this instead of delete operator!
	void Destroy();
	//returns MySQL handle by id
	static inline CMySQLHandle *GetHandle(int cid) {
		return SQLHandle.at(cid);
	}
	inline int GetID() const {
		return m_MyID;
	}


	void SetActiveResult(CMySQLResult *result);
	
	int SaveActiveResult();
	bool DeleteSavedResult(int resultid);
	bool SetActiveResult(int resultid);
	inline CMySQLResult *GetActiveResult() const {
		return m_ActiveResult;
	}
	inline bool IsActiveResultSaved() const {
		return m_ActiveResultID > 0 ? true : false;
	}

	
	static void ClearAll();
private:
	CMySQLHandle(int id);
	~CMySQLHandle();

	static unordered_map<int, CMySQLHandle *> SQLHandle;
	
	boost::atomic<bool> m_QueryThreadRunning;
	boost::thread *m_QueryThread;
	boost::lockfree::spsc_queue <
			CMySQLQuery *,
			boost::lockfree::capacity<16384> 
		> m_QueryQueue;

	unordered_map<int, CMySQLResult*> m_SavedResults;

	CMySQLResult *m_ActiveResult;
	int m_ActiveResultID; //ID of stored result; 0 if not stored yet

	int m_MyID;

	CMySQLConnection
		*m_MainConnection, //only used in main thread
		*m_QueryConnection; //used for unthreaded queries
};

enum E_DATATYPES 
{
	DATATYPE_INT,
	DATATYPE_FLOAT,
	DATATYPE_STRING
};


#endif // INC_CMYSQLHANDLE_H
