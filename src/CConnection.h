#pragma once

#include <string>
#include <forward_list>
#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

using std::string;
using std::forward_list;

class CQuery;
typedef struct st_mysql MYSQL;


class CConnection
{
public: //constructor / deconstructor
	CConnection(const string &host, const string &user, const string &passw, const string &db,
		size_t port, bool auto_reconnect);
	~CConnection();
	CConnection(const CConnection &rhs) = delete;

private: //variables
	bool m_IsConnected = false;
	MYSQL *m_Connection = nullptr;

public: //functions
	inline bool IsConnected() const
	{
		return m_IsConnected && m_Connection != nullptr;
	}
	bool EscapeString(const char *src, string &dest);
	bool SetCharset(string charset);
	bool Execute(CQuery *query);

};

class CThreadedConnection
{
public:
	CThreadedConnection(
		const string &host, const string &user, const string &passw, const string &db, size_t port);
	~CThreadedConnection();
	CThreadedConnection(const CThreadedConnection &rhs) = delete;

private:
	CConnection m_Connection;

	boost::thread m_WorkerThread;
	boost::atomic<bool> m_WorkerThreadActive;

	boost::lockfree::spsc_queue < CQuery *,
		boost::lockfree::fixed_sized < true >,
		boost::lockfree::capacity < 32768 >> m_Queue;

public:
	inline bool Queue(CQuery *query)
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
		const size_t size, const string &host, const string &user, const string &passw, const string &db, size_t port);
	~CConnectionPool();
	CConnectionPool(const CConnectionPool &rhs) = delete;

private:
	using Pool_t = forward_list<CThreadedConnection *>;

	Pool_t m_Pool;
	Pool_t::iterator m_PoolPos;

public:
	bool Queue(CQuery *query);
	bool SetCharset(string charset);

};
