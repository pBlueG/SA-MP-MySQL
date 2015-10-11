#include "natives.hpp"
#include "CQuery.hpp"
#include "CHandle.hpp"
#include "CCallback.hpp"
#include "CResult.hpp"
#include "COptions.hpp"
#include "CLog.hpp"
#include "misc.hpp"

#include <fstream>


// native ORM:orm_create(const table[], MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::orm_create)
{
	return 0;
}

// native orm_destroy(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_destroy)
{
	return 0;
}

// native ORM_Error:orm_errno(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_errno)
{
	return 0;
}

// native orm_apply_cache(ORM:id, row);
AMX_DECLARE_NATIVE(Native::orm_apply_cache)
{
	return 0;
}

// native orm_select(ORM:id, callback[], format[], {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_select)
{
	return 0;
}

// native orm_update(ORM:id);
AMX_DECLARE_NATIVE(Native::orm_update)
{
	return 0;
}

// native orm_insert(ORM:id, callback[]="", format[]="", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_insert)
{
	return 0;
}

// native orm_delete(ORM:id, bool:clearvars=true);
AMX_DECLARE_NATIVE(Native::orm_delete)
{
	return 0;
}

// native orm_save(ORM:id, callback[]="", format[]="", {Float, _}:...);
AMX_DECLARE_NATIVE(Native::orm_save)
{
	return 0;
}

// native orm_addvar_int(ORM:id, &var, varname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar_int)
{
	return 0;
}

// native orm_addvar_float(ORM:id, &Float:var, varname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar_float)
{
	return 0;
}

// native orm_addvar_string(ORM:id, var[], var_maxlen, varname[]);
AMX_DECLARE_NATIVE(Native::orm_addvar_string)
{
	return 0;
}

// native orm_delvar(ORM:id, varname[]);
AMX_DECLARE_NATIVE(Native::orm_delvar)
{
	return 0;
}

// native orm_setkey(ORM:id, varname[]);
AMX_DECLARE_NATIVE(Native::orm_setkey)
{
	return 0;
}



// native mysql_log(E_LOGLEVEL:loglevel = ERROR | WARNING, E_LOGTYPE:logtype = TEXT);
AMX_DECLARE_NATIVE(Native::mysql_log)
{
	enum _LogLevel
	{
		NONE = 0,
		ERROR = 1,
		WARNING = 2,
		INFO = 4,
		DEBUG = 8,
	};

	cell loglevel = params[1];

	CLog::Get()->SetLogLevel(LOGLEVEL::ERROR, (loglevel & _LogLevel::ERROR));
	CLog::Get()->SetLogLevel(LOGLEVEL::WARNING, (loglevel & _LogLevel::WARNING));
	CLog::Get()->SetLogLevel(LOGLEVEL::INFO, (loglevel & _LogLevel::INFO));
	CLog::Get()->SetLogLevel(LOGLEVEL::DEBUG, (loglevel & _LogLevel::DEBUG));

	return 1;
}

// native MySQL:mysql_connect(const host[], const user[], const password[], const database[], MySQLOpt:option_id = MySQLOpt:0);
AMX_DECLARE_NATIVE(Native::mysql_connect)
{
	CScopedDebugInfo dbg_info(amx, "mysql_connect", "ss*sd");
	OptionsId_t options_id = static_cast<OptionsId_t>(params[5]);
	auto *options = COptionManager::Get()->GetOptionHandle(options_id);
	if (options == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid option id '{}'", options_id);
		return 0;
	}

	CError<CHandle> handle_error;
	Handle_t handle = CHandleManager::Get()->Create(
		amx_GetCppString(amx, params[1]),
		amx_GetCppString(amx, params[2]),
		amx_GetCppString(amx, params[3]),
		amx_GetCppString(amx, params[4]),
		options,
		handle_error);

	if (handle_error)
	{
		CLog::Get()->LogNative(handle_error);
		return 0;
	}

	assert(handle != nullptr);

	cell ret_val = handle->GetId();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native MySQL:mysql_connect_file(const file_name[] = "mysql.ini");
AMX_DECLARE_NATIVE(Native::mysql_connect_file)
{
	CScopedDebugInfo dbg_info(amx, "mysql_connect_file", "s");
	string file_name = amx_GetCppString(amx, params[1]);
	if (file_name.find_first_of("/\\") != string::npos) //no directory seperators allowed
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "file \"{}\" not in SA - MP root folder", file_name);
		return 0;
	}

	CError<CHandle> handle_error;
	Handle_t handle = CHandleManager::Get()->CreateFromFile(file_name, handle_error);

	if (handle_error)
	{
		CLog::Get()->LogNative(handle_error);
		return 0;
	}

	assert(handle != nullptr);

	cell ret_val = handle->GetId();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
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
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}

	cell ret_val = CHandleManager::Get()->Destroy(handle);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_unprocessed_queries(MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_unprocessed_queries)
{
	return 0;
}

