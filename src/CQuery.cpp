#include "CQuery.h"
#include "CResult.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


bool CQuery::Execute(MYSQL *connection)
{
	//TODO: error logging
	if (mysql_real_query(connection, m_Query.c_str(), m_Query.length()) != 0)
		return false;
	
	m_Result = CResultSet::Create(connection);
	return m_Result != nullptr;
}
