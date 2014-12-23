/**
 * MySQL plugin R39-3
 */


#if defined mysql_included
	#endinput
#endif
#define mysql_included


/**
 * Common error codes
 *
 * Client: http://dev.mysql.com/doc/refman/5.1/en/error-messages-client.html
 * Server: http://dev.mysql.com/doc/refman/5.1/en/error-messages-server.html
 */

#define ER_DBACCESS_DENIED_ERROR 		1044
#define ER_ACCESS_DENIED_ERROR 			1045
#define ER_UNKNOWN_TABLE 				1109
#define ER_SYNTAX_ERROR 				1149
#define CR_SERVER_GONE_ERROR 			2006
#define CR_SERVER_LOST 					2013
#define CR_COMMAND_OUT_OF_SYNC 			2014
#define CR_SERVER_LOST_EXTENDED 		2055


enum E_LOGLEVEL
{
	LOG_NONE = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_DEBUG = 4,
	
	LOG_ALL = LOG_ERROR | LOG_WARNING | LOG_DEBUG
};

enum E_LOGTYPE
{
	LOG_TYPE_TEXT = 1,
	LOG_TYPE_HTML = 2
};

enum ORM_Error
{
	ERROR_OK,
	ERROR_NO_DATA
};

enum E_VAR_DATATYPE 
{
	DATATYPE_INT,
	DATATYPE_FLOAT,
	DATATYPE_STRING
};

enum E_MYSQL_OPTION
{
	DUPLICATE_CONNECTIONS,
	LOG_TRUNCATE_DATA
};

enum E_EXECTIME_UNIT
{
	UNIT_MILLISECONDS,
	UNIT_MICROSECONDS
};


#define mysql_real_escape_string 					mysql_escape_string
#define cache_num_fields 							cache_get_field_count
#define cache_num_rows 								cache_get_row_count
#define mysql_function_query(%0,%1,%2,%3,"%4"%5) 	mysql_tquery(%0,%1,%3,#%4%5)
#define mysql_reload(%0) 							mysql_tquery(%0,"FLUSH PRIVILEGES")
#define mysql_debug(%0)								(%0?mysql_log(LOG_ALL):mysql_log())
#define ismysqlnull(%0) 							(strcmp(%0,"NULL",false)==0)


// ORM functions
native ORM:orm_create(const table[], connectionHandle = 1);
native orm_destroy(ORM:id);

native ORM_Error:orm_errno(ORM:id);

native orm_apply_cache(ORM:id, row);
native orm_select(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
/*
native orm_select_inline(ORM:id, callback:Callback, format[], {Float,_}:...); //y_inline
*/
native orm_update(ORM:id);
native orm_insert(ORM:id, callback[] = "", format[] = "", {Float, _}:...);
/*
native orm_insert_inline(ORM:id, callback:Callback, format[], {Float,_}:...); //y_inline
*/
native orm_delete(ORM:id, bool:clearvars=true);

native orm_load(ORM:id, callback[] = "", format[] = "", {Float, _}:...) = orm_select;
native orm_save(ORM:id, callback[] = "", format[] = "", {Float, _}:...);

native orm_addvar_int(ORM:id, &var, varname[]);
native orm_addvar_float(ORM:id, &Float:var, varname[]);
native orm_addvar_string(ORM:id, var[], var_maxlen, varname[]);

native orm_delvar(ORM:id, varname[]);
native orm_setkey(ORM:id, varname[]);


// MySQL functions
native mysql_log(E_LOGLEVEL:loglevel = LOG_ERROR | LOG_WARNING, E_LOGTYPE:logtype = LOG_TYPE_TEXT);
native mysql_connect(const host[], const user[], const database[], const password[], port = 3306, bool:autoreconnect = true, pool_size = 2);
native mysql_close(connectionHandle = 1);
native mysql_reconnect(connectionHandle = 1);

native mysql_unprocessed_queries(connectionHandle = 1);
native mysql_current_handle();
native mysql_option(E_MYSQL_OPTION:type, value);

native mysql_errno(connectionHandle = 1);
native mysql_escape_string(const source[], destination[], connectionHandle = 1, max_len = sizeof(destination));
native mysql_format(connectionHandle, output[], len, format[], {Float,_}:...);
native mysql_pquery(connectionHandle, query[], callback[] = "", format[] = "", {Float,_}:...);
/*
native mysql_pquery_inline(connHandle, query[], callback:Callback, format[], {Float,_}:...); //y_inline
*/
native mysql_tquery(connectionHandle, query[], callback[] = "", format[] = "", {Float,_}:...);
/*
native mysql_tquery_inline(connHandle, query[], callback:Callback, format[], {Float,_}:...); //y_inline
*/
native Cache:mysql_query(conhandle, query[], bool:use_cache = true);

native mysql_stat(destination[], connectionHandle = 1, max_len = sizeof(destination));
native mysql_get_charset(destination[], connectionHandle = 1, max_len = sizeof(destination));
native mysql_set_charset(charset[], connectionHandle = 1);


// Cache functions
native cache_get_data(&num_rows, &num_fields, connectionHandle = 1);
native cache_get_row_count(connectionHandle = 1);
native cache_get_field_count(connectionHandle = 1);
native cache_get_field_name(field_index, destination[], connectionHandle = 1, max_len = sizeof(destination));

native cache_get_row(row, field_idx, destination[], connectionHandle = 1, max_len = sizeof(destination));
native cache_get_row_int(row, field_idx, connectionHandle = 1);
native Float:cache_get_row_float(row, field_idx, connectionHandle = 1);

native cache_get_field_content(row, const field_name[], destination[], connectionHandle = 1, max_len = sizeof(destination));
native cache_get_field_content_int(row, const field_name[], connectionHandle = 1);
native Float:cache_get_field_content_float(row, const field_name[], connectionHandle = 1);

native Cache:cache_save(connectionHandle = 1);
native cache_delete(Cache:cache_id, connectionHandle = 1);
native cache_set_active(Cache:cache_id, connectionHandle = 1);
native cache_is_valid(Cache:cache_id, connectionHandle = 1);

native cache_affected_rows(connectionHandle = 1);
native cache_insert_id(connectionHandle = 1);
native cache_warning_count(connectionHandle = 1);

native cache_get_query_exec_time(E_EXECTIME_UNIT:unit = UNIT_MICROSECONDS);
native cache_get_query_string(destination[], max_len = sizeof(destination));


// Forward declarations
forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);


