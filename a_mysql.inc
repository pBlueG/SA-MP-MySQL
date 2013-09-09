/**
 * MySQL plugin R33
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

enum { //log levels
	LOG_NONE = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_DEBUG = 4
};

enum { //log types
	LOG_TYPE_TEXT = 1,
	LOG_TYPE_HTML = 2
};

enum ORM_Error {
	ERROR_OK,
	ERROR_NO_DATA
};

enum VarDatatype {
	DATATYPE_INT,
	DATATYPE_FLOAT,
	DATATYPE_STRING
};


#define mysql_insert_id cache_insert_id
#define mysql_affected_rows cache_affected_rows
#define mysql_warning_count cache_warning_count
#define mysql_real_escape_string mysql_escape_string

// cache_num_fields
#define cache_num_fields cache_get_field_count

// cache_num_rows
#define cache_num_rows cache_get_row_count

// cache_get_field
#define cache_get_field(%0) cache_get_field_name(%0)

// mysql_function_query(conHandle, query[], bool:cache, callback[], format[], {Float,_}:...);
#define mysql_function_query(%0,%1,%2,%3,"%4"%5) mysql_tquery(%0,%1,%3,#%4%5)

// mysql_fetch_row(destination)
#define mysql_fetch_row(%1) mysql_fetch_row_format(%1,"|")

// mysql_next_row()
#define mysql_next_row() mysql_retrieve_row()

// mysql_get_field(fieldname, destination)
#define mysql_get_field(%1,%2) mysql_fetch_field_row(%2,%1)

// ismysqlnull(string[])
#define ismysqlnull(%1) (strcmp(%1,"NULL",false)==0)


// ORM natives
native ORM:orm_create(table[], connectionHandle = 1);
native orm_destroy(ORM:id);

native ORM_Error:orm_errno(ORM:id);

native orm_apply_cache(ORM:id, row);
native orm_select(ORM:id, callback[]="", format[]="", {Float, _}:...);
/*
native orm_select_inline(ORM:id, callback:Callback, format[], {Float,_}:...); //y_inline
*/
native orm_update(ORM:id);
native orm_insert(ORM:id, callback[]="", format[]="", {Float, _}:...);
/*
native orm_insert_inline(ORM:id, callback:Callback, format[], {Float,_}:...); //y_inline
*/
native orm_delete(ORM:id, bool:clearvars=true);

native orm_addvar(ORM:id, &{Float, _}:var, VarDatatype:datatype, var_maxlen, varname[]);
/*
native orm_addvar_int(ORM:id, &var, varname[]);
native orm_addvar_float(ORM:id, &Float:var, varname[]);
native orm_addvar_string(ORM:id, &var, var_maxlen, varname[]);
*/
#define orm_addvar_int(%0,%1,%2) orm_addvar(%0,%1,DATATYPE_INT,1,%2)
#define orm_addvar_float(%0,%1,%2) orm_addvar(%0,%1,DATATYPE_FLOAT,1,%2)
#define orm_addvar_string(%0,%1,%2,%3) orm_addvar(%0,%1,DATATYPE_STRING,%2,%3)

native orm_setkey(ORM:id, varname[]);


// MySQL natives
native mysql_log(loglevel = LOG_ERROR | LOG_WARNING, logtype = LOG_TYPE_TEXT);
native mysql_connect(const host[], const user[], const database[], const password[], port = 3306, bool:autoreconnect = true);
native mysql_close(connectionHandle = 1, bool:wait = true);
native mysql_reconnect(connectionHandle = 1);

native mysql_errno(connectionHandle = 1);
native mysql_escape_string(const source[], destination[], connectionHandle = 1, max_len=sizeof(destination));
native mysql_format(connectionHandle, output[], len, format[], {Float,_}:...);
native mysql_tquery(connectionHandle, query[], callback[], format[], {Float,_}:...);
/*
native mysql_tquery_inline(connHandle, query[], callback:Callback, format[], {Float,_}:...); //y_inline
*/
native Cache:mysql_query(conhandle, query[]);

native mysql_stat(destination[], connectionHandle = 1, max_len=sizeof(destination));
native mysql_get_charset(destination[], connectionHandle = 1, max_len=sizeof(destination));
native mysql_set_charset(charset[], connectionHandle = 1);


// Cache functions.
native cache_get_data(&num_rows, &num_fields, connectionHandle = 1);
native cache_get_row_count(connectionHandle = 1);
native cache_get_field_count(connectionHandle = 1);
native cache_get_field_name(field_index, destination[], connectionHandle = 1, max_len=sizeof(destination));

native cache_get_row(row, field_idx, destination[], connectionHandle = 1, max_len=sizeof(destination));
native cache_get_row_int(row, field_idx, connectionHandle = 1);
native Float:cache_get_row_float(row, field_idx, connectionHandle = 1);

native cache_get_field_content(row, const field_name[], destination[], connectionHandle = 1, max_len=sizeof(destination));
native cache_get_field_content_int(row, const field_name[], connectionHandle = 1);
native Float:cache_get_field_content_float(row, const field_name[], connectionHandle = 1);

native Cache:cache_save(connectionHandle = 1);
native cache_delete(Cache:cache_id, connectionHandle = 1);
native cache_set_active(Cache:cache_id, connectionHandle = 1);

native cache_affected_rows(connectionHandle = 1);
native cache_insert_id(connectionHandle = 1);
native cache_warning_count(connectionHandle = 1);


// Forward declarations.
forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);