// native mysql_global_options(E_MYSQL_GLOBAL_OPTION:type, value);
AMX_DECLARE_NATIVE(Native::mysql_global_options)
{
	CScopedDebugInfo dbg_info(amx, "mysql_global_options", "dd");
	switch (static_cast<COptionManager::GlobalOption>(params[1]))
	{
	case COptionManager::GlobalOption::DUPLICATE_CONNECTIONS:
		COptionManager::Get()->SetGlobalOption(
			COptionManager::GlobalOption::DUPLICATE_CONNECTIONS, params[2] != 0);
		break;
	default:
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "unknown option type '{}'", params[1]);
		return 0;
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
	return 1;
}

// native MySQLOpt:mysql_init_options();
AMX_DECLARE_NATIVE(Native::mysql_init_options)
{
	CScopedDebugInfo dbg_info(amx, "mysql_init_options");
	cell ret_val = COptionManager::Get()->Create();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
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
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid option id '{}'", options_id);
		return 0;
	}

	cell *param_addr = nullptr;
	amx_GetAddr(amx, params[3], &param_addr);
	if (param_addr == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no value specified");
		return 0;
	}

	cell value = *param_addr;
	bool ret_val = false;
	
	switch (static_cast<COptions::Type>(params[2]))
	{
	case COptions::Type::AUTO_RECONNECT:
		ret_val = options->SetOption(COptions::Type::AUTO_RECONNECT, static_cast<bool>(value != 0));
		break;
	case COptions::Type::MULTI_STATEMENTS:
		ret_val = options->SetOption(COptions::Type::MULTI_STATEMENTS, static_cast<bool>(value != 0));
		break;
	case COptions::Type::POOL_SIZE:
		if (value >= 0 && value <= 32)
			ret_val = options->SetOption(COptions::Type::POOL_SIZE, static_cast<unsigned int>(value));
		else
			CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid pool size '{}'", value);
		break;
	case COptions::Type::SERVER_PORT:
		if (value >= 0 && value <= std::numeric_limits<unsigned short>::max())
			ret_val = options->SetOption(COptions::Type::SERVER_PORT, static_cast<unsigned int>(value));
		else
			CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid MySQL server port '{}'", value);
		break;
	case COptions::Type::SSL_ENABLE:
		ret_val = options->SetOption(COptions::Type::SSL_ENABLE, static_cast<bool>(value != 0));
		break;
	case COptions::Type::SSL_KEY_FILE:
		ret_val = options->SetOption(COptions::Type::SSL_KEY_FILE, amx_GetCppString(amx, value));
		break;
	case COptions::Type::SSL_CERT_FILE:
		ret_val = options->SetOption(COptions::Type::SSL_CERT_FILE, amx_GetCppString(amx, value));
		break;
	case COptions::Type::SSL_CA_FILE:
		ret_val = options->SetOption(COptions::Type::SSL_CA_FILE, amx_GetCppString(amx, value));
		break;
	case COptions::Type::SSL_CA_PATH:
		ret_val = options->SetOption(COptions::Type::SSL_CA_PATH, amx_GetCppString(amx, value));
		break;
	case COptions::Type::SSL_CIPHER:
		ret_val = options->SetOption(COptions::Type::SSL_CIPHER, amx_GetCppString(amx, value));
		break;
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_pquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_pquery)
{
	return 0;
}

