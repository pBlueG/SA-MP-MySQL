#include "mysql.hpp"

#include "CQuery.hpp"
#include "CConnection.hpp"
#include "CDispatcher.hpp"
#include "COptions.hpp"
#include "CLog.hpp"


CConnection::CConnection(const string &host, const string &user, const string &passw, const string &db,
	const COptions *options)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, 
		"CConnection::CConnection(this={}, host='{}', user='{}', passw='****', db='{}', options={})",
		static_cast<const void *>(this), host, user, passw, db, static_cast<const void *>(options));

	assert(options != nullptr);

	//initialize
	m_Connection = mysql_init(nullptr);
	if (m_Connection == nullptr)
	{
		CLog::Get()->Log(LOGLEVEL::ERROR, 
			"CConnection::CConnection - MySQL initialization failed (not enough memory available)");
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
	auto *result = mysql_real_connect(m_Connection, host.c_str(),
		user.c_str(), passw.c_str(), db.c_str(), 
		options->GetOption<unsigned int>(COptions::Type::SERVER_PORT), 
		nullptr, connect_flags);

	if (result == nullptr)
	{
		CLog::Get()->Log(LOGLEVEL::ERROR, 
			"CConnection::CConnection - establishing connection to MySQL database failed: #{} '{}'",
			mysql_errno(m_Connection), mysql_error(m_Connection));
		return;
	}

	//set additional connection options
	my_bool reconnect = options->GetOption<bool>(COptions::Type::AUTO_RECONNECT);
	mysql_options(m_Connection, MYSQL_OPT_RECONNECT, &reconnect);

	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::CConnection - new connection = {}",
		static_cast<const void *>(m_Connection));
}

CConnection::~CConnection()
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::~CConnection(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(m_Connection));

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	if (IsConnected())
		mysql_close(m_Connection);
}


bool CConnection::EscapeString(const char *src, string &dest)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::EscapeString(src='{}', this={}, connection={})",
		src, static_cast<const void *>(this), static_cast<const void *>(m_Connection));

	if (IsConnected() == false || src == nullptr)
		return false;

	const size_t src_len = strlen(src);
	char *tmp_str = static_cast<char *>(malloc((src_len * 2 + 1) * sizeof(char)));

	//TODO: mutex?
	mysql_real_escape_string(m_Connection, tmp_str, src, src_len);
	dest.assign(tmp_str);
	free(tmp_str);

	return true;
}

bool CConnection::SetCharset(string charset)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::SetCharset(charset='{}', this={}, connection={})",
		charset, static_cast<const void *>(this), static_cast<const void *>(m_Connection));

	if (IsConnected() == false || charset.empty())
		return false;

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	int error = mysql_set_character_set(m_Connection, charset.c_str());
	if (error != 0)
		return false;

	return true;
}

bool CConnection::GetCharset(string &charset)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::GetCharset(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(m_Connection));

	if (IsConnected() == false)
		return false;

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	charset = mysql_character_set_name(m_Connection);

	return true;
}

bool CConnection::Execute(Query_t query)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::Execute(query={}, this={}, connection={})",
		static_cast<const void *>(query.get()), static_cast<const void *>(this), 
		static_cast<const void *>(m_Connection));

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	return IsConnected() && query->Execute(m_Connection);
}

bool CConnection::GetError(unsigned int &id, string &msg)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::GetError(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(m_Connection));

	if (IsConnected() == false)
		return false;

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	id = mysql_errno(m_Connection);
	msg = mysql_error(m_Connection);

	return true;
}

bool CConnection::GetStatus(string &stat)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnection::GetStatus(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(m_Connection));

	if (IsConnected() == false)
		return false;

	boost::lock_guard<boost::mutex> lock_guard(m_Mutex);
	const char *stat_raw = mysql_stat(m_Connection);
	
	if (stat_raw == nullptr)
		return false;

	stat = stat_raw;
	return true;
}



CThreadedConnection::CThreadedConnection(
	const string &host, const string &user, const string &passw, const string &db, 
	const COptions *options)
	:
	m_Connection(host, user, passw, db, options),
	m_WorkerThreadActive(true),
	m_WorkerThread(std::bind(&CThreadedConnection::WorkerFunc, this))
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CThreadedConnection::CThreadedConnection(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(&m_Connection));
}

void CThreadedConnection::WorkerFunc()
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CThreadedConnection::WorkerFunc(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(&m_Connection));

	mysql_thread_init();

	while (m_WorkerThreadActive)
	{
		Query_t query;
		while (m_Queue.pop(query))
		{
			if (m_Connection.Execute(query))
			{
				CDispatcher::Get()->Dispatch(std::bind(&CQuery::CallCallback, query));
			}
			else
			{
				// TODO: OnQueryError callback
			}
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}

	mysql_thread_end();
}

CThreadedConnection::~CThreadedConnection()
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CThreadedConnection::~CThreadedConnection(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(&m_Connection));

	m_WorkerThreadActive = false;
	m_WorkerThread.join();
	assert(m_Queue.empty());
}



CConnectionPool::CConnectionPool(
	const size_t size, const string &host, const string &user, const string &passw, const string &db, 
	const COptions *options)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnectionPool::(size={}, this={})",
		size, static_cast<const void *>(this));

	SConnectionNode *node = m_CurrentNode = new SConnectionNode;
	for (size_t i = 0; i != size; ++i)
	{
		node->Connection = new CThreadedConnection(host, user, passw, db, options);
		node->Next = ((i + 1) != size) ? m_CurrentNode : (node = new SConnectionNode);
	}
}

bool CConnectionPool::Queue(Query_t query)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnectionPool::Queue(query={}, this={})",
		static_cast<const void *>(query.get()), static_cast<const void *>(this));

	boost::lock_guard<boost::mutex> lock_guard(m_PoolMutex);
	auto *connection = m_CurrentNode->Connection;

	m_CurrentNode = m_CurrentNode->Next;
	assert(m_CurrentNode != nullptr && connection != nullptr);

	return connection->Queue(query);
}

bool CConnectionPool::SetCharset(string charset)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnectionPool::SetCharset(charset='{}', this={})",
		charset, static_cast<const void *>(this));

	boost::lock_guard<boost::mutex> lock_guard(m_PoolMutex);
	SConnectionNode *node = m_CurrentNode;

	do
	{
		if (node->Connection->SetCharset(charset) == false)
			return false;

	} while ((node = node->Next) != m_CurrentNode);

	return true;
}

CConnectionPool::~CConnectionPool()
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CConnectionPool::~CConnectionPool(this={})",
		static_cast<const void *>(this));

	boost::lock_guard<boost::mutex> lock_guard(m_PoolMutex);
	SConnectionNode *node = m_CurrentNode;

	do
	{
		delete node->Connection;

		auto *old_node = node;
		node = node->Next;
		delete old_node;
	} while (node != m_CurrentNode);
}
