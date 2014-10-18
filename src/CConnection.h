#pragma once

#include <string>

using std::string;

class CQuery;
typedef struct st_mysql MYSQL;


class CConnection
{
public: //constructor / deconstructor
	CConnection(const string &host, const string &user, const string &passw, const string &db,
		size_t port, bool auto_reconnect);
	~CConnection();


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
