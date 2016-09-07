/**
 * MySQL plugin @MYSQL_PLUGIN_VERSION@
 */


#if defined mysql_included
	#endinput
#endif
#define mysql_included


/**
 * Common error codes
 *
 * Client: http://dev.mysql.com/doc/refman/5.5/en/error-messages-client.html
 * Server: http://dev.mysql.com/doc/refman/5.5/en/error-messages-server.html
 */

#define ER_DBACCESS_DENIED_ERROR 		1044
#define ER_ACCESS_DENIED_ERROR 			1045
#define ER_UNKNOWN_TABLE 				1109
#define ER_SYNTAX_ERROR 				1149
#define CR_SERVER_GONE_ERROR 			2006
#define CR_SERVER_LOST 					2013
#define CR_COMMAND_OUT_OF_SYNC 			2014
#define CR_SERVER_LOST_EXTENDED 		2055


#if !defined E_LOGLEVEL
enum E_LOGLEVEL
{
	NONE = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 4,
	ERROR = 8,
	
	ALL = ERROR | WARNING | INFO | DEBUG
};
#endif

enum E_ORM_ERROR
{
	ERROR_INVALID,
	ERROR_OK,
	ERROR_NO_DATA
};

enum E_MYSQL_GLOBAL_OPTION
{
	DUPLICATE_CONNECTIONS,
	DUPLICATE_CONNECTION_WARNING
};

enum E_MYSQL_OPTION
{
	AUTO_RECONNECT,
	MULTI_STATEMENTS,
	POOL_SIZE,
	SERVER_PORT,
	SSL_ENABLE,
	SSL_KEY_FILE,
	SSL_CERT_FILE,
	SSL_CA_FILE,
	SSL_CA_PATH,
	SSL_CIPHER
};

enum E_MYSQL_FIELD_TYPE
{
	MYSQL_TYPE_INVALID = -1,
	MYSQL_TYPE_DECIMAL = 0, 
	MYSQL_TYPE_TINY,
	MYSQL_TYPE_SHORT,  
	MYSQL_TYPE_LONG,
	MYSQL_TYPE_FLOAT,  
	MYSQL_TYPE_DOUBLE,
	MYSQL_TYPE_NULL,   
	MYSQL_TYPE_TIMESTAMP,
	MYSQL_TYPE_LONGLONG,
	MYSQL_TYPE_INT24,
	MYSQL_TYPE_DATE,   
	MYSQL_TYPE_TIME,
	MYSQL_TYPE_DATETIME, 
	MYSQL_TYPE_YEAR,
	MYSQL_TYPE_NEWDATE, 
	MYSQL_TYPE_VARCHAR,
	MYSQL_TYPE_BIT,
	MYSQL_TYPE_TIMESTAMP2,
	MYSQL_TYPE_DATETIME2,
	MYSQL_TYPE_TIME2,
	MYSQL_TYPE_JSON = 245,
	MYSQL_TYPE_NEWDECIMAL = 246,
	MYSQL_TYPE_ENUM = 247,
	MYSQL_TYPE_SET = 248,
	MYSQL_TYPE_TINY_BLOB = 249,
	MYSQL_TYPE_MEDIUM_BLOB = 250,
	MYSQL_TYPE_LONG_BLOB = 251,
	MYSQL_TYPE_BLOB = 252,
	MYSQL_TYPE_VAR_STRING = 253,
	MYSQL_TYPE_STRING = 254,
	MYSQL_TYPE_GEOMETRY = 255
};

enum E_MYSQL_EXECTIME_UNIT
{
	MILLISECONDS,
	MICROSECONDS
};

#define MYSQL_DEFAULT_HANDLE MySQL:1
#define MYSQL_INVALID_HANDLE MySQL:0
#define MYSQL_INVALID_CACHE Cache:0
#define MYSQL_INVALID_ORM ORM:0



