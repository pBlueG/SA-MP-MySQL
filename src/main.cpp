#include "sdk.hpp"
#include "natives.hpp"
#include "CHandle.hpp"
#include "CCallback.hpp"
#include "CResult.hpp"
#include "CDispatcher.hpp"
#include "COptions.hpp"
#include "COrm.hpp"
#include "CLog.hpp"
#include "version.hpp"

#include "mysql.hpp"



extern void	*pAMXFunctions;
logprintf_t logprintf;


PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];

	if (mysql_library_init(0, NULL, NULL))
	{
		logprintf(" >> plugin.mysql: can't initialize MySQL library.");
		return false;
	}

	logprintf(" >> plugin.mysql: " MYSQL_VERSION " successfully loaded.");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	logprintf("plugin.mysql: Unloading plugin...");

	COrmManager::CSingleton::Destroy();
	CHandleManager::CSingleton::Destroy();
	CCallbackManager::CSingleton::Destroy();
	CResultSetManager::CSingleton::Destroy();
	CDispatcher::CSingleton::Destroy();
	COptionManager::CSingleton::Destroy();
	CLog::CSingleton::Destroy();
	samplog::Api::Destroy();

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

	AMX_DEFINE_NATIVE(orm_apply_cache)

	AMX_DEFINE_NATIVE(orm_select)
	AMX_DEFINE_NATIVE(orm_update)
	AMX_DEFINE_NATIVE(orm_insert)
	AMX_DEFINE_NATIVE(orm_delete)

	AMX_DEFINE_NATIVE(orm_save)

	AMX_DEFINE_NATIVE(orm_addvar_int)
	AMX_DEFINE_NATIVE(orm_addvar_float)
	AMX_DEFINE_NATIVE(orm_addvar_string)

	AMX_DEFINE_NATIVE(orm_clear_vars)
	AMX_DEFINE_NATIVE(orm_delvar)
	AMX_DEFINE_NATIVE(orm_setkey)


	AMX_DEFINE_NATIVE(mysql_connect)
	AMX_DEFINE_NATIVE(mysql_connect_file)
	AMX_DEFINE_NATIVE(mysql_close)

	AMX_DEFINE_NATIVE(mysql_unprocessed_queries)
	AMX_DEFINE_NATIVE(mysql_global_options)

	AMX_DEFINE_NATIVE(mysql_init_options)
	AMX_DEFINE_NATIVE(mysql_set_option)

	AMX_DEFINE_NATIVE(mysql_pquery)
	AMX_DEFINE_NATIVE(mysql_tquery)
	AMX_DEFINE_NATIVE(mysql_query)
	AMX_DEFINE_NATIVE(mysql_tquery_file)
	AMX_DEFINE_NATIVE(mysql_query_file)

	AMX_DEFINE_NATIVE(mysql_errno)
	AMX_DEFINE_NATIVE(mysql_error)
	AMX_DEFINE_NATIVE(mysql_escape_string)
	AMX_DEFINE_NATIVE(mysql_format)
	AMX_DEFINE_NATIVE(mysql_get_charset)
	AMX_DEFINE_NATIVE(mysql_set_charset)
	AMX_DEFINE_NATIVE(mysql_stat)


	AMX_DEFINE_NATIVE(cache_get_row_count)
	AMX_DEFINE_NATIVE(cache_get_field_count)
	AMX_DEFINE_NATIVE(cache_get_result_count)
	AMX_DEFINE_NATIVE(cache_get_field_name)
	AMX_DEFINE_NATIVE(cache_get_field_type)
	AMX_DEFINE_NATIVE(cache_set_result)

	AMX_DEFINE_NATIVE(cache_get_value_index)
	AMX_DEFINE_NATIVE(cache_get_value_index_int)
	AMX_DEFINE_NATIVE(cache_get_value_index_float)
	AMX_DEFINE_NATIVE(cache_is_value_index_null)

	AMX_DEFINE_NATIVE(cache_get_value_name)
	AMX_DEFINE_NATIVE(cache_get_value_name_int)
	AMX_DEFINE_NATIVE(cache_get_value_name_bigint)
	AMX_DEFINE_NATIVE(cache_get_value_name_float)
	AMX_DEFINE_NATIVE(cache_is_value_name_null)

	AMX_DEFINE_NATIVE(cache_save)
	AMX_DEFINE_NATIVE(cache_delete)
	AMX_DEFINE_NATIVE(cache_set_active)
	AMX_DEFINE_NATIVE(cache_unset_active)
	AMX_DEFINE_NATIVE(cache_is_any_active)
	AMX_DEFINE_NATIVE(cache_is_valid)

	AMX_DEFINE_NATIVE(cache_affected_rows)
	AMX_DEFINE_NATIVE(cache_insert_id)
	AMX_DEFINE_NATIVE(cache_warning_count)

	AMX_DEFINE_NATIVE(cache_get_query_exec_time)
	AMX_DEFINE_NATIVE(cache_get_query_string)
	{ NULL, NULL }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	samplog::Api::Get()->RegisterAmx(amx);
	CCallbackManager::Get()->AddAmx(amx);
	return amx_Register(amx, native_list, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	samplog::Api::Get()->EraseAmx(amx);
	CCallbackManager::Get()->RemoveAmx(amx);
	return AMX_ERR_NONE;
}
