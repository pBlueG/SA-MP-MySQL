#pragma once

#include <string>
#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

using std::string;

#include "types.hpp"


class CConnection
{
public: //constructor / deconstructor
	CConnection(
		const string &host, const string &user, const string &passw, const string &db,
		const COptions *options);
	~CConnection();
	CConnection(const CConnection &rhs) = delete;

private: //variables
	bool m_IsConnected = false;
	MYSQL *m_Connection = nullptr;

	boost::mutex m_Mutex; //protect every MySQL C API call

public: //functions
	inline bool IsConnected() const
	{
		return m_IsConnected && m_Connection != nullptr;
	}
	bool EscapeString(const char *src, string &dest);
	bool SetCharset(string charset);
	bool GetCharset(string &charset);
	bool Execute(Query_t query);
	bool GetError(unsigned int &id, string &msg);
	bool GetStatus(string &stat);

};

class CThreadedConnection
{
public:
	CThreadedConnection(
		const string &host, const string &user, const string &passw, const string &db,
		const COptions *options);
	~CThreadedConnection();
	CThreadedConnection(const CThreadedConnection &rhs) = delete;

private:
	CConnection m_Connection;

	boost::thread m_WorkerThread;
	boost::atomic<bool> m_WorkerThreadActive;

	boost::lockfree::spsc_queue < Query_t,
		boost::lockfree::fixed_sized < true >,
		boost::lockfree::capacity < 32768 >> m_Queue;

private:
	void WorkerFunc();

public:
	inline bool Queue(Query_t query)
	{
		return m_Queue.push(query);
	}
	inline bool SetCharset(string charset)
	{
		return m_Connection.SetCharset(charset);
	}

};

class CConnectionPool
{
public:
	CConnectionPool(
		const size_t size, const string &host, const string &user, const string &passw, const string &db,
		const COptions *options);
	~CConnectionPool();
	CConnectionPool(const CConnectionPool &rhs) = delete;

private:
	struct SConnectionNode
	{
		CThreadedConnection *Connection;
		SConnectionNode *Next;
	};

	SConnectionNode *m_CurrentNode;
	boost::mutex m_PoolMutex;

public:
	bool Queue(Query_t query);
	bool SetCharset(string charset);

};
