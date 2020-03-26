#include "mysql.hpp"

#include "CQuery.hpp"
#include "CConnection.hpp"
#include "CDispatcher.hpp"
#include "COptions.hpp"
#include "CLog.hpp"


CConnection::CConnection(const char *host, const char *user, const char *passw,
						 const char *db, const COptions *options)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::CConnection(this={}, host='{}', user='{}', passw='****', db='{}', options={})",
					 static_cast<const void *>(this),
					 host ? host : "(nullptr)",
					 user ? user : "(nullptr)",
					 db ? db : "(nullptr)",
					 static_cast<const void *>(options));

	assert(options != nullptr);

	//initialize
	m_Connection = mysql_init(nullptr);
	if (m_Connection == nullptr)
	{
		CLog::Get()->Log(LogLevel::ERROR,
						 "CConnection::CConnection - MySQL initialization failed " \
						 "(not enough memory available)");
		return;
	}

	if (options->GetOption<bool>(COptions::Type::SSL_ENABLE))
	{
		string
			key = options->GetOption<string>(COptions::Type::SSL_KEY_FILE),
			cert = options->GetOption<string>(COptions::Type::SSL_CERT_FILE),
			ca = options->GetOption<string>(COptions::Type::SSL_CA_FILE),
			capath = options->GetOption<string>(COptions::Type::SSL_CA_PATH),
			cipher = options->GetOption<string>(COptions::Type::SSL_CIPHER);


		mysql_ssl_set(m_Connection,
					  key.empty() ? nullptr : key.c_str(),
					  cert.empty() ? nullptr : cert.c_str(),
					  ca.empty() ? nullptr : ca.c_str(),
					  capath.empty() ? nullptr : capath.c_str(),
					  cipher.empty() ? nullptr : cipher.c_str());
	}

	//prepare connection flags through passed options
	unsigned long connect_flags = 0;
	if (options->GetOption<bool>(COptions::Type::MULTI_STATEMENTS) == true)
		connect_flags |= CLIENT_MULTI_STATEMENTS;

	//connect
	auto *result = mysql_real_connect(m_Connection, host, user, passw, db,
					options->GetOption<unsigned int>(COptions::Type::SERVER_PORT),
					nullptr, connect_flags);

	if (result == nullptr)
	{
		CLog::Get()->Log(LogLevel::ERROR,
						 "CConnection::CConnection - establishing connection to " \
						 "MySQL database failed: #{} '{}'",
						 mysql_errno(m_Connection), mysql_error(m_Connection));
		return;
	}

	m_IsConnected = true;

	//set additional connection options
	bool reconnect = options->GetOption<bool>(COptions::Type::AUTO_RECONNECT);
	mysql_options(m_Connection, MYSQL_OPT_RECONNECT, &reconnect);

	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::CConnection - new connection = {}",
					 static_cast<const void *>(m_Connection));
}

CConnection::~CConnection()
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::~CConnection(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	if (IsConnected())
		mysql_close(m_Connection);
}


bool CConnection::EscapeString(const char *src, string &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::EscapeString(src='{}', this={}, connection={})",
					 src ? src : "(nullptr)",
					 static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	if (IsConnected() == false || src == nullptr)
		return false;

	const size_t src_len = strlen(src);

	dest.resize(src_len * 2 + 1);

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	size_t newsize = mysql_real_escape_string(m_Connection, &dest[0], src, src_len);
	dest.resize(newsize);
	return true;
}

bool CConnection::SetCharset(string charset)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::SetCharset(charset='{}', this={}, connection={})",
					 charset, static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	if (IsConnected() == false || charset.empty())
		return false;

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	int error = mysql_set_character_set(m_Connection, charset.c_str());
	if (error != 0)
		return false;

	return true;
}

bool CConnection::GetCharset(string &charset)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CConnection::GetCharset(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	if (IsConnected() == false)
		return false;

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	charset = mysql_character_set_name(m_Connection);

	return true;
}

bool CConnection::Execute(Query_t query)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::Execute(query={}, this={}, connection={})",
					 static_cast<const void *>(query.get()),
					 static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	return IsConnected() && query->Execute(m_Connection);
}

bool CConnection::GetError(unsigned int &id, string &msg)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::GetError(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	if (m_Connection == nullptr)
		return false;

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	id = mysql_errno(m_Connection);
	msg = mysql_error(m_Connection);

	return true;
}

bool CConnection::GetStatus(string &stat)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnection::GetStatus(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(m_Connection));

	if (IsConnected() == false)
		return false;

	std::lock_guard<std::mutex> lock_guard(m_Mutex);
	const char *stat_raw = mysql_stat(m_Connection);

	if (stat_raw == nullptr)
		return false;

	stat = stat_raw;
	return true;
}



