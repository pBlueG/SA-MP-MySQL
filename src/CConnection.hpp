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
		const char *host, const char *user, const char *passw, const char *db,
		const COptions *options);
	~CConnection();
	CConnection(const CConnection &rhs) = delete;

private: //variables
	MYSQL *m_Connection = nullptr;
	bool m_IsConnected = false;

	boost::mutex m_Mutex; //protect every MySQL C API call

public: //functions
	inline bool IsConnected() const
	{
		return m_Connection != nullptr && m_IsConnected;
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
		const char *host, const char *user, const char *passw, const char *db,
		const COptions *options);
	~CThreadedConnection();
	CThreadedConnection(const CThreadedConnection &rhs) = delete;

private:
	CConnection m_Connection;

	boost::thread m_WorkerThread;
	boost::atomic<bool> m_WorkerThreadActive;

	boost::lockfree::spsc_queue < Query_t,
		boost::lockfree::fixed_sized < true >,
		boost::lockfree::capacity < 65536 >> m_Queue;

	boost::atomic<unsigned int> m_UnprocessedQueries;

private:
	void WorkerFunc();

public:
	inline bool Queue(Query_t query)
	{
		return m_Queue.push(query) && ++m_UnprocessedQueries;
	}
	inline bool SetCharset(string charset)
	{
		return m_Connection.SetCharset(charset);
	}
	inline unsigned int GetUnprocessedQueryCount()
	{
		return m_UnprocessedQueries;
	}

};

class CConnectionPool
{
public:
	CConnectionPool(
		const size_t size, const char *host, const char *user, const char *passw, const char *db,
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
	unsigned int GetUnprocessedQueryCount();

};
