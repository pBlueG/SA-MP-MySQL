#include "CMySQLConnection.h"
#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "CCallback.h"
#include "CLog.h"

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
namespace this_thread = boost::this_thread;


CMySQLConnection *CMySQLConnection::Create(string &host, string &user, string &passwd, string &db, unsigned int port, bool auto_reconnect, bool threaded /*= true*/)
{
	return new CMySQLConnection(host, user, passwd, db, port, auto_reconnect, threaded);
}

void CMySQLConnection::Destroy()
{
	if(m_IsConnected)
		Disconnect();
	delete this;
}

CMySQLConnection::CMySQLConnection(string &host, string &user, string &passw, string &db, size_t port, bool auto_reconnect, bool threaded)
	: 
	m_QueryThreadRunning(true),
	m_QueryThread(NULL),

	m_Host(host),
	m_User(user),
	m_Passw(passw),
	m_Database(db),
	m_Port(port),

	m_IsConnected(false),
	m_AutoReconnect(auto_reconnect),

	m_Connection(NULL)
{
	if(threaded)
		m_QueryThread = new thread(boost::bind(&CMySQLConnection::ProcessQueries, this));
}

CMySQLConnection::~CMySQLConnection()
{
	if(m_QueryThread != NULL)
	{
		m_QueryThreadRunning = false;
		m_QueryThread->join();
		delete m_QueryThread;
	
		CMySQLQuery *query = NULL;
		while(m_QueryQueue.pop(query))
			delete query;
	}
}

bool CMySQLConnection::Connect()
{
	if(m_QueryThread != NULL && this_thread::get_id() != m_QueryThread->get_id()) //not in query thread and threaded: queue
	{
		boost::lock_guard<boost::mutex> lockguard(m_FuncQueueMtx);
		m_FuncQueue.push(boost::bind(&CMySQLConnection::Connect, this));
	}
	else //in query thread or unthreaded: execute
	{
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Connect", "establishing connection to database...");

		if (m_Connection == NULL)
		{
			m_Connection = mysql_init(NULL);
			if (m_Connection == NULL)
				CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::Connect", "MySQL initialization failed");
		}

		if (!m_IsConnected && !mysql_real_connect(m_Connection, m_Host.c_str(), m_User.c_str(), m_Passw.c_str(), m_Database.c_str(), m_Port, NULL, CLIENT_MULTI_RESULTS))
		{
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::Connect", "(error #%d) %s", mysql_errno(m_Connection), mysql_error(m_Connection));

			m_IsConnected = false;
		}
		else
		{
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Connect", "connection was successful");

			my_bool reconnect = m_AutoReconnect;
			mysql_options(m_Connection, MYSQL_OPT_RECONNECT, &reconnect);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Connect", "auto-reconnect has been %s", m_AutoReconnect == true ? "enabled" : "disabled");

			m_IsConnected = true;
		}
	}
	return true;
}

bool CMySQLConnection::Disconnect()
{
	if(m_QueryThread != NULL && this_thread::get_id() != m_QueryThread->get_id()) //not in query thread and threaded: queue
	{
		boost::lock_guard<boost::mutex> lockguard(m_FuncQueueMtx);
		m_FuncQueue.push(boost::bind(&CMySQLConnection::Disconnect, this));
	}
	else //in query thread or unthreaded: execute
	{
		if (!m_QueryQueue.empty())
			return false;
		
		if (m_Connection == NULL || m_IsConnected == false)
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLConnection::Disconnect", "no connection available");
		else
		{
			mysql_close(m_Connection);
			m_Connection = NULL;
			m_IsConnected = false;
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Disconnect", "connection was closed");
		}
	}
	return true;
}

bool CMySQLConnection::EscapeString(const char *src, string &dest)
{
	if (src != NULL && m_IsConnected)
	{
		const size_t src_len = strlen(src);
		char *tmpEscapedStr = static_cast<char *>(malloc((src_len * 2 + 1) * sizeof(char)));

		mysql_real_escape_string(m_Connection, tmpEscapedStr, src, src_len);
		dest.assign(tmpEscapedStr);

		free(tmpEscapedStr);
	}
	return true;
}

bool CMySQLConnection::SetCharset(string charset)
{
	if(m_QueryThread != NULL && this_thread::get_id() != m_QueryThread->get_id()) //not in query thread and threaded: queue
	{
		boost::lock_guard<boost::mutex> lockguard(m_FuncQueueMtx);
		m_FuncQueue.push(boost::bind(&CMySQLConnection::SetCharset, this, charset));
	}
	else //in query thread or unthreaded: execute
	{
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::SetCharset", "setting charset \"%s\"", charset.c_str());
		if (m_IsConnected && !charset.empty())
		{
			int error = mysql_set_character_set(m_Connection, charset.c_str());
			if (error == 0)
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::SetCharset", "charset \"%s\" has been set", charset.c_str());
			else
				CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::SetCharset", "error %d: %s", error, mysql_error(m_Connection));
		}
		else
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::SetCharset", "invalid charset (\"%s\") or not connected", charset.c_str());
	}
	return true;
}

void CMySQLConnection::ProcessQueries()
{
	mysql_thread_init();
	while(m_QueryThreadRunning)
	{
		vector<function<bool()> > tmp_queue;
		m_FuncQueueMtx.lock();
		while (m_FuncQueue.size() > 0)
		{
			if (m_FuncQueue.front()() == false)
				tmp_queue.push_back(m_FuncQueue.front());
			m_FuncQueue.pop();
		}
		m_FuncQueueMtx.unlock();

		if(m_IsConnected)
		{
			CMySQLQuery *query;
			while(m_QueryQueue.pop(query))
			{
				if(query->Execute(m_Connection) == false)
				{
					if (m_AutoReconnect && mysql_errno(m_Connection) == 2006)
					{
						CLog::Get()->LogFunction(LOG_WARNING, "CMySQLConnection::ProcessQueries", "lost connection, reconnecting...");

						MYSQL_RES *mysql_result;
						if ((mysql_result = mysql_store_result(m_Connection)) != NULL)
							mysql_free_result(mysql_result);

						Disconnect();
						Connect();
					}
				}
				CCallback::Get()->QueueQuery(query);
			}
		}

		m_FuncQueueMtx.lock();
		for (size_t i = 0; i < tmp_queue.size(); ++i)
			m_FuncQueue.push(tmp_queue.at(i));
		m_FuncQueueMtx.unlock();

		this_thread::sleep_for(chrono::milliseconds(10));
	}
	mysql_thread_end();
}