#if defined MYSQL_USE_YINLINE || defined E_CALLBACK_DATA
	#if !defined E_CALLBACK_DATA
		#include <YSI\y_inline>
	#endif
	
	new stock g_MySQL_InlineData[1000][E_CALLBACK_DATA];
	new stock g_MySQL_VarArray[32][YSI_MAX_STRING];
	new stock g_MySQL_AddressArray[32];

	stock MySQL_Internal_SaveInline(callback:CB) {
		static bool:g_MySQL_LazyInit = true;
		if(g_MySQL_LazyInit == true) {
			//set g_MySQL_InlineData empty
			for(new i=0; i < sizeof(g_MySQL_InlineData); ++i)
				for(new E_CALLBACK_DATA:e = E_CALLBACK_DATA:0; e < E_CALLBACK_DATA; ++e)
					g_MySQL_InlineData[i][e] = 0;
			g_MySQL_LazyInit = false;
		}
		
		new InlineData[E_CALLBACK_DATA];
		if (Callback_Get(CB, InlineData))
		{
			for(new i=0; i < sizeof(g_MySQL_InlineData); ++i) {
				if(g_MySQL_InlineData[i][E_CALLBACK_DATA_POINTER] == 0) {
					g_MySQL_InlineData[i] = InlineData;
					return i;
				}
			}
		}
		return -1;
	}
	
	#define mysql_tquery_inline(%0,%1,%2,"%3"%4) \
		mysql_tquery(%0,%1,"FJ37DH3JG_MYSQL_INTERNAL","d"#%3,MySQL_Internal_SaveInline(%2)%4)
	
	#define orm_select_inline(%0,%1,"%2"%3) \
		orm_select(%0,"FJ37DH3JG_MYSQL_INTERNAL","d"#%2,MySQL_Internal_SaveInline(%1)%3)
	
	#define orm_insert_inline(%0,%1,"%2"%3) \
		orm_insert(%0,"FJ37DH3JG_MYSQL_INTERNAL","d"#%2,MySQL_Internal_SaveInline(%1)%3)


	forward FJ37DH3JG_MYSQL_INTERNAL(...);
	public FJ37DH3JG_MYSQL_INTERNAL(...) {
		new InlineDataIndex = getarg(0);
		if(InlineDataIndex < 0)
			return 0;
		
		for(new i=0; i < numargs()-1; ++i) {
			for(new l=0; l < YSI_MAX_STRING; ++l) {
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
		
		for(new i=0; i < numargs()-1; ++i) {
			g_MySQL_AddressArray[i] = 0;
			for(new l=0, lmax=strlen(g_MySQL_VarArray[i]); l < lmax; ++l)
				g_MySQL_VarArray[i][l] = 0;
		}
		return 1;
	}
#endif


// Wrappers for cache functions
stock mysql_reload(connectionHandle = 1) {
	mysql_tquery(connectionHandle, "FLUSH PRIVILEGES", "", "");
	return 1;
}

static stock RowIndex[20];
static stock bool:RetrieveRow[20];
stock mysql_store_result(connectionHandle = 1) {
	RowIndex[connectionHandle] = 0;
	RetrieveRow[connectionHandle] = false;
	
}
stock mysql_free_result(connectionHandle = 1) {
	#pragma unused connectionHandle
}
stock mysql_num_rows(connectionHandle = 1) {
	return cache_get_row_count(connectionHandle);
}
stock mysql_num_fields(connectionHandle = 1) {
	return cache_get_field_count(connectionHandle);
}
stock mysql_retrieve_row(connectionHandle = 1) {
	if(RetrieveRow[connectionHandle] != false)
		RowIndex[connectionHandle]++;
	else
		RetrieveRow[connectionHandle] = true;
	
	if(RowIndex[connectionHandle] >= cache_get_row_count(connectionHandle))
		return 0;
	return 1;
}
stock mysql_data_seek(offset, connectionHandle = 1) {
	new Rows = cache_get_row_count(connectionHandle);
	if(offset < 0)
		offset = 0;
	else if(offset >= Rows)
		offset = Rows-1;
	RowIndex[connectionHandle] = offset;
}
stock mysql_fetch_field(number, destination[], connectionHandle = 1, max_len=sizeof(destination)) {
	strdel(destination, 0, max_len-1);
	return cache_get_field(number, destination, connectionHandle, max_len);
}
stock mysql_fetch_field_row(destination[], const fieldname[], connectionHandle = 1, max_len=sizeof(destination)) {
	strdel(destination, 0, max_len-1);
	new RIDX = RowIndex[connectionHandle];
	cache_get_field_content(RIDX < 0 ? 0 : RIDX, fieldname, destination, connectionHandle, max_len);
}
stock mysql_fetch_row_format(destination[], const delimiter[] = "|", connectionHandle = 1, max_len=sizeof(destination)) {
	new retrieve = mysql_retrieve_row(connectionHandle);
	strdel(destination, 0, max_len-1);
	new Fields = cache_get_field_count(connectionHandle);
	for(new f=0; f < Fields; f++) {
		if(f != 0) {
			strcat(destination, delimiter, max_len);
			
		}
		new row_val_tmp[512];
		new RIDX = RowIndex[connectionHandle];
		cache_get_row(RIDX < 0 ? 0 : RIDX, f, row_val_tmp, connectionHandle);
		if(f == 0)
			strins(destination, row_val_tmp, 0, max_len);
		else
			strcat(destination, row_val_tmp, max_len);
	}
	return retrieve;
}

