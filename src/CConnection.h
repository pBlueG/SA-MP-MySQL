#pragma once

#include <string>
#include <forward_list>
#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

using std::string;
using std::forward_list;

class CQuery;
typedef struct st_mysql MYSQL;
class COptions;


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
	bool Execute(CQuery::Type_t query);

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

	boost::lockfree::spsc_queue < CQuery::Type_t,
		boost::lockfree::fixed_sized < true >,
		boost::lockfree::capacity < 32768 >> m_Queue;

public:
	inline bool Queue(CQuery::Type_t query)
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
	using Pool_t = forward_list<CThreadedConnection *>;

	Pool_t m_Pool;
	Pool_t::iterator m_PoolPos;

public:
	bool Queue(CQuery::Type_t query);
	bool SetCharset(string charset);

};
