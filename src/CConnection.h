#pragma once

#include <string>

using std::string;

class CQuery;
typedef struct st_mysql MYSQL;


class CConnection
{
private: //constructor / deconstructor
	CConnection(string &host, string &user, string &passw, string &db,
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

	inline bool QueueQuery(CQuery *query)
	{
		
	}


private: //functions
	void ProcessQueries();

};
