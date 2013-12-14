#pragma once
#ifndef INC_CMYSQLCONNECTION_H
#define INC_CMYSQLCONNECTION_H


#include <string>
#include <boost/atomic.hpp>

using std::string;
using boost::atomic;


#ifdef WIN32
	#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"


class CMySQLConnection
{
public:
	static CMySQLConnection *Create(string &host, string &user, string &passwd, string &db, size_t port, bool auto_reconnect);
	void Destroy();

	//(dis)connect to the MySQL server
	void Connect();
	void Disconnect();

	//escape a string to dest
	void EscapeString(const char *src, string &dest);

	atomic<bool> IsInUse;

	inline MYSQL *GetMySQLPointer() const
	{
		return m_Connection;
	}

	inline bool GetAutoReconnect() const
	{
		return m_AutoReconnect;
	}
	inline bool IsConnected() const
	{
		return m_IsConnected;
	}

	inline bool operator==(CMySQLConnection &rhs)
	{
		return (rhs.m_Host.compare(m_Host) == 0 && rhs.m_User.compare(m_User) == 0 && rhs.m_Database.compare(m_Database) == 0 && rhs.m_Passw.compare(m_Passw) == 0);
	}

private:
	CMySQLConnection(string &host, string &user, string &passw, string &db, size_t port, bool auto_reconnect)
		: m_Host(host),
		m_User(user),
		m_Passw(passw),
		m_Database(db),
		m_Port(port),

		m_IsConnected(false),
		m_AutoReconnect(auto_reconnect),

		m_Connection(NULL),

		IsInUse(false)
	{ }
	~CMySQLConnection()
	{ }


	//MySQL server login values
	string
		m_Host,
		m_User,
		m_Passw,
		m_Database;
	size_t m_Port;

	//connection status
	bool m_IsConnected;

	//automatic reconnect
	bool m_AutoReconnect;

	//internal MYSQL pointer
	MYSQL *m_Connection;
};


#endif // INC_CMYSQLCONNECTION_H
