#include "CScripting.h"
#include "CQuery.h"
#include "CHandle.h"
#include "CCallback.h"
#include "CResult.h"
#include "misc.h"


// native ORM:orm_create(const table[], MySQL:handle = DEFAULT_MYSQL_HANDLE);
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
	return 0;
}

// TODO: remove auto-reconnect option (moved to mysql_option)
// native MySQL:mysql_connect(const host[], const user[], const password[], const database[], port = 3306, bool:autoreconnect = true, pool_size = 2);
AMX_DECLARE_NATIVE(Native::mysql_connect)
{
	CHandle::Error handle_error;
	CHandle *handle = CHandleManager::Get()->Create(
		amx_GetCppString(amx, params[1]),
		amx_GetCppString(amx, params[2]),
		amx_GetCppString(amx, params[3]),
		amx_GetCppString(amx, params[4]),
		params[5],
		params[7],
		handle_error);

	if (handle_error != CHandle::Error::NONE)
		return 0; //TODO: error message

	assert(handle != nullptr);

	return handle->GetId();
}

// native mysql_close(MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_close)
{
	CHandle *handle = CHandleManager::Get()->GetHandle(static_cast<CHandle::Id_t>(params[1]));
	if (handle == nullptr)
		return 0;

	return CHandleManager::Get()->Destroy(handle);
}

// native mysql_reconnect(MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_reconnect)
{
	return 0;
}

// native mysql_unprocessed_queries(MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_unprocessed_queries)
{
	return 0;
}

// native mysql_current_handle();
AMX_DECLARE_NATIVE(Native::mysql_current_handle)
{
	return 0;
}

// TODO: add auto reconnect option to mysql_option
// native mysql_option(E_MYSQL_OPTION:type, value);
AMX_DECLARE_NATIVE(Native::mysql_option)
{
	return 0;
}

// native mysql_pquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_pquery)
{
	return 0;
}

// native mysql_tquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_tquery)
{
	const CHandle::Id_t handle_id = static_cast<CHandle::Id_t>(params[1]);
	CHandle *handle = CHandleManager::Get()->GetHandle(handle_id);

	if (handle == nullptr)
		return 0;

	CCallback::Error callback_error;
	CCallback::Type_t callback = CCallback::Create(
		amx, 
		amx_GetCppString(amx, params[3]),
		amx_GetCppString(amx, params[4]),
		params, 5,
		callback_error);

	if (callback_error != CCallback::Error::NONE &&
		callback_error != CCallback::Error::EMPTY_NAME)
		return 0; // TODO: log error with CCallbackManager::GetErrorString
	

	CQuery::Type_t query = CQuery::Create(amx_GetCppString(amx, params[2]));
	if (callback != nullptr)
	{
		query->OnExecutionFinished([=](CResultSet *result)
		{
			// TODO: pre-execute: set active handle & result(cache)
			CResultSetManager::Get()->SetActiveResultSet(result);

			//execute PAWN callback
			callback->Execute();

			// TODO: post-execute: unset active handle & result(cache) + delete result
			CResultSetManager::Get()->SetActiveResultSet(nullptr);
			delete result;
		});
	}

	return handle->Execute(CHandle::ExecutionType::THREADED, query);
}

// native Cache:mysql_query(MySQL:handle, const query[], bool:use_cache = true);
AMX_DECLARE_NATIVE(Native::mysql_query)
{
	return 0;
}

// native mysql_errno(MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_errno)
{
	return 0;
}

// native mysql_format(MySQL:handle, output[], len, const format[], {Float,_}:...);
AMX_DECLARE_NATIVE(Native::mysql_format)
{
	return 0;
}

// native mysql_escape_string(const source[], destination[], max_len = sizeof(destination), MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_escape_string)
{
	return 0;
}

// native mysql_set_charset(const charset[], MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_set_charset)
{
	return 0;
}

// native mysql_get_charset(destination[], max_len = sizeof(destination), MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_get_charset)
{
	return 0;
}

// native mysql_stat(destination[], max_len = sizeof(destination), MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_stat)
{
	return 0;
}



