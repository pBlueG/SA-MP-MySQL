#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
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
	std::mutex m_Mutex; //protect every MySQL C API call

	MYSQL *m_Connection = nullptr;
	bool m_IsConnected = false;

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

	std::mutex m_QueueNotifierMutex;
	std::condition_variable m_QueueNotifier;
	boost::lockfree::spsc_queue < Query_t,
		boost::lockfree::fixed_sized < true >,
		boost::lockfree::capacity < 65536 >> m_Queue;

	std::atomic<unsigned int> m_UnprocessedQueries;

	std::atomic<bool> m_WorkerThreadActive;
	std::thread m_WorkerThread;

private:
	void WorkerFunc();

public:
	bool Queue(Query_t query);
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
		const size_t size, const char *host, const char *user,
		const char *passw, const char *db,
		const COptions *options);
	~CConnectionPool();
	CConnectionPool(const CConnectionPool &rhs) = delete;

private:
	std::mutex m_PoolMutex;

	struct SConnectionNode
	{
		CThreadedConnection *Connection;
		SConnectionNode *Next;
	};

	SConnectionNode *m_CurrentNode;

public:
	bool Queue(Query_t query);
	bool SetCharset(string charset);
	unsigned int GetUnprocessedQueryCount();

};
