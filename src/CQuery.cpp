#include "CResult.hpp"
#include "mysql.hpp"
#include "CQuery.hpp"


bool CQuery::Execute(MYSQL *connection)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CQuery::Execute(this={}, connection={})",
					 static_cast<const void *>(this),
					 static_cast<const void *>(connection));

	int error = 0;

	default_clock::time_point exec_timepoint = default_clock::now();
	error = mysql_real_query(connection, m_Query.c_str(), m_Query.length());
	default_clock::duration exec_time = default_clock::now() - exec_timepoint;

	if (error != 0)
	{
		const char *error_str = mysql_error(connection);
		string msg = fmt::format("error #{} while executing query \"{}\": {}",
			mysql_errno(connection), m_Query,
			error_str ? error_str : "(nullptr)");

		if (!m_DbgInfo.empty())
			CLog::Get()->Log(LogLevel::ERROR, m_DbgInfo, msg.c_str());
		else
			CLog::Get()->Log(LogLevel::ERROR, msg.c_str());
		return false;
	}

	auto
		query_exec_time_milli = std::chrono::duration_cast<std::chrono::milliseconds>(exec_time).count(),
		query_exec_time_micro = std::chrono::duration_cast<std::chrono::microseconds>(exec_time).count();

	CLog::Get()->Log(LogLevel::INFO,
		"query \"{}\" successfully executed within {}.{} milliseconds",
		m_Query, query_exec_time_milli,
		query_exec_time_micro - (query_exec_time_milli * 1000));

	m_Result = CResultSet::Create(connection, exec_time, m_Query);
	return m_Result != nullptr;
}