// native cache_get_data(&num_rows, &num_fields);
AMX_DECLARE_NATIVE(Native::cache_get_data)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	cell
		*num_rows_dest = nullptr,
		*num_fields_dest = nullptr;
	amx_GetAddr(amx, params[1], &num_rows_dest);
	amx_GetAddr(amx, params[2], &num_fields_dest);
	if (num_rows_dest == nullptr || num_fields_dest == nullptr)
		return 0;

	(*num_rows_dest) = static_cast<cell>(result->GetActiveResult()->GetRowCount());
	(*num_fields_dest) = result->GetActiveResult()->GetFieldCount();
	return 1;
}

// native cache_get_row_count();
AMX_DECLARE_NATIVE(Native::cache_get_row_count)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	return result != nullptr ? static_cast<cell>(result->GetActiveResult()->GetRowCount()) : -1;
}

// native cache_get_field_count();
AMX_DECLARE_NATIVE(Native::cache_get_field_count)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	return result != nullptr ? result->GetActiveResult()->GetFieldCount() : -1;
}

// native cache_get_field_name(field_index, destination[], max_len = sizeof(destination))
AMX_DECLARE_NATIVE(Native::cache_get_field_name)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string field_name;
	if (result->GetActiveResult()->GetFieldName(params[1], field_name) == false)
		return 0;

	amx_SetCppString(amx, params[2], field_name, params[3]);
	return 1;
}

// native cache_get_row(row_idx, field_idx, destination[], max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_row)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string data;
	if (result->GetActiveResult()->GetRowData(params[1], params[2], data) == false)
		return 0;

	amx_SetCppString(amx, params[3], data, params[4]);
	return 1;
}

// native cache_get_row_int(row_idx, field_idx);
AMX_DECLARE_NATIVE(Native::cache_get_row_int)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string data;
	if (result->GetActiveResult()->GetRowData(params[1], params[2], data) == false)
		return 0;

	cell data_int = 0;
	if (ConvertStrToData<cell>(data, data_int) == false)
		return 0;

	return data_int;
}

// native Float:cache_get_row_float(row_idx, field_idx);
AMX_DECLARE_NATIVE(Native::cache_get_row_float)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string data;
	if (result->GetActiveResult()->GetRowData(params[1], params[2], data) == false)
		return 0;

	float data_float = 0.0f;
	if (ConvertStrToData<float>(data, data_float) == false)
		return 0;

	return amx_ftoc(data_float);
}

// native cache_get_field_content(row_idx, const field_name[], destination[], max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_field_content)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string data;
	if (result->GetActiveResult()->GetRowDataByName(params[1], amx_GetCppString(amx, params[2]), data) == false)
		return 0;

	amx_SetCppString(amx, params[3], data, params[4]);
	return 1;
}

// native cache_get_field_content_int(row_idx, const field_name[]);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_int)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string data;
	if (result->GetActiveResult()->GetRowDataByName(params[1], amx_GetCppString(amx, params[2]), data) == false)
		return 0;

	cell data_int = 0;
	if (ConvertStrToData<cell>(data, data_int) == false)
		return 0;

	return data_int;
}

// native Float:cache_get_field_content_float(row_idx, const field_name[]);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_float)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	if (result == nullptr)
		return 0;

	string data;
	if (result->GetActiveResult()->GetRowDataByName(params[1], amx_GetCppString(amx, params[2]), data) == false)
		return 0;

	float data_float = 0.0f;
	if (ConvertStrToData<float>(data, data_float) == false)
		return 0;

	return amx_ftoc(data_float);
}

// native Cache:cache_save();
AMX_DECLARE_NATIVE(Native::cache_save)
{
	return 0;
}

// native cache_delete(Cache:cache_id, MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::cache_delete)
{
	return 0;
}

// native cache_set_active(Cache:cache_id, MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::cache_set_active)
{
	return 0;
}

// native cache_is_valid(Cache:cache_id, MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::cache_is_valid)
{
	return 0;
}

// native cache_affected_rows();
AMX_DECLARE_NATIVE(Native::cache_affected_rows)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	return result != nullptr ? static_cast<cell>(result->AffectedRows()) : -1;
}

// native cache_warning_count();
AMX_DECLARE_NATIVE(Native::cache_warning_count)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	return result != nullptr ? result->WarningCount() : -1;
}

// native cache_insert_id();
AMX_DECLARE_NATIVE(Native::cache_insert_id)
{
	auto *result = CResultSetManager::Get()->GetActiveResultSet();
	return result != nullptr ? static_cast<cell>(result->InsertId()) : -1;
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
