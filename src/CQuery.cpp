#include "CResult.hpp"
#include "mysql.hpp"
#include "CQuery.hpp"


bool CQuery::Execute(MYSQL *connection)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CQuery::Execute(this={}, connection={})",
		static_cast<const void *>(this), static_cast<const void *>(connection));

	int error = 0;

	default_clock::time_point exec_timepoint = default_clock::now();
	error = mysql_real_query(connection, m_Query.c_str(), m_Query.length());
	default_clock::duration exec_time = default_clock::now() - exec_timepoint;

	if (error != 0)
	{
		const char *error_str = mysql_error(connection);
		CLog::Get()->Log(LogLevel::ERROR, m_DbgInfo, 
			"error #{} while executing query \"{}\": {}",
			mysql_errno(connection), m_Query, error_str ? error_str : "(nullptr)");
		return false;
	}
	CLog::Get()->Log(LogLevel::INFO, "query \"{}\" successfully executed", m_Query);
	
	m_Result = CResultSet::Create(connection, exec_time, m_Query);
	return m_Result != nullptr;
}
