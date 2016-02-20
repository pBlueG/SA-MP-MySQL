#include "CPreparedStmt.hpp"


bool CPreparedStmt::Execute(MYSQL *connection)
{
	//TODO: we have to close the statement somehow...
	//TODO: error logging, debug info, ...
	if (m_Stmt == nullptr)
	{
		m_Stmt = mysql_stmt_init(connection);
		if (m_Stmt == nullptr) // out of memory
			return false;

		if (mysql_stmt_prepare(m_Stmt, m_Query.c_str(), m_Query.length()))
		{
			fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(m_Stmt));
			return false;
		}

		unsigned int max_idx = 0;
		for (auto &p : m_Params)
			if (max_idx < p.column_index)
				max_idx = p.column_index;

		unsigned long param_count = mysql_stmt_param_count(m_Stmt);

		if (param_count != (max_idx + 1) )
		{
			fprintf(stderr, " invalid parameter count\n");
			return false;
		}

		MYSQL_BIND *bind = static_cast<MYSQL_BIND *>(calloc(max_idx + 1, sizeof(MYSQL_BIND)));
		for (auto &p : m_Params)
		{
			unsigned int idx = p.column_index;
			bind[idx].buffer = p.addr;
			bind[idx].buffer_type = p.type;
			bind[idx].buffer_length = p.max_size;
			bind[idx].length = p.length_addr;
			bind[idx].is_null = reinterpret_cast<my_bool *>(p.isnull_addr);
		}


		if (mysql_stmt_bind_param(m_Stmt, bind))
		{
			fprintf(stderr, " mysql_stmt_bind_param() failed\n");
			fprintf(stderr, " %s\n", mysql_stmt_error(m_Stmt));
			return false;
		}

		free(bind);
	}

	if (mysql_stmt_execute(m_Stmt))
	{
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(m_Stmt));
		return false;
	}
	
	/*affected_rows = mysql_stmt_affected_rows(stmt);
	fprintf(stdout, " total affected rows(insert 1): %lu\n",
		(unsigned long)affected_rows);
	*/
	return true;
}