// native mysql_tquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_tquery)
{
	CScopedDebugInfo dbg_info(amx, "mysql_tquery", "dsss");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);

	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}

	CError<CCallback> callback_error;
	Callback_t callback = CCallback::Create(
		amx, 
		amx_GetCppString(amx, params[3]),
		amx_GetCppString(amx, params[4]),
		params, 5,
		callback_error);

	if (callback_error && callback_error.type() != CCallback::Error::EMPTY_NAME)
	{
		CLog::Get()->LogNative(callback_error);
		return 0;
	}
	

	Query_t query = CQuery::Create(amx_GetCppString(amx, params[2]));
	if (callback != nullptr)
	{
		query->OnExecutionFinished([=](ResultSet_t resultset)
		{
			CResultSetManager::Get()->SetActiveResultSet(resultset);

			callback->Execute();

			//unset active result(cache) + delete result (done by shared_ptr)
			CResultSetManager::Get()->SetActiveResultSet(nullptr);
		});
	}

	cell ret_val = handle->Execute(CHandle::ExecutionType::THREADED, query);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native Cache:mysql_query(MySQL:handle, const query[], bool:use_cache = true);
AMX_DECLARE_NATIVE(Native::mysql_query)
{
	return 0;
}

// native mysql_query_file(MySQL:handle, const file_path[]);
AMX_DECLARE_NATIVE(Native::mysql_query_file)
{
	CScopedDebugInfo dbg_info(amx, "mysql_query_file", "ds");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[1]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);

	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string filename = amx_GetCppString(amx, params[2]);
	std::ifstream file(filename);
	if (file.fail())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "can't open file '{}'", filename);
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
		 * a query could look like this: "SELECT stuff FROM table; -- selects # records"
		 * that's why we search for both comment specifiers and check for which comes first
		 * NOTE: we don't process C-style multiple-line comments, because the MySQL server
		 *       handles them in a special way
		 */
		size_t
			comment_pos = tmp_query_str.find("-- "),
			alt_comment_pos = tmp_query_str.find('#');

		if (alt_comment_pos != string::npos)
		{
			if (comment_pos == string::npos)
				comment_pos = alt_comment_pos;
			else
				comment_pos = (alt_comment_pos < comment_pos) ? alt_comment_pos : comment_pos;
		}

		if (comment_pos != string::npos)
			tmp_query_str.erase(comment_pos);

		
		query_str.append(tmp_query_str);

		if (query_str.back() == ';')
		{
			Query_t query = CQuery::Create(query_str);
			handle->Execute(CHandle::ExecutionType::UNTHREADED, query);
			query_str.clear();
		}
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
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
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return -1;
	}

	unsigned int errorid = 0;
	if (handle->GetErrorId(errorid) == false)
		return -1;

	cell ret_val = errorid;
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_format(MySQL:handle, output[], len, const format[], {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_format)
{
	return 0;
}

// native mysql_escape_string(const source[], destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_escape_string)
{
	CScopedDebugInfo dbg_info(amx, "mysql_escape_string", "srdd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[4]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string
		unescaped_str = amx_GetCppString(amx, params[1]),
		escaped_str;
	if (handle->EscapeString(unescaped_str, escaped_str) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "can't escape string '{}'", unescaped_str);
		return 0;
	}

	size_t max_str_len = params[3] - 1;
	if (escaped_str.length() > max_str_len)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"destination array too small (needs at least '{}' cells; has only '{}')",
			escaped_str.length() + 1, max_str_len + 1);
		return 0;
	}

	amx_SetCppString(amx, params[2], escaped_str, max_str_len + 1);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
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
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}
	
	cell ret_val = handle->SetCharacterSet(amx_GetCppString(amx, params[1]));
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native mysql_get_charset(destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_get_charset)
{
	CScopedDebugInfo dbg_info(amx, "mysql_get_charset", "rdd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[3]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string charset;
	if (handle->GetCharacterSet(charset) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "can't retreive character set");
		return 0;
	}

	size_t max_str_len = params[2] - 1;
	if (charset.length() > max_str_len)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"destination array too small (needs at least '{}' cells; has only '{}')", 
			charset.length() + 1, max_str_len + 1);
		return 0;
	}

	amx_SetCppString(amx, params[1], charset, max_str_len + 1);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
	return 1;
}

