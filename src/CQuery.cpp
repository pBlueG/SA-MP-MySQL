#include "CResult.hpp"
#include "mysql.hpp"
#include "CQuery.hpp"


bool CQuery::Execute(MYSQL *connection)
{
	//TODO: error logging
	if (mysql_real_query(connection, m_Query.c_str(), m_Query.length()) != 0)
	{
		CLog::Get()->Log(LOGLEVEL::ERROR, m_DbgInfo, "(error #{}) {}",
			mysql_errno(connection), mysql_error(connection));
		return false;
	}
	CLog::Get()->Log(LOGLEVEL::INFO, "query \"{}\" successfully executed", m_Query);
	
	m_Result = CResultSet::Create(connection);
	return m_Result != nullptr;
}
