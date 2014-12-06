#include "CQuery.h"
#include "CConnection.h"
#include "CDispatcher.h"
#include "COptions.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


CConnection::CConnection(const string &host, const string &user, const string &passw, const string &db,
	const COptions *options)
{
	//boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	assert(options != nullptr);

	m_Connection = mysql_init(NULL);
	if (m_Connection == NULL)
		return; //TODO: error "MySQL initialization failed"
	
	auto *result = mysql_real_connect(m_Connection, host.c_str(),
		user.c_str(), passw.c_str(), db.c_str(), 
		options->GetOption<unsigned int>(COptions::Type::SERVER_PORT), 
		NULL, NULL); //TODO: option "MULTI_STATEMENTS"

	if (result == NULL)
		return; //TODO: error "connection failed"
	

	my_bool reconnect = options->GetOption<bool>(COptions::Type::AUTO_RECONNECT);
	mysql_options(m_Connection, MYSQL_OPT_RECONNECT, &reconnect);
	
	m_IsConnected = true;
}

CConnection::~CConnection()
{
	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	if (IsConnected())
		mysql_close(m_Connection);
}


bool CConnection::EscapeString(const char *src, string &dest)
{
	if (src == nullptr || IsConnected() == false)
		return false;


	const size_t src_len = strlen(src);
	char *tmp_str = static_cast<char *>(malloc((src_len * 2 + 1) * sizeof(char)));

	mysql_real_escape_string(m_Connection, tmp_str, src, src_len);
	dest.assign(tmp_str);
	free(tmp_str);

	return true;
}

bool CConnection::SetCharset(string charset)
{
	if (m_IsConnected == false || charset.empty())
		return false;
	

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	int error = mysql_set_character_set(m_Connection, charset.c_str());
	if (error != 0)
		return false;

	return true;
}

bool CConnection::Execute(CQuery::Type_t query)
{
	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	return IsConnected() && query->Execute(m_Connection);
}



CThreadedConnection::CThreadedConnection(
	const string &host, const string &user, const string &passw, const string &db, 
	const COptions *options)
	:
	m_Connection(host, user, passw, db, options),
	m_WorkerThreadActive(true),
	m_WorkerThread([this]()
	{
		mysql_thread_init();

		while (m_WorkerThreadActive)
		{
			CQuery::Type_t query;
			while (m_Queue.pop(query))
			{
				if (m_Connection.Execute(query))
				{
					CDispatcher::Get()->Dispatch(std::bind(&CQuery::CallCallback, query));
				}
			}
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		}

		mysql_thread_end();
	})
{

}

CThreadedConnection::~CThreadedConnection()
{
	m_WorkerThreadActive = false;
	m_WorkerThread.join();
	assert(m_Queue.empty());
}



CConnectionPool::CConnectionPool(
	const size_t size, const string &host, const string &user, const string &passw, const string &db, 
	const COptions *options)
{
	for (size_t i = 0; i < size; ++i)
		m_Pool.push_front(new CThreadedConnection(host, user, passw, db, options));
	m_PoolPos = m_Pool.begin();
}

bool CConnectionPool::Queue(CQuery::Type_t query)
{
	if (m_PoolPos == m_Pool.end())
		m_PoolPos = m_Pool.begin();

	auto *connection = *m_PoolPos;
	if (connection != nullptr)
	{
		m_PoolPos++;
		return connection->Queue(query);
	}
	return false;
}

bool CConnectionPool::SetCharset(string charset)
{
	for (auto *c : m_Pool)
		if (c == nullptr || c->SetCharset(charset) == false)
			return false;

	return true;
}

CConnectionPool::~CConnectionPool()
{
	for (auto *c : m_Pool)
		delete c;
}
