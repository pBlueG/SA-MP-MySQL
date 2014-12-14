#pragma once

#include "sdk.h"

#define AMX_DECLARE_NATIVE(native) \
	cell AMX_NATIVE_CALL native(AMX *amx, cell *params)

#define AMX_DEFINE_NATIVE(native) \
	{#native, Native::native},


namespace Native 
{
	//ORM natives
	AMX_DECLARE_NATIVE(orm_create);
	AMX_DECLARE_NATIVE(orm_destroy);

	AMX_DECLARE_NATIVE(orm_errno);
	
	AMX_DECLARE_NATIVE(orm_select);
	AMX_DECLARE_NATIVE(orm_update);
	AMX_DECLARE_NATIVE(orm_insert);
	AMX_DECLARE_NATIVE(orm_delete);

	AMX_DECLARE_NATIVE(orm_save);
	
	AMX_DECLARE_NATIVE(orm_apply_cache);

	AMX_DECLARE_NATIVE(orm_addvar_int);
	AMX_DECLARE_NATIVE(orm_addvar_float);
	AMX_DECLARE_NATIVE(orm_addvar_string);
	AMX_DECLARE_NATIVE(orm_delvar);
	AMX_DECLARE_NATIVE(orm_setkey);


	//MySQL natives
	AMX_DECLARE_NATIVE(mysql_log);
	AMX_DECLARE_NATIVE(mysql_connect);
	AMX_DECLARE_NATIVE(mysql_close);
	AMX_DECLARE_NATIVE(mysql_reconnect);

	AMX_DECLARE_NATIVE(mysql_unprocessed_queries);
	AMX_DECLARE_NATIVE(mysql_current_handle);
	AMX_DECLARE_NATIVE(mysql_global_options);

	AMX_DECLARE_NATIVE(mysql_init_options);
	AMX_DECLARE_NATIVE(mysql_set_option);

	AMX_DECLARE_NATIVE(mysql_errno);
	AMX_DECLARE_NATIVE(mysql_escape_string);
	AMX_DECLARE_NATIVE(mysql_format);
	AMX_DECLARE_NATIVE(mysql_pquery);
	AMX_DECLARE_NATIVE(mysql_tquery);
	AMX_DECLARE_NATIVE(mysql_query);
	
	AMX_DECLARE_NATIVE(mysql_stat);
	AMX_DECLARE_NATIVE(mysql_set_charset);
	AMX_DECLARE_NATIVE(mysql_get_charset);
	

	//Cache natives
	AMX_DECLARE_NATIVE(cache_get_row_count);
	AMX_DECLARE_NATIVE(cache_get_field_count);
	AMX_DECLARE_NATIVE(cache_get_result_count);
	AMX_DECLARE_NATIVE(cache_get_field_name);
	AMX_DECLARE_NATIVE(cache_set_result);

	AMX_DECLARE_NATIVE(cache_get_row);
	AMX_DECLARE_NATIVE(cache_get_row_int);
	AMX_DECLARE_NATIVE(cache_get_row_float);

	AMX_DECLARE_NATIVE(cache_get_field_content);
	AMX_DECLARE_NATIVE(cache_get_field_content_int);
	AMX_DECLARE_NATIVE(cache_get_field_content_float);

	AMX_DECLARE_NATIVE(cache_save);
	AMX_DECLARE_NATIVE(cache_delete);
	AMX_DECLARE_NATIVE(cache_set_active);
	AMX_DECLARE_NATIVE(cache_is_valid);
	
	AMX_DECLARE_NATIVE(cache_affected_rows);
	AMX_DECLARE_NATIVE(cache_insert_id);
	AMX_DECLARE_NATIVE(cache_warning_count);

	AMX_DECLARE_NATIVE(cache_get_query_exec_time);
	AMX_DECLARE_NATIVE(cache_get_query_string);
};