// ORM functions
native ORM:orm_create(const table[], MySQL:handle = MYSQL_DEFAULT_HANDLE);
native orm_destroy(ORM:id);

native E_ORM_ERROR:orm_errno(ORM:id);

native orm_apply_cache(ORM:id, row_idx, result_idx = 0);

native orm_select(ORM:id, const callback[] = "", const format[] = "", {Float, _}:...);
native orm_update(ORM:id, const callback[] = "", const format[] = "", {Float, _}:...);
native orm_insert(ORM:id, const callback[] = "", const format[] = "", {Float, _}:...);
native orm_delete(ORM:id, const callback[] = "", const format[] = "", {Float, _}:...);

native orm_load(ORM:id, const callback[] = "", const format[] = "", {Float, _}:...) = orm_select;
native orm_save(ORM:id, const callback[] = "", const format[] = "", {Float, _}:...);

native orm_addvar_int(ORM:id, &var, const columnname[]);
native orm_addvar_float(ORM:id, &Float:var, const columnname[]);
native orm_addvar_string(ORM:id, var[], var_maxlen, const columnname[]);

native orm_clear_vars(ORM:id);
native orm_delvar(ORM:id, const columnname[]);
native orm_setkey(ORM:id, const columnname[]);


// MySQL functions
native mysql_log(E_LOGLEVEL:loglevel = ERROR | WARNING);
native MySQL:mysql_connect(const host[], const user[], const password[], const database[], MySQLOpt:option_id = MySQLOpt:0);
native MySQL:mysql_connect_file(const file_name[] = "mysql.ini");
native mysql_close(MySQL:handle = MYSQL_DEFAULT_HANDLE);

native mysql_unprocessed_queries(MySQL:handle = MYSQL_DEFAULT_HANDLE);
native mysql_global_options(E_MYSQL_GLOBAL_OPTION:type, value);

native MySQLOpt:mysql_init_options();
native mysql_set_option(MySQLOpt:option_id, E_MYSQL_OPTION:type, ...);

native mysql_pquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
native mysql_tquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
native Cache:mysql_query(MySQL:handle, const query[], bool:use_cache = true);
native mysql_tquery_file(MySQL:handle, const file_path[], const callback[] = "", const format[] = "", {Float,_}:...);
native Cache:mysql_query_file(MySQL:handle, const file_path[], bool:use_cache = false);

native mysql_errno(MySQL:handle = MYSQL_DEFAULT_HANDLE);
native mysql_error(destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);
native mysql_escape_string(const source[], destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);
native mysql_format(MySQL:handle, output[], max_len, const format[], {Float,_}:...);
native mysql_set_charset(const charset[], MySQL:handle = MYSQL_DEFAULT_HANDLE);
native mysql_get_charset(destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);
native mysql_stat(destination[], max_len = sizeof(destination), MySQL:handle = MYSQL_DEFAULT_HANDLE);



// Cache functions
native cache_get_row_count(&destination);
native cache_get_field_count(&destination);
native cache_get_result_count(&destination);
native cache_get_field_name(field_index, destination[], max_len = sizeof(destination));
native E_MYSQL_FIELD_TYPE:cache_get_field_type(field_index);
native cache_set_result(result_index);

stock cache_num_rows()
{
	new row_count;
	cache_get_row_count(row_count);
	return row_count;
}
stock cache_num_fields()
{
	new field_count;
	cache_get_field_count(field_count);
	return field_count;
}
stock cache_num_results()
{
	new result_count;
	cache_get_result_count(result_count);
	return result_count;
}


//  overload macros for cache_get_value natives
#define cache_get_value(%1) (_:MSCGV0:MSCGV1:MSCGV2:cache_get_ovrld_value(%1))
#define MSCGV0:MSCGV1:MSCGV2:cache_get_ovrld_value(%1,"%2",%3) cache_get_value_name(%1,#%2,%3)
#define MSCGV1:MSCGV2:cache_get_ovrld_value(%1,%8string%9:%2,%3) cache_get_value_name(%1,%2,%3)
#define MSCGV2:cache_get_ovrld_value(%1,%2,%3) cache_get_value_index(%1,%2,%3)

