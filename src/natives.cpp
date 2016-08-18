#include "natives.hpp"
#include "CQuery.hpp"
#include "CHandle.hpp"
#include "CCallback.hpp"
#include "CResult.hpp"
#include "COptions.hpp"
#include "COrm.hpp"
#include "CLog.hpp"
#include "misc.hpp"

#include <fstream>
#include <fmt/printf.h>


// native ORM:orm_create(const table[], MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::orm_create)
{
	CScopedDebugInfo dbg_info(amx, "orm_create", "sd");
	const char *tablename = nullptr;
	amx_StrParam(amx, params[1], tablename);
	const HandleId_t handleid = params[2];

	CError<COrm> orm_error;
	OrmId_t orm_id = COrmManager::Get()->Create(handleid, tablename, orm_error);
	if (orm_error)
	{
		CLog::Get()->LogNative(orm_error);
		return 0;
	}

	cell ret_val = orm_id;
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_destroy(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_destroy)
{
	CScopedDebugInfo dbg_info(amx, "orm_destroy", "d");
	const OrmId_t ormid = params[1];

	if (COrmManager::Get()->IsValid(ormid) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}


	cell ret_val = COrmManager::Get()->Delete(ormid);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native E_ORM_ERROR:orm_errno(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_errno)
{
	CScopedDebugInfo dbg_info(amx, "orm_errno", "d");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	cell ret_val = static_cast<cell>(orm->GetError());
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_apply_cache(ORM:id, row_idx, resultset_idx = 0);
AMX_DECLARE_NATIVE(Native::orm_apply_cache)
{
	CScopedDebugInfo dbg_info(amx, "orm_apply_cache", "ddd");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	ResultSet_t active_resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (!active_resultset)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active resultset");
		return 0;
	}

	auto const res = active_resultset->GetResultByIndex(params[3]);
	if (res == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid resultset index '{}'", params[3]);
		return 0;
	}

	if (orm->ApplyResultByName(res, params[2]) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row index index '{}'", params[2]);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

static bool FireOrmQueryWithCallback(AMX *amx, cell *params, COrm::QueryType type)
{
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return false;
	}

	Handle_t handle = CHandleManager::Get()->GetHandle(orm->GetHandleId());
	if (!handle)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "handle id '{}' passed to orm instance is invalid",
							   orm->GetHandleId());
		return false;
	}

	string
		callback_str = amx_GetCppString(amx, params[2]),
		format_str = amx_GetCppString(amx, params[3]);

	CError<CCallback> callback_error;
	Callback_t callback = CCallback::Create(
		amx,
		callback_str.c_str(),
		format_str.c_str(),
		params, 4,
		callback_error);

	if (callback_error && callback_error.type() != CCallback::Error::EMPTY_NAME)
	{
		CLog::Get()->LogNative(callback_error);
		return false;
	}

	if (type == COrm::QueryType::SAVE)
		type = orm->GetSaveQueryType();

	string query_str;
	CError<COrm> error;
	if ((error = orm->GenerateQuery(type, query_str)))
	{
		CLog::Get()->LogNative(error);
		return false;
	}

	CLog::Get()->LogNative(LogLevel::INFO, "generated query \"{}\"", query_str);

	Query_t query = CQuery::Create(query_str);
	query->OnExecutionFinished([orm, callback, type](ResultSet_t result)
	{
		switch (type)
		{
			case COrm::QueryType::SELECT:
				orm->ApplyResult(result->GetActiveResult());
				break;
			case COrm::QueryType::INSERT:
				orm->UpdateKeyValue(result->GetActiveResult());
				break;
		}

		if (callback)
			callback->Execute();

		orm->ResetError();
	});

	return handle->Execute(CHandle::ExecutionType::THREADED, query);
}

// native orm_select(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_select)
{
	CScopedDebugInfo dbg_info(amx, "orm_select", "dss");
	cell ret_val = FireOrmQueryWithCallback(amx, params, COrm::QueryType::SELECT);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_update(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_update)
{
	CScopedDebugInfo dbg_info(amx, "orm_update", "dss");
	cell ret_val = FireOrmQueryWithCallback(amx, params, COrm::QueryType::UPDATE);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_insert(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_insert)
{
	CScopedDebugInfo dbg_info(amx, "orm_insert", "dss");
	cell ret_val = FireOrmQueryWithCallback(amx, params, COrm::QueryType::INSERT);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_delete(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_delete)
{
	CScopedDebugInfo dbg_info(amx, "orm_delete", "dss");
	cell ret_val = FireOrmQueryWithCallback(amx, params, COrm::QueryType::DELETE);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_save(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_save)
{
	CScopedDebugInfo dbg_info(amx, "orm_save", "dss");
	cell ret_val = FireOrmQueryWithCallback(amx, params, COrm::QueryType::SAVE);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native orm_addvar_int(ORM:id, &var, const columnname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar_int)
{
	CScopedDebugInfo dbg_info(amx, "orm_addvar_int", "drs");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	cell *var = nullptr;
	amx_GetAddr(amx, params[2], &var);

	const char *column_name = nullptr;
	amx_StrParam(amx, params[3], column_name);

	CError<COrm> error;
	if ((error = orm->AddVariable(
		COrm::Variable::Type::INT, column_name, var)))
	{
		CLog::Get()->LogNative(error);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native orm_addvar_float(ORM:id, &Float:var, const columnname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar_float)
{
	CScopedDebugInfo dbg_info(amx, "orm_addvar_float", "drs");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	cell *var = nullptr;
	amx_GetAddr(amx, params[2], &var);

	const char *column_name = nullptr;
	amx_StrParam(amx, params[3], column_name);

	CError<COrm> error;
	if ((error = orm->AddVariable(
		COrm::Variable::Type::FLOAT, column_name, var)))
	{
		CLog::Get()->LogNative(error);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native orm_addvar_string(ORM:id, var[], var_maxlen, const columnname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar_string)
{
	CScopedDebugInfo dbg_info(amx, "orm_addvar_string", "drds");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	cell *var = nullptr;
	amx_GetAddr(amx, params[2], &var);

	const char *column_name = nullptr;
	amx_StrParam(amx, params[4], column_name);

	CError<COrm> error;
	if ((error = orm->AddVariable(
		COrm::Variable::Type::STRING, column_name, var, params[3])))
	{
		CLog::Get()->LogNative(error);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native orm_delvar(ORM:id, const columnname[]);
AMX_DECLARE_NATIVE(Native::orm_delvar)
{
	CScopedDebugInfo dbg_info(amx, "orm_delvar", "ds");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	const char *column_name = nullptr;
	amx_StrParam(amx, params[2], column_name);
	if (column_name == nullptr || strlen(column_name) == 0)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "empty column name");
		return 0;
	}


	CError<COrm> error;
	if ((error = orm->RemoveVariable(column_name)))
	{
		CLog::Get()->LogNative(error);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native orm_clear_vars(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_clear_vars)
{
	CScopedDebugInfo dbg_info(amx, "orm_clear_vars", "d");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}


	orm->ClearAllVariables();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native orm_setkey(ORM:id, const columnname[]);
AMX_DECLARE_NATIVE(Native::orm_setkey)
{
	CScopedDebugInfo dbg_info(amx, "orm_setkey", "ds");
	const OrmId_t ormid = params[1];
	Orm_t orm = COrmManager::Get()->Find(ormid);

	if (!orm)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid orm id '{}'", ormid);
		return 0;
	}

	const char *column_name = nullptr;
	amx_StrParam(amx, params[2], column_name);
	if (column_name == nullptr || strlen(column_name) == 0)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "empty column name");
		return 0;
	}


	CError<COrm> error;
	if ((error = orm->SetKeyVariable(column_name)))
	{
		CLog::Get()->LogNative(error);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}



// native mysql_log(E_LOGLEVEL:loglevel = ERROR | WARNING);
AMX_DECLARE_NATIVE(Native::mysql_log)
{
	CLog::Get()->SetLogLevel(static_cast<LogLevel>(params[1]));
	return 1;
}

// native MySQL:mysql_connect(const host[], const user[], const password[], 
//							  const database[], MySQLOpt:option_id = MySQLOpt:0);
AMX_DECLARE_NATIVE(Native::mysql_connect)
{
	CScopedDebugInfo dbg_info(amx, "mysql_connect", "ss*sd");
	OptionsId_t options_id = static_cast<OptionsId_t>(params[5]);
	auto *options = COptionManager::Get()->GetOptionHandle(options_id);
	if (options == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid option id '{}'", options_id);
		return 0;
	}

	const char
		*host_str = nullptr,
		*user_str = nullptr,
		*passwd_str = nullptr,
		*db_str = nullptr;

	amx_StrParam(amx, params[1], host_str);
	amx_StrParam(amx, params[2], user_str);
	amx_StrParam(amx, params[3], passwd_str);
	amx_StrParam(amx, params[4], db_str);

	CError<CHandle> handle_error;
	Handle_t handle = CHandleManager::Get()->Create(
		host_str, user_str, passwd_str, db_str,
		options, handle_error);

	if (handle_error)
	{
		CLog::Get()->LogNative(handle_error);
		return 0;
	}

	assert(handle != nullptr);

	cell ret_val = handle->GetId();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native MySQL:mysql_connect_file(const file_name[] = "mysql.ini");
AMX_DECLARE_NATIVE(Native::mysql_connect_file)
{
	CScopedDebugInfo dbg_info(amx, "mysql_connect_file", "s");
	string file_name = amx_GetCppString(amx, params[1]);
	//no directory seperators allowed
	if (file_name.find_first_of("/\\") != string::npos)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "file \"{}\" not in SA-MP root folder", file_name);
		return 0;
	}

	CError<CHandle> handle_error;
	Handle_t handle = CHandleManager::Get()->CreateFromFile(file_name, 
															handle_error);

	if (handle_error)
	{
		CLog::Get()->LogNative(handle_error);
		return 0;
	}

	assert(handle != nullptr);

	cell ret_val = handle->GetId();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_close(MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_close)
{
	CScopedDebugInfo dbg_info(amx, "mysql_close", "d");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	cell ret_val = CHandleManager::Get()->Destroy(handle);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_unprocessed_queries(MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_unprocessed_queries)
{
	CScopedDebugInfo dbg_info(amx, "mysql_unprocessed_queries", "d");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	cell ret_val = handle->GetUnprocessedQueryCount();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_global_options(E_MYSQL_GLOBAL_OPTION:type, value);
AMX_DECLARE_NATIVE(Native::mysql_global_options)
{
	CScopedDebugInfo dbg_info(amx, "mysql_global_options", "dd");
	using OptEnum = COptionManager::GlobalOption;
	OptEnum option = static_cast<OptEnum>(params[1]);
	switch (option)
	{
		//boolean type
		case OptEnum::DUPLICATE_CONNECTIONS:
		case OptEnum::DUPLICATE_CONNECTION_WARNING:
			COptionManager::Get()->SetGlobalOption(
				option, static_cast<bool>(params[2] != 0));
			break;
		default:
			CLog::Get()->LogNative(LogLevel::ERROR, 
								   "unknown option type '{}'", params[1]);
			return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native MySQLOpt:mysql_init_options();
AMX_DECLARE_NATIVE(Native::mysql_init_options)
{
	CScopedDebugInfo dbg_info(amx, "mysql_init_options");
	cell ret_val = COptionManager::Get()->Create();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_set_option(MySQLOpt:option_id, E_MYSQL_OPTION:type, ...);
AMX_DECLARE_NATIVE(Native::mysql_set_option)
{
	CScopedDebugInfo dbg_info(amx, "mysql_set_option", "dd");
	OptionsId_t options_id = static_cast<OptionsId_t>(params[1]);
	auto *options = COptionManager::Get()->GetOptionHandle(options_id);
	if (options == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid option id '{}'", options_id);
		return 0;
	}

	if (params[0] == sizeof(cell) * 2)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no value specified");
		return 0;
	}

	cell *param_addr = nullptr;
	amx_GetAddr(amx, params[3], &param_addr);

	cell value = *param_addr;
	bool ret_val = false;

	const COptions::Type option = static_cast<COptions::Type>(params[2]);
	switch (option)
	{
		//boolean types
		case COptions::Type::AUTO_RECONNECT:
		case COptions::Type::MULTI_STATEMENTS:
		case COptions::Type::SSL_ENABLE:
			ret_val = options->SetOption(option, static_cast<bool>(value != 0));
			break;

		//string types
		case COptions::Type::SSL_KEY_FILE:
		case COptions::Type::SSL_CERT_FILE:
		case COptions::Type::SSL_CA_FILE:
		case COptions::Type::SSL_CA_PATH:
		case COptions::Type::SSL_CIPHER:
			ret_val = options->SetOption(option, amx_GetCppString(amx, value));
			break;

		//other types
		case COptions::Type::POOL_SIZE:
			if (value >= 0 && value <= 32)
				ret_val = options->SetOption(option, 
											 static_cast<unsigned int>(value));
			else
				CLog::Get()->LogNative(LogLevel::ERROR, 
									   "invalid pool size '{}'", value);
			break;
		case COptions::Type::SERVER_PORT:
			if (value >= 0 && value <= std::numeric_limits<unsigned short>::max())
				ret_val = options->SetOption(option,
											 static_cast<unsigned int>(value));
			else
				CLog::Get()->LogNative(LogLevel::ERROR, 
									   "invalid MySQL server port '{}'", value);
			break;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

static bool SendQueryWithCallback(AMX *amx, cell *params, 
								  CHandle::ExecutionType query_type)
{
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);

	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return false;
	}

	string
		callback_str = amx_GetCppString(amx, params[3]),
		format_str = amx_GetCppString(amx, params[4]);

	CError<CCallback> callback_error;
	Callback_t callback = CCallback::Create(
		amx,
		callback_str.c_str(),
		format_str.c_str(),
		params, 5,
		callback_error);

	if (callback_error && callback_error.type() != CCallback::Error::EMPTY_NAME)
	{
		CLog::Get()->LogNative(callback_error);
		return false;
	}


	string query_str = amx_GetCppString(amx, params[2]);
	Query_t query = CQuery::Create(query_str);
	if (callback != nullptr)
	{
		query->OnExecutionFinished([callback](ResultSet_t resultset)
		{
			CResultSetManager::Get()->SetActiveResultSet(resultset);

			callback->Execute();

			//unset active result(cache) + delete result (done by shared_ptr)
			CResultSetManager::Get()->SetActiveResultSet(nullptr);
		});
	}

	query->OnError(
		[amx, handle_id, callback_str, query_str](unsigned int errorid, 
												  string error)
	{
		CError<CCallback> error_cb_error;
		//forward OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle);
		Callback_t error_cb = CCallback::Create(error_cb_error, amx, 
												"OnQueryError", "dsssd",
												errorid, error.c_str(), 
												callback_str.c_str(), 
												query_str.c_str(), handle_id);

		if (!error_cb_error)
			error_cb->Execute();
	});

	return handle->Execute(query_type, query);
}

// native mysql_pquery(MySQL:handle, const query[], const callback[] = "", 
//					   const format[] = "", {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_pquery)
{
	CScopedDebugInfo dbg_info(amx, "mysql_pquery", "dsss");

	cell ret_val = SendQueryWithCallback(amx, params, 
										 CHandle::ExecutionType::PARALLEL);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_tquery(MySQL:handle, const query[], const callback[] = "",
//					   const format[] = "", {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_tquery)
{
	CScopedDebugInfo dbg_info(amx, "mysql_tquery", "dsss");

	cell ret_val = SendQueryWithCallback(amx, params, 
										 CHandle::ExecutionType::THREADED);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native Cache:mysql_query(MySQL:handle, const query[], bool:use_cache = true);
AMX_DECLARE_NATIVE(Native::mysql_query)
{
	CScopedDebugInfo dbg_info(amx, "mysql_query", "dsd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);

	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	Query_t query = CQuery::Create(amx_GetCppString(amx, params[2]));
	cell ret_val = 0;

	if (handle->Execute(CHandle::ExecutionType::UNTHREADED, query) 
		&& params[3] != 0)
	{
		CResultSetManager::Get()->SetActiveResultSet(query->GetResult());
		ret_val = CResultSetManager::Get()->StoreActiveResultSet();
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_query_file(MySQL:handle, const file_path[]);
AMX_DECLARE_NATIVE(Native::mysql_query_file)
{
	CScopedDebugInfo dbg_info(amx, "mysql_query_file", "ds");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);

	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string filename = amx_GetCppString(amx, params[2]);
	std::ifstream file(filename);
	if (file.fail())
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "can't open file '{}'", filename);
		return 0;
	}

	string query_str;
	while (file.good())
	{
		string tmp_query_str;
		std::getline(file, tmp_query_str);

		if (tmp_query_str.empty())
			continue;

		/*
		 * check for comments (start with "-- " or "#")
		 * a query could look like this: 
		 *   "SELECT stuff FROM table; -- selects # records"
		 * that's why we search for both comment specifiers 
		 * and check for which comes first
		 * NOTE: we don't process C-style multiple-line comments, 
		 *		 because the MySQL server handles them in a special way
		 */
		size_t comment_pos = std::min(tmp_query_str.find("-- "), 
									  tmp_query_str.find('#'));
		if (comment_pos != string::npos)
			tmp_query_str.erase(comment_pos);

		size_t sem_pos;
		while ((sem_pos = tmp_query_str.find(';')) != string::npos)
		{
			query_str.append(tmp_query_str.substr(0U, sem_pos + 1));
			tmp_query_str.erase(0, sem_pos + 1);

			handle->Execute(CHandle::ExecutionType::UNTHREADED, 
							CQuery::Create(query_str));
			query_str.clear();
		}

		query_str.append(tmp_query_str);
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native mysql_errno(MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_errno)
{
	CScopedDebugInfo dbg_info(amx, "mysql_errno", "d");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return -1;
	}

	unsigned int errorid = 0;
	if (handle->GetErrorId(errorid) == false)
		return -1;

	cell ret_val = errorid;
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_format(MySQL:handle, output[], len, 
//					   const format[], {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_format)
{
	CScopedDebugInfo dbg_info(amx, "mysql_format", "drds");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}


	const size_t dest_maxsize = params[3];
	const char *format_str = nullptr;
	amx_StrParam(amx, params[4], format_str);

	if (format_str == nullptr || params[3] <= 0)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid format string or destination size ({})",
							   params[3]);
		return 0;
	}

	fmt::MemoryWriter dest_writer;

	const unsigned int
		first_param_idx = 5,
		num_args = (params[0] / sizeof(cell)),
		num_dyn_args = num_args - (first_param_idx - 1);
	unsigned int param_counter = 0;

	for (; *format_str != '\0'; ++format_str)
	{
		bool break_loop = false;

		if (dest_writer.size() >= dest_maxsize)
		{
			CLog::Get()->LogNative(LogLevel::ERROR, 
								   "destination size '{}' is too small",
								   dest_maxsize);
			break;
		}

		if (*format_str == '%')
		{
			++format_str;

			if (*format_str == '%')
			{
				dest_writer << '%';
				continue;
			}

			if (param_counter >= num_dyn_args)
			{
				CLog::Get()->LogNative(LogLevel::ERROR, 
									   "no value for specifier '%{}' passed",
									   *format_str);
				break;
			}

			const char *format_spec_ptr = format_str - 1;
			while (!isalpha(*format_str))
				++format_str;
			string format_spec(format_spec_ptr, format_str + 1);

			cell *amx_address = nullptr;
			amx_GetAddr(amx, params[first_param_idx + param_counter], 
						&amx_address);

			switch (*format_str)
			{
				case 'd':
				case 'i':
				case 'o':
				case 'x':
				case 'X':
				case 'u':
					dest_writer << fmt::sprintf(format_spec,
												static_cast<int>(*amx_address));
					break;
				case 's':
					dest_writer << amx_GetCppString(amx, 
										params[first_param_idx + param_counter]);
					break;
				case 'f':
				case 'F':
				case 'a':
				case 'A':
				case 'g':
				case 'G':
					dest_writer << fmt::sprintf(format_spec, 
												amx_ctof(*amx_address));
					break;
				case 'e':
				{
					char *source_str = nullptr;
					amx_StrParam(amx, params[first_param_idx + param_counter], 
								 source_str);

					string escaped_str;
					if (source_str != nullptr 
						&& handle->EscapeString(source_str, escaped_str))
					{
						dest_writer << escaped_str;
					}
					else
					{
						CLog::Get()->LogNative(LogLevel::ERROR, 
										"can't escape string '{}'",
										source_str ? source_str : "(nullptr)");
						break_loop = true;
					}
					break;
				}
				case 'b':
				{
					string bin_str;
					ConvertDataToStr<int, 2>(*amx_address, bin_str);
					dest_writer << bin_str;
					break;
				}
				default:
					CLog::Get()->LogNative(LogLevel::ERROR, 
										   "invalid format specifier '%{}'", 
										   *format_str);
					// can't break out of loop from within a switch
					break_loop = true;
			}

			if (break_loop)
				break;
			else
				param_counter++;
		}
		else
		{
			dest_writer << *format_str;
		}
	}

	cell ret_val = 0;
	if (*format_str == '\0') //loop exited normally
	{
		ret_val = static_cast<cell>(dest_writer.size());
		amx_SetCString(amx, params[2], dest_writer.c_str(), dest_maxsize);
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_escape_string(const source[], destination[], 
//							  max_len = sizeof(destination), 
//							  MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_escape_string)
{
	CScopedDebugInfo dbg_info(amx, "mysql_escape_string", "srdd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[4]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	char *unescaped_str = nullptr;
	amx_StrParam(amx, params[1], unescaped_str);

	string escaped_str;
	if (unescaped_str != nullptr 
		&& handle->EscapeString(unescaped_str, escaped_str) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "can't escape string '{}'", escaped_str);
		return 0;
	}

	size_t max_str_len = params[3] - 1;
	if (escaped_str.length() > max_str_len)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "destination array too small " \
							   "(needs at least '{}' cells; has only '{}')",
							   escaped_str.length() + 1, max_str_len + 1);
		return 0;
	}

	amx_SetCString(amx, params[2], escaped_str.c_str(), max_str_len + 1);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native mysql_set_charset(const charset[], MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_set_charset)
{
	CScopedDebugInfo dbg_info(amx, "mysql_set_charset", "sd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[2]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	cell ret_val = handle->SetCharacterSet(amx_GetCppString(amx, params[1]));
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_get_charset(destination[], max_len = sizeof(destination), 
//							MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_get_charset)
{
	CScopedDebugInfo dbg_info(amx, "mysql_get_charset", "rdd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[3]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string charset;
	if (handle->GetCharacterSet(charset) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "can't retreive character set");
		return 0;
	}

	size_t max_str_len = params[2] - 1;
	if (charset.length() > max_str_len)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "destination array too small " \
							   "(needs at least '{}' cells; has only '{}')",
							   charset.length() + 1, max_str_len + 1);
		return 0;
	}

	amx_SetCppString(amx, params[1], charset, max_str_len + 1);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native mysql_stat(destination[], max_len = sizeof(destination), 
//					 MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_stat)
{
	CScopedDebugInfo dbg_info(amx, "mysql_stat", "rdd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[3]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, 
							   "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string status;
	if (handle->GetStatus(status) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "can't retreive status");
		return 0;
	}

	size_t max_str_len = params[2] - 1;
	if (status.length() > max_str_len)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "destination array too small " \
							   "(needs at least '{}' cells; has only '{}')",
							   status.length() + 1, max_str_len + 1);
		return 0;
	}

	amx_SetCppString(amx, params[1], status, max_str_len + 1);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}



// native cache_get_row_count();
AMX_DECLARE_NATIVE(Native::cache_get_row_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_row_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = static_cast<cell>(resultset->GetActiveResult()->GetRowCount());
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_field_count();
AMX_DECLARE_NATIVE(Native::cache_get_field_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = resultset->GetActiveResult()->GetFieldCount();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_result_count();
AMX_DECLARE_NATIVE(Native::cache_get_result_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_result_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = resultset->GetResultCount();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_field_name(field_index, destination[], 
//							   max_len = sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_field_name)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_name", "drd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	string field_name;
	if (resultset->GetActiveResult()->GetFieldName(params[1], field_name) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid index '{}'", params[1]);
		return 0;
	}

	amx_SetCppString(amx, params[2], field_name, params[3]);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native E_MYSQL_FIELD_TYPE:cache_get_field_type(field_index);
AMX_DECLARE_NATIVE(Native::cache_get_field_type)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_type", "d");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return -1;
	}

	enum_field_types type;
	if (resultset->GetActiveResult()->GetFieldType(params[1], type) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid index '{}'", params[1]);
		return -1;
	}

	cell ret_val = type;
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_set_result(result_index);
AMX_DECLARE_NATIVE(Native::cache_set_result)
{
	CScopedDebugInfo dbg_info(amx, "cache_set_result", "d");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = resultset->SetActiveResult(params[1]);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}


// native cache_get_value_index(row_idx, column_idx, destination[], 
//								max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_value_index)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_value_index", "ddrd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	const char *data = nullptr;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row ('{}') or field ('{}') index", 
							   params[1], params[2]);
		return 0;
	}

	if (data == nullptr) //NULL value
		data = "NULL";

	CLog::Get()->LogNative(LogLevel::DEBUG, "assigned value: '{}'", data);

	amx_SetCString(amx, params[3], data, params[4]);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_value_index_int(row_idx, column_idx, &destination);
AMX_DECLARE_NATIVE(Native::cache_get_value_index_int)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_value_index_int", "ddr");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell *dest_addr = nullptr;
	if (amx_GetAddr(amx, params[3], &dest_addr) != AMX_ERR_NONE 
		|| dest_addr == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid reference passed");
		return 0;
	}

	const char *data = nullptr;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row ('{}') or field ('{}') index", 
							   params[1], params[2]);
		return 0;
	}

	if (ConvertStrToData<cell>(data, *dest_addr) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "value '{}' is not a number",
							   data ? data : "NULL");
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "assigned value: '{}'", *dest_addr);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_value_index_float(row_idx, column_idx, &Float:destination);
AMX_DECLARE_NATIVE(Native::cache_get_value_index_float)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_value_index_float", "ddr");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell *dest_addr = nullptr;
	if (amx_GetAddr(amx, params[3], &dest_addr) != AMX_ERR_NONE 
		|| dest_addr == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid reference passed");
		return 0;
	}

	const char *data = nullptr;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row ('{}') or field ('{}') index", 
							   params[1], params[2]);
		return 0;
	}

	if (ConvertStrToData<float>(data, amx_ctof(*dest_addr)) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "value '{}' is not a number",
							   data ? data : "NULL");
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, 
						   "assigned value: '{}'", amx_ctof(*dest_addr));

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_is_value_index_null(row_idx, column_idx, &bool:destination);
AMX_DECLARE_NATIVE(Native::cache_is_value_index_null)
{
	CScopedDebugInfo dbg_info(amx, "cache_is_value_index_null", "ddr");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell *dest_addr = nullptr;
	if (amx_GetAddr(amx, params[3], &dest_addr) != AMX_ERR_NONE || dest_addr == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid reference passed");
		return 0;
	}

	const char *data = nullptr;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row ('{}') or field ('{}') index", 
							   params[1], params[2]);
		return 0;
	}

	*dest_addr = (data == nullptr) ? 1 : 0;
	CLog::Get()->LogNative(LogLevel::DEBUG, "assigned value: '{}'", *dest_addr);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_value_name(row_idx, const column_name[], destination[], 
//							   max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_value_name)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_value_name", "dsrd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	const string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row index '{}' (number of rows: '{}')", 
							   row_idx, result->GetRowCount());
		return 0;
	}

	const char *data = nullptr;
	if (result->GetRowDataByName(row_idx, field_name, &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	if (data == nullptr) //NULL value
		data = "NULL";

	CLog::Get()->LogNative(LogLevel::DEBUG, "assigned value: '{}'", data);

	amx_SetCString(amx, params[3], data, params[4]);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_value_name_int(row_idx, const column_name[], &destination);
AMX_DECLARE_NATIVE(Native::cache_get_value_name_int)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_value_name_int", "dsr");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell *dest_addr = nullptr;
	if (amx_GetAddr(amx, params[3], &dest_addr) != AMX_ERR_NONE 
		|| dest_addr == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid reference passed");
		return 0;
	}

	const string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row index '{}' (number of rows: '{}')", 
							   row_idx, result->GetRowCount());
		return 0;
	}

	const char *data = nullptr;
	if (result->GetRowDataByName(row_idx, field_name, &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	if (ConvertStrToData<cell>(data, *dest_addr) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "value '{}' is not a number",
							   data ? data : "NULL");
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "assigned value: '{}'", *dest_addr);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_value_name_float(row_idx, const column_name[], &Float:destination);
AMX_DECLARE_NATIVE(Native::cache_get_value_name_float)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_value_name_float", "dsr");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell *dest_addr = nullptr;
	if (amx_GetAddr(amx, params[3], &dest_addr) != AMX_ERR_NONE 
		|| dest_addr == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid reference passed");
		return 0;
	}

	const string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row index '{}' (number of rows: '{}')", 
							   row_idx, result->GetRowCount());
		return 0;
	}

	const char *data = nullptr;
	if (result->GetRowDataByName(row_idx, field_name, &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	if (ConvertStrToData<float>(data, amx_ctof(*dest_addr)) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "value '{}' is not a number",
							   data ? data : "NULL");
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, 
						   "assigned value: '{}'", amx_ctof(*dest_addr));

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_is_value_name_null(row_idx, const column_name[], &bool:destination);
AMX_DECLARE_NATIVE(Native::cache_is_value_name_null)
{
	CScopedDebugInfo dbg_info(amx, "cache_is_value_name_null", "dsr");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	cell *dest_addr = nullptr;
	if (amx_GetAddr(amx, params[3], &dest_addr) != AMX_ERR_NONE 
		|| dest_addr == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid reference passed");
		return 0;
	}

	const string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LogLevel::ERROR,
							   "invalid row index '{}' (number of rows: '{}')", 
							   row_idx, result->GetRowCount());
		return 0;
	}

	const char *data = nullptr;
	if (result->GetRowDataByName(row_idx, field_name, &data) == false)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	*dest_addr = (data == nullptr) ? 1 : 0;
	CLog::Get()->LogNative(LogLevel::DEBUG, "assigned value: '{}'", *dest_addr);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native Cache:cache_save();
AMX_DECLARE_NATIVE(Native::cache_save)
{
	CScopedDebugInfo dbg_info(amx, "cache_save", "");

	cell ret_val = CResultSetManager::Get()->StoreActiveResultSet();
	if (ret_val == 0)
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_delete(Cache:cache_id);
AMX_DECLARE_NATIVE(Native::cache_delete)
{
	CScopedDebugInfo dbg_info(amx, "cache_delete", "d");
	if (!CResultSetManager::Get()->DeleteResultSet(params[1]))
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid cache id '{}'", params[1]);
		return 0;
	}

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native cache_set_active(Cache:cache_id);
AMX_DECLARE_NATIVE(Native::cache_set_active)
{
	CScopedDebugInfo dbg_info(amx, "cache_set_active", "d");
	auto resultset = CResultSetManager::Get()->GetResultSet(params[1]);
	if (!resultset)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "invalid cache id '{}'", params[1]);
		return 0;
	}

	CResultSetManager::Get()->SetActiveResultSet(resultset);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}

// native bool:cache_is_any_active();
AMX_DECLARE_NATIVE(Native::cache_is_any_active)
{
	CScopedDebugInfo dbg_info(amx, "cache_is_any_active", "");
	bool ret_val = (CResultSetManager::Get()->GetActiveResultSet() != nullptr);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val ? 1 : 0;
}

// native bool:cache_is_valid(Cache:cache_id);
AMX_DECLARE_NATIVE(Native::cache_is_valid)
{
	CScopedDebugInfo dbg_info(amx, "cache_is_valid", "d");
	bool ret_val = CResultSetManager::Get()->IsValidResultSet(params[1]);
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val ? 1 : 0;
}

// native cache_affected_rows();
AMX_DECLARE_NATIVE(Native::cache_affected_rows)
{
	CScopedDebugInfo dbg_info(amx, "cache_affected_rows", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = static_cast<cell>(resultset->GetActiveResult()->AffectedRows());
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_warning_count();
AMX_DECLARE_NATIVE(Native::cache_warning_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_warning_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = resultset->GetActiveResult()->WarningCount();
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_insert_id();
AMX_DECLARE_NATIVE(Native::cache_insert_id)
{
	CScopedDebugInfo dbg_info(amx, "cache_insert_id", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = static_cast<cell>(resultset->GetActiveResult()->InsertId());
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_query_exec_time(E_EXECTIME_UNIT:unit = MICROSECONDS);
AMX_DECLARE_NATIVE(Native::cache_get_query_exec_time)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_query_exec_time", "d");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = static_cast<cell>(
		resultset->GetExecutionTime(static_cast<CResultSet::TimeType>(params[1])));
	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_query_string(destination[], max_len = sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_query_string)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_query_string", "rd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LogLevel::ERROR, "no active cache");
		return 0;
	}

	amx_SetCppString(amx, params[1], resultset->GetExecutedQuery(), params[2]);

	CLog::Get()->LogNative(LogLevel::DEBUG, "return value: '1'");
	return 1;
}
