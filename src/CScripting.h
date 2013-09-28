#pragma once
#ifndef INC_CSCRIPTING_H
#define INC_CSCRIPTING_H


#include "main.h"


namespace Native {
	//utility natives
	cell AMX_NATIVE_CALL serialize_array(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL unserialize_array(AMX* amx, cell* params);


	//ORM natives
	cell AMX_NATIVE_CALL orm_create(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL orm_destroy(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL orm_errno(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL orm_select(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL orm_update(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL orm_insert(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL orm_delete(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL orm_save(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL orm_apply_cache(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL orm_addvar(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL orm_setkey(AMX* amx, cell* params);


	//MySQL natives
	cell AMX_NATIVE_CALL mysql_log(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_connect(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_close(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_reconnect(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_unprocessed_queries(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL mysql_errno(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_escape_string(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_format(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_tquery(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_query(AMX* amx, cell* params);
	
	cell AMX_NATIVE_CALL mysql_stat(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_set_charset(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_get_charset(AMX* amx, cell* params);
	

	//Cache natives
	cell AMX_NATIVE_CALL cache_get_data(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_row_count(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_count(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_name(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL cache_get_row(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_row_int(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_row_float(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL cache_get_field_content(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_content_int(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_content_float(AMX* amx, cell* params);

	cell AMX_NATIVE_CALL cache_save(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_delete(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_set_active(AMX* amx, cell* params);
	
	cell AMX_NATIVE_CALL cache_affected_rows(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_insert_id(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_warning_count(AMX* amx, cell* params);
};


#endif // INC_CSCRIPTING_H