// native mysql_stat(destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_stat)
{
	CScopedDebugInfo dbg_info(amx, "mysql_stat", "rdd");
	const HandleId_t handle_id = static_cast<HandleId_t>(params[3]);
	Handle_t handle = CHandleManager::Get()->GetHandle(handle_id);
	if (handle == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid connection handle '{}'", handle_id);
		return 0;
	}

	string status;
	if (handle->GetStatus(status) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "can't retreive status");
		return 0;
	}

	size_t max_str_len = params[2] - 1;
	if (status.length() > max_str_len)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"destination array too small (needs at least '{}' cells; has only '{}')",
			status.length() + 1, max_str_len + 1);
		return 0;
	}

	amx_SetCppString(amx, params[1], status, max_str_len + 1);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
	return 1;
}



// native cache_get_row_count();
AMX_DECLARE_NATIVE(Native::cache_get_row_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_row_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = static_cast<cell>(resultset->GetActiveResult()->GetRowCount());
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_field_count();
AMX_DECLARE_NATIVE(Native::cache_get_field_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = resultset->GetActiveResult()->GetFieldCount();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_result_count();
AMX_DECLARE_NATIVE(Native::cache_get_result_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_result_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = resultset->GetResultCount();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_field_name(field_index, destination[], max_len = sizeof(destination))
AMX_DECLARE_NATIVE(Native::cache_get_field_name)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_name", "drd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string field_name;
	if (resultset->GetActiveResult()->GetFieldName(params[1], field_name) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "invalid index '{}'", params[1]);
		return 0;
	}

	amx_SetCppString(amx, params[2], field_name, params[3]);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
	return 1;
}

// native cache_set_result(result_index);
AMX_DECLARE_NATIVE(Native::cache_set_result)
{
	CScopedDebugInfo dbg_info(amx, "cache_set_result", "d");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	cell ret_val = resultset->SetActiveResult(params[1]);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}


// native cache_get_row(row_idx, field_idx, destination[], max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_row)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_row", "ddrd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string data;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], data) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, 
			"invalid row ('{}') or field ('{}') index", params[1], params[2]);
		return 0;
	}

	amx_SetCppString(amx, params[3], data, params[4]);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_row_int(row_idx, field_idx);
AMX_DECLARE_NATIVE(Native::cache_get_row_int)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_row_int", "dd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string data;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], data) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"invalid row ('{}') or field ('{}') index", params[1], params[2]);
		return 0;
	}

	cell data_int = 0;
	if (ConvertStrToData<cell>(data, data_int) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "value '{}' is not a number", data);
		return 0;
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", data_int);
	return data_int;
}

// native Float:cache_get_row_float(row_idx, field_idx);
AMX_DECLARE_NATIVE(Native::cache_get_row_float)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_row_float", "dd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string data;
	if (resultset->GetActiveResult()->GetRowData(params[1], params[2], data) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"invalid row ('{}') or field ('{}') index", params[1], params[2]);
		return 0;
	}

	float data_float = 0.0f;
	if (ConvertStrToData<float>(data, data_float) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "value '{}' is not a number", data);
		return 0;
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", data_float);
	return amx_ftoc(data_float);
}

// native cache_get_field_content(row_idx, const field_name[], destination[], max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_field_content)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_content", "dsrd");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, 
			"invalid row index '{}' (number of rows: '{}')", row_idx, result->GetRowCount());
		return 0;
	}

	string data;
	if (result->GetRowDataByName(row_idx, field_name, data) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	amx_SetCppString(amx, params[3], data, params[4]);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '1'");
	return 1;
}

