#include "CScripting.h"
#include "CHandle.h"
#include "CQuery.h"
#include "CCallback.h"
#include "CResult.h"


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
// native MySQL:mysql_connect(const host[], const user[], const database[], const password[], port = 3306, bool:autoreconnect = true, pool_size = 2);
AMX_DECLARE_NATIVE(Native::mysql_connect)
{
	return 0;
}

// native mysql_close(MySQL:handle = DEFAULT_MYSQL_HANDLE);
AMX_DECLARE_NATIVE(Native::mysql_close)
{
	return 0;
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

	CCallback::Error callback_error = CCallback::Error::NONE;
	CCallback::Type_t callback = CCallback::Create(
		amx, 
		amx_GetCppString(amx, params[3]),
		amx_GetCppString(amx, params[4]),
		params, 5,
		callback_error);

	if (callback_error != CCallback::Error::NONE &&
		callback_error != CCallback::Error::EMPTY_NAME)
		return 0; // TODO: log error with CCallbackManager::GetErrorString
	

	CQuery *query = CQuery::Create(amx_GetCppString(amx, params[2]));
	if (callback != nullptr)
	{
		query->OnExecutionFinished([=](const CResult *result)
		{
			// TODO: pre-execute: set active handle & result(cache)
			callback->Execute();
			// TODO: post-execute: unset active handle & result(cache) + delete result
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
	return 0;
}

// native cache_get_row_count();
AMX_DECLARE_NATIVE(Native::cache_get_row_count)
{
	return 0;
}

// native cache_get_field_count();
AMX_DECLARE_NATIVE(Native::cache_get_field_count)
{
	return 0;
}

// native cache_get_field_name(field_index, destination[], max_len = sizeof(destination))
AMX_DECLARE_NATIVE(Native::cache_get_field_name)
{
	return 0;
}

// native cache_get_row(row_idx, field_idx, destination[], max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_row)
{
	return 0;
}

// native cache_get_row_int(row_idx, field_idx);
AMX_DECLARE_NATIVE(Native::cache_get_row_int)
{
	return 0;
}

// native Float:cache_get_row_float(row_idx, field_idx);
AMX_DECLARE_NATIVE(Native::cache_get_row_float)
{
	return 0;
}

// native cache_get_field_content(row_idx, const field_name[], destination[], max_len=sizeof(destination));
AMX_DECLARE_NATIVE(Native::cache_get_field_content)
{
	return 0;
}

// native cache_get_field_content_int(row_idx, const field_name[]);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_int)
{
	return 0;
}

// native Float:cache_get_field_content_float(row_idx, const field_name[]);
AMX_DECLARE_NATIVE(Native::cache_get_field_content_float)
{
	return 0;
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
	return 0;
}

// native cache_warning_count();
AMX_DECLARE_NATIVE(Native::cache_warning_count)
{
	return 0;
}

// native cache_insert_id();
AMX_DECLARE_NATIVE(Native::cache_insert_id)
{
	return 0;
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