#define cache_get_value_int(%1) (_:MSCGVI0:MSCGVI1:MSCGVI2:cache_get_value_int_ovrld(%1))
#define MSCGVI0:MSCGVI1:MSCGVI2:cache_get_value_int_ovrld(%1,"%2",%3) cache_get_value_name_int(%1,#%2,%3)
#define MSCGVI1:MSCGVI2:cache_get_value_int_ovrld(%1,%8string%9:%2,%3) cache_get_value_name_int(%1,%2,%3)
#define MSCGVI2:cache_get_value_int_ovrld(%1,%2,%3) cache_get_value_index_int(%1,%2,%3)

#define cache_get_value_float(%1) (_:MSCGVF0:MSCGVF1:MSCGVF2:cache_get_value_float_ovrld(%1))
#define MSCGVF0:MSCGVF1:MSCGVF2:cache_get_value_float_ovrld(%1,"%2",%3) cache_get_value_name_float(%1,#%2,%3)
#define MSCGVF1:MSCGVF2:cache_get_value_float_ovrld(%1,%8string%9:%2,%3) cache_get_value_name_float(%1,%2,%3)
#define MSCGVF2:cache_get_value_float_ovrld(%1,%2,%3) cache_get_value_index_float(%1,%2,%3)

#define cache_get_value_bool(%1) cache_get_value_int(%1)

#define cache_is_value_null(%1) (_:MSCIVN0:MSCIVN1:MSCIVN2:cache_is_value_null_ovrld(%1))
#define MSCIVN0:MSCIVN1:MSCIVN2:cache_is_value_null_ovrld(%1,"%2",%3) cache_is_value_name_null(%1,#%2,%3)
#define MSCIVN1:MSCIVN2:cache_is_value_null_ovrld(%1,%8string%9:%2,%3) cache_is_value_name_null(%1,%2,%3)
#define MSCIVN2:cache_is_value_null_ovrld(%1,%2,%3) cache_is_value_index_null(%1,%2,%3)


native cache_get_value_index(row_idx, column_idx, destination[], max_len = sizeof(destination));
native cache_get_value_index_int(row_idx, column_idx, &destination);
native cache_get_value_index_float(row_idx, column_idx, &Float:destination);
/*
native cache_get_value_index_bool(row_idx, column_idx, &bool:destination);
*/
stock cache_get_value_index_bool(row_idx, column_idx, &bool:destination)
{
    return cache_get_value_index_int(row_idx, column_idx, _:destination);
}
native cache_is_value_index_null(row_idx, column_idx, &bool:destination);

native cache_get_value_name(row_idx, const column_name[], destination[], max_len = sizeof(destination));
native cache_get_value_name_int(row_idx, const column_name[], &destination);
native cache_get_value_name_float(row_idx, const column_name[], &Float:destination);
/*
native cache_get_value_name_bool(row_idx, const column_name[], &bool:destination);
*/
stock cache_get_value_name_bool(row_idx, const column_name[], &bool:destination)
{
    return cache_get_value_name_int(row_idx, column_name, _:destination);
}
native cache_is_value_name_null(row_idx, const column_name[], &bool:destination);

native Cache:cache_save();
native cache_delete(Cache:cache_id);
native cache_set_active(Cache:cache_id);
native cache_unset_active();
native bool:cache_is_any_active();
native bool:cache_is_valid(Cache:cache_id);

native cache_affected_rows();
native cache_insert_id();
native cache_warning_count();

native cache_get_query_exec_time(E_MYSQL_EXECTIME_UNIT:unit = MICROSECONDS);
native cache_get_query_string(destination[], max_len = sizeof(destination));


// Forward declarations
forward OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle);