// native cache_get_field_content_int(row_idx, const field_name[]);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_int)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_content_int", "ds");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"invalid row index '{}' (number of rows: '{}')", row_idx, result->GetRowCount());
		return 0;
	}

	string data;
	if (result->GetRowDataByName(row_idx, field_name, data) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	cell data_int = 0;
	if (ConvertStrToData<cell>(data, data_int) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "value '{}' is not a number", data);
		return 0;
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", data_int);
	return data_int;
}

// native Float:cache_get_field_content_float(row_idx, const field_name[]);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_float)
{
	CScopedDebugInfo dbg_info(amx, "cache_get_field_content_float", "ds");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return 0;
	}

	string field_name = amx_GetCppString(amx, params[2]);
	if (field_name.empty())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "empty field name");
		return 0;
	}

	auto result = resultset->GetActiveResult();
	const cell &row_idx = params[1];
	if (row_idx >= result->GetRowCount())
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR,
			"invalid row index '{}' (number of rows: '{}')", row_idx, result->GetRowCount());
		return 0;
	}

	string data;
	if (result->GetRowDataByName(row_idx, field_name, data) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "field '{}' not found", field_name);
		return 0;
	}

	float data_float = 0.0f;
	if (ConvertStrToData<float>(data, data_float) == false)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "value '{}' is not a number", data);
		return 0;
	}

	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", data_float);
	return amx_ftoc(data_float);
}

// native Cache:cache_save();
AMX_DECLARE_NATIVE(Native::cache_save)
{
	CScopedDebugInfo dbg_info(amx, "cache_save", "");
	cell ret_val = CResultSetManager::Get()->StoreActiveResultSet();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_delete(Cache:cache_id);
AMX_DECLARE_NATIVE(Native::cache_delete)
{
	CScopedDebugInfo dbg_info(amx, "cache_delete", "d");
	cell ret_val = CResultSetManager::Get()->DeleteResultSet(params[1]);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_set_active(Cache:cache_id);
AMX_DECLARE_NATIVE(Native::cache_set_active)
{
	CScopedDebugInfo dbg_info(amx, "cache_set_active", "d");
	auto resultset = CResultSetManager::Get()->GetResultSet(params[1]);
	if (resultset)
		CResultSetManager::Get()->SetActiveResultSet(resultset);

	cell ret_val = resultset != nullptr;
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_is_valid(Cache:cache_id);
AMX_DECLARE_NATIVE(Native::cache_is_valid)
{
	CScopedDebugInfo dbg_info(amx, "cache_is_valid", "d");
	cell ret_val = CResultSetManager::Get()->IsValidResultSet(params[1]);
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_affected_rows();
AMX_DECLARE_NATIVE(Native::cache_affected_rows)
{
	CScopedDebugInfo dbg_info(amx, "cache_affected_rows", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = static_cast<cell>(resultset->AffectedRows());
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_warning_count();
AMX_DECLARE_NATIVE(Native::cache_warning_count)
{
	CScopedDebugInfo dbg_info(amx, "cache_warning_count", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = resultset->WarningCount();
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_insert_id();
AMX_DECLARE_NATIVE(Native::cache_insert_id)
{
	CScopedDebugInfo dbg_info(amx, "cache_insert_id", "");
	auto resultset = CResultSetManager::Get()->GetActiveResultSet();
	if (resultset == nullptr)
	{
		CLog::Get()->LogNative(LOGLEVEL::ERROR, "no active cache");
		return -1;
	}

	cell ret_val = static_cast<cell>(resultset->InsertId());
	CLog::Get()->LogNative(LOGLEVEL::DEBUG, "return value: '{}'", ret_val);
	return ret_val;
}

// native cache_get_query_exec_time(E_EXECTIME_UNIT:unit = MICROSECONDS);
AMX_DECLARE_NATIVE(Native::cache_get_query_exec_time)
{
	return 0;
}

// native cache_get_query_string(destination[], max_len = sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_query_string)
{
	return 0;
}
