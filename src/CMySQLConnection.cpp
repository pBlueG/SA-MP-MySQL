#pragma once

#include "CMySQLConnection.h"
#include "CLog.h"


atomic<unsigned int> CMySQLConnection::_dummy_QueryCounter;


CMySQLConnection *CMySQLConnection::Create(string &host, string &user, string &passwd, string &db, unsigned int port, bool auto_reconnect,
	atomic<unsigned int> &query_counter, const unsigned int connection_id)
{
	return new CMySQLConnection(host, user, passwd, db, port, auto_reconnect, query_counter, connection_id);
}

void CMySQLConnection::Destroy()
{
	delete this;
}

void CMySQLConnection::Connect()
{
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Connect", "establishing connection to database...");

	if (m_Connection == NULL)
	{
		m_Connection = mysql_init(NULL);
		if (m_Connection == NULL)
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::Connect", "MySQL initialization failed");
	}

	if (!m_IsConnected && !mysql_real_connect(m_Connection, m_Host.c_str(), m_User.c_str(), m_Passw.c_str(), m_Database.c_str(), m_Port, NULL, NULL))
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

void CMySQLConnection::Disconnect()
{
	if (m_Connection == NULL)
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLConnection::Disconnect", "no connection available");
	else
	{
		mysql_close(m_Connection);
		m_Connection = NULL;
		m_IsConnected = false;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Disconnect", "connection was closed");
	}
}

void CMySQLConnection::EscapeString(const char *src, string &dest)
{
	if (src != NULL && m_IsConnected)
	{
		const size_t src_len = strlen(src);
		char *tmpEscapedStr = static_cast<char *>(malloc((src_len * 2 + 1) * sizeof(char)));

		mysql_real_escape_string(m_Connection, tmpEscapedStr, src, src_len);
		dest.assign(tmpEscapedStr);

		free(tmpEscapedStr);
	}
}