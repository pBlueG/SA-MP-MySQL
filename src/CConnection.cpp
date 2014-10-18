#include "CConnection.h"
#include "CQuery.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


CConnection::CConnection(const string &host, const string &user, const string &passw, const string &db,
	size_t port, bool auto_reconnect)
{
	m_Connection = mysql_init(NULL);
	if (m_Connection == NULL)
		return; //TODO: error "MySQL initialization failed"
	
	auto *result = mysql_real_connect(m_Connection, host.c_str(),
		user.c_str(), passw.c_str(), db.c_str(), port, NULL, NULL);

	if (result == NULL)
		return; //TODO: error "connection failed"
	

	my_bool reconnect = auto_reconnect;
	mysql_options(m_Connection, MYSQL_OPT_RECONNECT, &reconnect);
		
	m_IsConnected = true;
}

CConnection::~CConnection()
{
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
	

	int error = mysql_set_character_set(m_Connection, charset.c_str());
	if (error != 0)
		return false;

	return true;
}

bool CConnection::Execute(CQuery *query)
{
	return IsConnected() && query->Execute(m_Connection);
}
