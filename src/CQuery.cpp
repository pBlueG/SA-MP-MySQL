#include "CResult.hpp"
#include "mysql.hpp"
#include "CQuery.hpp"


bool CQuery::Execute(MYSQL *connection)
{
	int error = 0;

	default_clock::time_point exec_timepoint = default_clock::now();
	error = mysql_real_query(connection, m_Query.c_str(), m_Query.length());
	default_clock::duration exec_time = default_clock::now() - exec_timepoint;

	if (error != 0)
	{
		CLog::Get()->Log(LogLevel::ERROR, m_DbgInfo, "(error #{}) {}",
			mysql_errno(connection), mysql_error(connection));
		return false;
	}
	CLog::Get()->Log(LogLevel::INFO, "query \"{}\" successfully executed", m_Query);
	
	m_Result = CResultSet::Create(connection, exec_time, m_Query);
	return m_Result != nullptr;
}
