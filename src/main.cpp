#include "sdk.h"
#include "CScripting.h"
#include "CCallback.h"
#include "CDispatcher.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


extern void	*pAMXFunctions;
logprintf_t logprintf;


PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK; 
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) 
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	
	if (mysql_library_init(0, NULL, NULL)) 
	{
		logprintf(" >> plugin.mysql: plugin failed to load due to uninitialized MySQL library ('libmysql.dll' probably missing).");
		return false;
	}
	
	
	logprintf(" >> plugin.mysql: R40 successfully loaded.");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() 
{
	logprintf("plugin.mysql: Unloading plugin...");

	mysql_library_end();

	logprintf("plugin.mysql: Plugin unloaded."); 
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() 
{
	CDispatcher::Get()->Process();
}


extern "C" const AMX_NATIVE_INFO native_list[] = 
{
	AMX_DEFINE_NATIVE(orm_create)
	AMX_DEFINE_NATIVE(orm_destroy)

	AMX_DEFINE_NATIVE(orm_errno)
	
	AMX_DEFINE_NATIVE(orm_select)
	AMX_DEFINE_NATIVE(orm_update)
	AMX_DEFINE_NATIVE(orm_insert)
	AMX_DEFINE_NATIVE(orm_delete)
	
	AMX_DEFINE_NATIVE(orm_save)
	
	AMX_DEFINE_NATIVE(orm_apply_cache)

	AMX_DEFINE_NATIVE(orm_addvar_int)
	AMX_DEFINE_NATIVE(orm_addvar_float)
	AMX_DEFINE_NATIVE(orm_addvar_string)
	AMX_DEFINE_NATIVE(orm_delvar)
	AMX_DEFINE_NATIVE(orm_setkey)


	AMX_DEFINE_NATIVE(mysql_log)
	AMX_DEFINE_NATIVE(mysql_connect)
	AMX_DEFINE_NATIVE(mysql_close)
	AMX_DEFINE_NATIVE(mysql_reconnect)
	
	AMX_DEFINE_NATIVE(mysql_unprocessed_queries)
	AMX_DEFINE_NATIVE(mysql_current_handle)
	AMX_DEFINE_NATIVE(mysql_option)
	
	AMX_DEFINE_NATIVE(mysql_errno)
	AMX_DEFINE_NATIVE(mysql_escape_string)
	AMX_DEFINE_NATIVE(mysql_format)
	AMX_DEFINE_NATIVE(mysql_pquery)
	AMX_DEFINE_NATIVE(mysql_tquery)
	AMX_DEFINE_NATIVE(mysql_query)
	 
	AMX_DEFINE_NATIVE(mysql_stat)
	AMX_DEFINE_NATIVE(mysql_get_charset)
	AMX_DEFINE_NATIVE(mysql_set_charset)


	AMX_DEFINE_NATIVE(cache_get_data)
	AMX_DEFINE_NATIVE(cache_get_row_count)
	AMX_DEFINE_NATIVE(cache_get_field_count)
	AMX_DEFINE_NATIVE(cache_get_field_name)

	AMX_DEFINE_NATIVE(cache_get_row)
	AMX_DEFINE_NATIVE(cache_get_row_int)
	AMX_DEFINE_NATIVE(cache_get_row_float)

	AMX_DEFINE_NATIVE(cache_get_field_content)
	AMX_DEFINE_NATIVE(cache_get_field_content_int)
	AMX_DEFINE_NATIVE(cache_get_field_content_float)

	AMX_DEFINE_NATIVE(cache_save)
	AMX_DEFINE_NATIVE(cache_delete)
	AMX_DEFINE_NATIVE(cache_set_active)
	AMX_DEFINE_NATIVE(cache_is_valid)

	AMX_DEFINE_NATIVE(cache_affected_rows)
	AMX_DEFINE_NATIVE(cache_insert_id)
	AMX_DEFINE_NATIVE(cache_warning_count)

	AMX_DEFINE_NATIVE(cache_get_query_exec_time)
	AMX_DEFINE_NATIVE(cache_get_query_string)
	{NULL, NULL}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) 
{
	CCallbackManager::Get()->AddAmx(amx);
	return amx_Register(amx, native_list, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	CCallbackManager::Get()->RemoveAmx(amx);
	return AMX_ERR_NONE;
}