#if defined MYSQL_USE_YINLINE || defined E_CALLBACK_DATA
	#if !defined E_CALLBACK_DATA
		#include <YSI\y_inline>
	#endif
	
	static g_MySQL_InlineData[1000][E_CALLBACK_DATA];
	static g_MySQL_VarArray[32][YSI_MAX_STRING];
	static g_MySQL_AddressArray[32];

	stock MySQL_Internal_SaveInline(callback:CB) 
	{
		static bool:g_MySQL_LazyInit = true;
		if(g_MySQL_LazyInit == true) 
		{
			//set g_MySQL_InlineData empty
			for(new i=0; i < sizeof(g_MySQL_InlineData); ++i)
				for(new E_CALLBACK_DATA:e = E_CALLBACK_DATA:0; e < E_CALLBACK_DATA; ++e)
					g_MySQL_InlineData[i][e] = 0;
			g_MySQL_LazyInit = false;
		}
		
		for(new i=0; i < sizeof(g_MySQL_InlineData); ++i)
			if(_:g_MySQL_InlineData[i][E_CALLBACK_DATA_POINTER] == 0)
				if (Callback_Get(CB, g_MySQL_InlineData[i]))
					return i;
					
		return -1;
	}
	
	#define mysql_pquery_inline(%0,%1,%2,"%3"%4) \
		mysql_pquery(%0,%1,"FJ37DH3JG_MYSQL_INTERNAL","d"#%3,MySQL_Internal_SaveInline(%2)%4)
		
	#define mysql_tquery_inline(%0,%1,%2,"%3"%4) \
		mysql_tquery(%0,%1,"FJ37DH3JG_MYSQL_INTERNAL","d"#%3,MySQL_Internal_SaveInline(%2)%4)
	
	#define orm_select_inline(%0,%1,"%2"%3) \
		orm_select(%0,"FJ37DH3JG_MYSQL_INTERNAL","d"#%2,MySQL_Internal_SaveInline(%1)%3)
	
	#define orm_insert_inline(%0,%1,"%2"%3) \
		orm_insert(%0,"FJ37DH3JG_MYSQL_INTERNAL","d"#%2,MySQL_Internal_SaveInline(%1)%3)


	forward FJ37DH3JG_MYSQL_INTERNAL(...);
	public FJ37DH3JG_MYSQL_INTERNAL(...) 
	{
		new InlineDataIndex = getarg(0);
		if(InlineDataIndex < 0)
			return 0;
		
		for(new i=0; i < numargs()-1; ++i) 
		{
			for(new l=0; l < YSI_MAX_STRING; ++l) 
			{
				new TmpVal = getarg(i+1, l);
				if(l == 0 || g_MySQL_VarArray[i][l-1] < 256)
					g_MySQL_VarArray[i][l] = TmpVal;
				else 
					break;
			}
			
			g_MySQL_AddressArray[i] = AMX_GetRelativeAddress(g_MySQL_VarArray[i][0]);
		}
		
		Callback_Array(g_MySQL_InlineData[InlineDataIndex], g_MySQL_AddressArray);
		
		Callback_Release(g_MySQL_InlineData[InlineDataIndex]);
		for(new E_CALLBACK_DATA:e = E_CALLBACK_DATA:0; e < E_CALLBACK_DATA; ++e)
			g_MySQL_InlineData[InlineDataIndex][e] = 0;
		
		for(new i=0; i < numargs()-1; ++i) 
		{
			g_MySQL_AddressArray[i] = 0;
			for(new l=0, lmax=strlen(g_MySQL_VarArray[i]); l < lmax; ++l)
				g_MySQL_VarArray[i][l] = 0;
		}
		return 1;
	}
#endif
