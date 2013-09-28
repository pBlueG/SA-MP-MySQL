#pragma once

#include "main.h"
#include "CScripting.h"
#include "CMySQLHandle.h"
#include "CCallback.h"
#include "CLog.h"

//#include <vld.h>

namespace boost {
	void tss_cleanup_implemented(void) {}
};


extern void	*pAMXFunctions;
extern logprintf_t logprintf;
 

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK; 
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	
	if (mysql_library_init(0, NULL, NULL)) {
		logprintf(" >> plugin.mysql: Plugin failed to load due to uninitialized MySQL library (libmysql probably missing).");
		exit(0);
		return false;
	}
	
	CLog::Get()->Initialize("mysql_log.txt"); 

	logprintf(" >> plugin.mysql: R34 successfully loaded.");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	logprintf("plugin.mysql: Unloading plugin...");

	CCallback::ClearAll();
	CMySQLHandle::ClearAll();
	mysql_library_end();
	CLog::Delete(); //this has to be the last!

	logprintf("plugin.mysql: Plugin unloaded."); 
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	CCallback::ProcessCallbacks();
}


#if defined __cplusplus
extern "C"
#endif
const AMX_NATIVE_INFO MySQLNatives[] = {
	{"serialize_array",					Native::serialize_array},
	{"unserialize_array",				Native::unserialize_array},


	{"orm_create",						Native::orm_create},
	{"orm_destroy",						Native::orm_destroy},

	{"orm_errno",						Native::orm_errno},

	{"orm_select",						Native::orm_select},
	{"orm_update",						Native::orm_update},
	{"orm_insert",						Native::orm_insert},
	{"orm_delete",						Native::orm_delete},

	{"orm_save",						Native::orm_save},

	{"orm_apply_cache",					Native::orm_apply_cache},

	{"orm_addvar",						Native::orm_addvar},
	{"orm_setkey",						Native::orm_setkey},


	{"mysql_log",						Native::mysql_log}, 
	{"mysql_connect",					Native::mysql_connect},
	{"mysql_close",						Native::mysql_close},
	{"mysql_reconnect",					Native::mysql_reconnect},
	{"mysql_unprocessed_queries",		Native::mysql_unprocessed_queries},
	
	{"mysql_errno",						Native::mysql_errno},
	{"mysql_escape_string",				Native::mysql_escape_string},
	{"mysql_format",					Native::mysql_format},
	{"mysql_tquery",					Native::mysql_tquery},
	{"mysql_query",						Native::mysql_query},

	{"mysql_stat",						Native::mysql_stat},
	{"mysql_get_charset",				Native::mysql_get_charset},
	{"mysql_set_charset",				Native::mysql_set_charset},


	{"cache_get_data",					Native::cache_get_data},
	{"cache_get_row_count",				Native::cache_get_row_count},
	{"cache_get_field_count",			Native::cache_get_field_count},
	{"cache_get_field_name",			Native::cache_get_field_name},

	{"cache_get_row",					Native::cache_get_row},
	{"cache_get_row_int",				Native::cache_get_row_int},
	{"cache_get_row_float",				Native::cache_get_row_float},

	{"cache_get_field_content",			Native::cache_get_field_content},
	{"cache_get_field_content_int",		Native::cache_get_field_content_int},
	{"cache_get_field_content_float",	Native::cache_get_field_content_float},

	{"cache_save",						Native::cache_save},
	{"cache_delete",					Native::cache_delete},
	{"cache_set_active",				Native::cache_set_active},

	{"cache_affected_rows",				Native::cache_affected_rows},
	{"cache_insert_id",					Native::cache_insert_id},
	{"cache_warning_count",				Native::cache_warning_count},
	{NULL, NULL}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	CCallback::AddAmx(amx);
	return amx_Register(amx, MySQLNatives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	CCallback::EraseAmx(amx);
	return AMX_ERR_NONE;
}