CThreadedConnection::CThreadedConnection(
	const char *host, const char *user, const char *passw, const char *db,
	const COptions *options)
	:
	m_Connection(host, user, passw, db, options),
	m_UnprocessedQueries(0),
	m_WorkerThreadActive(true),
	m_WorkerThread(std::bind(&CThreadedConnection::WorkerFunc, this))
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CThreadedConnection::CThreadedConnection(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(&m_Connection));
}

void CThreadedConnection::WorkerFunc()
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CThreadedConnection::WorkerFunc(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(&m_Connection));

	std::unique_lock<std::mutex> lock(m_QueueNotifierMutex);

	mysql_thread_init();

	while (m_WorkerThreadActive)
	{
		m_QueueNotifier.wait(lock);
		Query_t query;
		while (m_Queue.pop(query))
		{
			DispatchFunction_t func;
			if (m_Connection.Execute(query))
			{
				func = std::bind(&CQuery::CallCallback, query);
			}
			else
			{
				unsigned int errorid = 0;
				string error;
				m_Connection.GetError(errorid, error);
				func = std::bind(&CQuery::CallErrorCallback, query, errorid, error);
			}

			--m_UnprocessedQueries;
			CDispatcher::Get()->Dispatch(std::move(func));
		}
	}

	CLog::Get()->Log(LogLevel::DEBUG,
					 "CThreadedConnection::WorkerFunc(this={}, connection={}) - shutting down",
					 static_cast<const void *>(this),
					 static_cast<const void *>(&m_Connection));

	mysql_thread_end();
}

bool CThreadedConnection::Queue(Query_t query)
{
	if (!m_Queue.push(query))
		return false;

	++m_UnprocessedQueries;
	m_QueueNotifier.notify_one();
	return true;
}

CThreadedConnection::~CThreadedConnection()
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CThreadedConnection::~CThreadedConnection(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(&m_Connection));

	{
		std::lock_guard<std::mutex> lock_guard(m_QueueNotifierMutex);
		m_WorkerThreadActive = false;
	}
	m_QueueNotifier.notify_one();
	m_WorkerThread.join();
}



CConnectionPool::CConnectionPool(
	const size_t size, const char *host, const char *user, const char *passw,
	const char *db, const COptions *options)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnectionPool::CConnectionPool(size={}, this={})",
					 size, static_cast<const void *>(this));

	assert(size != 0);

	std::lock_guard<std::mutex> lock_guard(m_PoolMutex);

	SConnectionNode *node = m_CurrentNode = new SConnectionNode;

	for (size_t i = 0; i != size; ++i)
	{
		SConnectionNode *old_node = node;
		old_node->Connection = new CThreadedConnection(host, user, passw,
													   db, options);
		old_node->Next = ((i + 1) == size) ? m_CurrentNode : (node = new SConnectionNode);
	}
}

bool CConnectionPool::Queue(Query_t query)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnectionPool::Queue(query={}, this={})",
					 static_cast<const void *>(query.get()),
					 static_cast<const void *>(this));

	std::lock_guard<std::mutex> lock_guard(m_PoolMutex);
	auto *connection = m_CurrentNode->Connection;

	m_CurrentNode = m_CurrentNode->Next;
	assert(m_CurrentNode != nullptr && connection != nullptr);

	return connection->Queue(query);
}

bool CConnectionPool::SetCharset(string charset)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnectionPool::SetCharset(charset='{}', this={})",
					 charset, static_cast<const void *>(this));

	std::lock_guard<std::mutex> lock_guard(m_PoolMutex);
	SConnectionNode *node = m_CurrentNode;

	do
	{
		if (node->Connection->SetCharset(charset) == false)
			return false;

	}
	while ((node = node->Next) != m_CurrentNode);

	return true;
}

unsigned int CConnectionPool::GetUnprocessedQueryCount()
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnectionPool::GetUnprocessedQueryCount(this={})",
					 static_cast<const void *>(this));

	std::lock_guard<std::mutex> lock_guard(m_PoolMutex);
	SConnectionNode *node = m_CurrentNode;

	unsigned int count = 0;
	do
	{
		count += node->Connection->GetUnprocessedQueryCount();
	}
	while ((node = node->Next) != m_CurrentNode);

	return count;
}

CConnectionPool::~CConnectionPool()
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CConnectionPool::~CConnectionPool(this={})",
					 static_cast<const void *>(this));

	std::lock_guard<std::mutex> lock_guard(m_PoolMutex);
	SConnectionNode *node = m_CurrentNode;

	do
	{
		delete node->Connection;

		auto *old_node = node;
		node = node->Next;
		delete old_node;
	}
	while (node != m_CurrentNode);
}
