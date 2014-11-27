/**
 * MySQL plugin R40
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


enum E_LOGLEVEL
{
	NONE = 0,
	ERROR = 1,
	WARNING = 2,
	DEBUG = 4,
	
	ALL = ERROR | WARNING | DEBUG
};

enum E_LOGTYPE
{
	INVALID,
	TEXT,
	HTML
};

enum E_ORM_ERROR
{
	ERROR_OK,
	ERROR_NO_DATA
};

enum E_MYSQL_OPTION
{
	DUPLICATE_CONNECTIONS,
	AUTO_RECONNECT
	//LOG_TRUNCATE_DATA //TODO: do we really need this?
};

enum E_EXECTIME_UNIT
{
	MILLISECONDS,
	MICROSECONDS
};

#define DEFAULT_MYSQL_HANDLE MySQL:1

#define cache_num_fields 	cache_get_field_count
#define cache_num_rows 		cache_get_row_count


// ORM functions
native ORM:orm_create(const table[], MySQL:handle = DEFAULT_MYSQL_HANDLE);
native orm_destroy(ORM:id);

native E_ORM_ERROR:orm_errno(ORM:id);

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
native mysql_log(E_LOGLEVEL:loglevel = ERROR | WARNING, E_LOGTYPE:logtype = TEXT);
native MySQL:mysql_connect(const host[], const user[], const password[], const database[], port = 3306, bool:autoreconnect = true, pool_size = 2);
native mysql_close(MySQL:handle = DEFAULT_MYSQL_HANDLE);
native mysql_reconnect(MySQL:handle = DEFAULT_MYSQL_HANDLE);

native mysql_unprocessed_queries(MySQL:handle = DEFAULT_MYSQL_HANDLE);
native mysql_current_handle();
native mysql_option(E_MYSQL_OPTION:type, value);

native mysql_pquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
/*
native mysql_pquery_inline(MySQL:handle, const query[], callback:Callback, const format[], {Float,_}:...); //y_inline
*/
native mysql_tquery(MySQL:handle, const query[], const callback[] = "", const format[] = "", {Float,_}:...);
/*
native mysql_tquery_inline(MySQL:handle, const query[], callback:Callback, const format[], {Float,_}:...); //y_inline
*/
native Cache:mysql_query(MySQL:handle, const query[], bool:use_cache = true);

native mysql_errno(MySQL:handle = DEFAULT_MYSQL_HANDLE);
native mysql_format(MySQL:handle, output[], len, const format[], {Float,_}:...);
native mysql_escape_string(const source[], destination[], max_len = sizeof(destination), MySQL:handle = DEFAULT_MYSQL_HANDLE);
native mysql_set_charset(const charset[], MySQL:handle = DEFAULT_MYSQL_HANDLE);
native mysql_get_charset(destination[], max_len = sizeof(destination), MySQL:handle = DEFAULT_MYSQL_HANDLE);
native mysql_stat(destination[], max_len = sizeof(destination), MySQL:handle = DEFAULT_MYSQL_HANDLE);



// Cache functions
native cache_get_data(&num_rows, &num_fields);
native cache_get_row_count();
native cache_get_field_count();
native cache_get_field_name(field_index, destination[], max_len = sizeof(destination));

native cache_get_row(row_idx, field_idx, destination[], max_len = sizeof(destination));
native cache_get_row_int(row_idx, field_idx);
native Float:cache_get_row_float(row_idx, field_idx);

native cache_get_field_content(row_idx, const field_name[], destination[], max_len = sizeof(destination));
native cache_get_field_content_int(row_idx, const field_name[]);
native Float:cache_get_field_content_float(row_idx, const field_name[]);

native Cache:cache_save();
native cache_delete(Cache:cache_id, MySQL:handle = DEFAULT_MYSQL_HANDLE);
native cache_set_active(Cache:cache_id, MySQL:handle = DEFAULT_MYSQL_HANDLE);
native cache_is_valid(Cache:cache_id, MySQL:handle = DEFAULT_MYSQL_HANDLE);

native cache_affected_rows();
native cache_warning_count();
native cache_insert_id();

native cache_get_query_exec_time(E_EXECTIME_UNIT:unit = MICROSECONDS);
native cache_get_query_string(destination[], max_len = sizeof(destination));


// Forward declarations
forward OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle);


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
