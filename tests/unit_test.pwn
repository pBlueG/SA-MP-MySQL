/**
 * ASCII art generated on http://www.network-science.de/ascii/ (font "univers")
 */

#include <a_samp>
#include <a_mysql>
#include <amx\amx_header>



#define MYSQL_HOSTNAME "127.0.0.1"
#define MYSQL_USERNAME "tester"
#define MYSQL_PASSWORD "1234"
#define MYSQL_DATABASE "test"



#define Test:%0() forward Test_%0();public Test_%0()

//cruelessly stolen from YSI's y_testing, with some minor additions
#define _Y_TESTEQ(%0) "\"%0\"", __line),_g_Failed=true
#define _Y_TESTDQ:_Y_TESTEQ(%0"%1"%2) _Y_TESTDQ:_Y_TESTEQ(%0\x22;%1\x22;%2)
#define _Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0)%1) _Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0\x29;%1)
#define _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0(%1) _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0\x28;%1)

#define ASSERT(%0) if(!(%0))printf("ASSERT FAILED: %s (line %d)", _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0)

#define ASSERT_TRUE(%0) ASSERT((%0))
#define ASSERT_FALSE(%0) ASSERT(!(%0))

new bool:_g_Failed = false;

new g_Int;
new Float:g_Float;
new g_String[32];

forward ValidCallback(num, Float:real, string[]);
public ValidCallback(num, Float:real, string[])
	return 1;

MySQL:SetupConnection()
{
	new MySQL:sql = mysql_connect_file();
	if(sql == MYSQL_INVALID_HANDLE || mysql_errno(sql) != 0)
		return mysql_close(sql), MYSQL_INVALID_HANDLE;

	mysql_query_file(sql, "test.sql");
	return sql;
}

TeardownConnection(MySQL:handle)
{
	mysql_query(handle, "DROP TABLE IF EXISTS `test`, `test2`", false);
	mysql_close(handle);
}

FloatCmp(Float:val1, Float:val2)
{
	return floatabs(val1 - val2) <= 0.001;
}


/*
########################################################
########################################################




 ,adPPYba,  8b,dPPYba, 88,dPYba,,adPYba,
a8"     "8a 88P'   "Y8 88P'   "88"    "8a
8b       d8 88         88      88      88
"8a,   ,a8" 88         88      88      88
 `"YbbdP"'  88         88      88      88

########################################################
########################################################
*/

Test:OrmCreate()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;
	
	ASSERT(orm_create("", MYSQL_INVALID_HANDLE) == MYSQL_INVALID_ORM);
	ASSERT(orm_create("", sql) == MYSQL_INVALID_ORM);
	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_destroy(orm));
	
	TeardownConnection(sql);
}

Test:OrmDestroy()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_destroy(MYSQL_INVALID_ORM));
	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}

Test:OrmApplyCache()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;
	new Cache:cache;

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	
	ASSERT((cache = mysql_query(sql, "SELECT * FROM test WHERE `number` = 5")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_num_rows() == 1);
	ASSERT_FALSE(orm_apply_cache(orm, 999));
	ASSERT_FALSE(orm_apply_cache(orm, 0, 999));
	ASSERT_TRUE(orm_apply_cache(orm, 0));
	ASSERT(g_Int == 5);
	ASSERT_TRUE(FloatCmp(g_Float, 3.14211));
	ASSERT(strcmp(g_String, "asdf") == 0);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));
	
	TeardownConnection(sql);
}

Test:OrmSelect()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_select(MYSQL_INVALID_ORM));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	TeardownConnection(sql);
	ASSERT_FALSE(orm_select(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));
	
	sql = SetupConnection();
	
	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	
	ASSERT_FALSE(orm_select(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_FALSE(orm_select(orm));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_select(orm));
	
	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));
	
	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_select(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_FALSE(orm_select(orm));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_select(orm));
	ASSERT_FALSE(orm_select(orm, "InvalidCallback"));
	ASSERT_TRUE(orm_select(orm, "ValidCallback"));
	ASSERT_FALSE(orm_select(orm, "ValidCallback", "dJd", 123, 456, 789));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));
	
	TeardownConnection(sql);
}

Test:OrmUpdate()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_update(MYSQL_INVALID_ORM));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	TeardownConnection(sql);
	ASSERT_FALSE(orm_update(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	sql = SetupConnection();

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_update(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_FALSE(orm_update(orm));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_update(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_update(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_FALSE(orm_update(orm));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_update(orm));
	ASSERT_FALSE(orm_update(orm, "InvalidCallback"));
	ASSERT_TRUE(orm_update(orm, "ValidCallback"));
	ASSERT_FALSE(orm_update(orm, "ValidCallback", "dJd", 123, 456, 789));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}

Test:OrmInsert()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_insert(MYSQL_INVALID_ORM));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	TeardownConnection(sql);
	ASSERT_FALSE(orm_insert(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	sql = SetupConnection();

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_insert(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_FALSE(orm_insert(orm));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_insert(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_insert(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_FALSE(orm_insert(orm));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_insert(orm));
	ASSERT_FALSE(orm_insert(orm, "InvalidCallback"));
	ASSERT_TRUE(orm_insert(orm, "ValidCallback"));
	ASSERT_FALSE(orm_insert(orm, "ValidCallback", "dJd", 123, 456, 789));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}

Test:OrmDelete()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_delete(MYSQL_INVALID_ORM));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	TeardownConnection(sql);
	ASSERT_FALSE(orm_delete(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	sql = SetupConnection();

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_delete(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_FALSE(orm_delete(orm));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_delete(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_delete(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_delete(orm));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_delete(orm));
	ASSERT_FALSE(orm_delete(orm, "InvalidCallback"));
	ASSERT_TRUE(orm_delete(orm, "ValidCallback"));
	ASSERT_FALSE(orm_delete(orm, "ValidCallback", "dJd", 123, 456, 789));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}

Test:OrmLoad()
{
	//literally the same function as orm_select
}

Test:OrmSave()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_save(MYSQL_INVALID_ORM));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	TeardownConnection(sql);
	ASSERT_FALSE(orm_save(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	sql = SetupConnection();

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_save(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_FALSE(orm_save(orm));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_save(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_save(orm));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_FALSE(orm_save(orm));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_TRUE(orm_save(orm));
	ASSERT_FALSE(orm_save(orm, "InvalidCallback"));
	ASSERT_TRUE(orm_save(orm, "ValidCallback"));
	ASSERT_FALSE(orm_save(orm, "ValidCallback", "dJd", 123, 456, 789));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));
	
	TeardownConnection(sql);
}

Test:OrmAddVar()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_addvar_int(MYSQL_INVALID_ORM, g_Int, "number"));
	ASSERT_FALSE(orm_addvar_float(MYSQL_INVALID_ORM, g_Float, "float"));
	ASSERT_FALSE(orm_addvar_string(MYSQL_INVALID_ORM, g_String, sizeof g_String, "text"));
	
	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	
	ASSERT_FALSE(orm_addvar_int(orm, g_Int, ""));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	
	ASSERT_FALSE(orm_addvar_float(orm, g_Float, ""));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	
	ASSERT_FALSE(orm_addvar_string(orm, g_String, sizeof g_String, ""));
	ASSERT_FALSE(orm_addvar_string(orm, g_String, 0, ""));
	ASSERT_FALSE(orm_addvar_string(orm, g_String, -500, ""));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));
	
	TeardownConnection(sql);
}

Test:OrmDelVar()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;
	
	ASSERT_FALSE(orm_delvar(MYSQL_INVALID_ORM, "asdf"));
	
	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_FALSE(orm_delvar(orm, ""));
	ASSERT_FALSE(orm_delvar(orm, "invalid"));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_delvar(orm, "number"));
	ASSERT_FALSE(orm_delvar(orm, "number"));
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_delete(orm));
	ASSERT_TRUE(orm_delvar(orm, "number"));
	ASSERT_FALSE(orm_delete(orm)); //no key registered
	ASSERT_FALSE(orm_delvar(orm, "number"));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}

Test:OrmClearVars()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_clear_vars(MYSQL_INVALID_ORM));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);

	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	g_Int = 1234;
	g_Float = 5.678;
	g_String = "910";
	ASSERT_TRUE(orm_clear_vars(orm));
	ASSERT(g_Int == 0);
	ASSERT_TRUE(FloatCmp(g_Float, 0.0));
	ASSERT(g_String[0] == '\0');

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}

Test:OrmSetKey()
{
	new MySQL:sql = SetupConnection();
	new ORM:orm;

	ASSERT_FALSE(orm_setkey(MYSQL_INVALID_ORM, "number"));

	ASSERT((orm = orm_create("test", sql)) != MYSQL_INVALID_ORM);
	
	ASSERT_TRUE(orm_addvar_int(orm, g_Int, "number"));
	ASSERT_TRUE(orm_addvar_string(orm, g_String, sizeof g_String, "text"));
	ASSERT_TRUE(orm_addvar_float(orm, g_Float, "float"));
	ASSERT_FALSE(orm_setkey(orm, ""));
	ASSERT_FALSE(orm_delete(orm)); // no key
	ASSERT_TRUE(orm_setkey(orm, "number"));
	ASSERT_TRUE(orm_delete(orm));

	ASSERT_TRUE(orm_destroy(orm));
	ASSERT_FALSE(orm_destroy(orm));

	TeardownConnection(sql);
}






/*
########################################################
########################################################

                                 88
                                 88
                                 88
 ,adPPYba, ,adPPYYba,  ,adPPYba, 88,dPPYba,   ,adPPYba,
a8"     "" ""     `Y8 a8"     "" 88P'    "8a a8P_____88
8b         ,adPPPPP88 8b         88       88 8PP"""""""
"8a,   ,aa 88,    ,88 "8a,   ,aa 88       88 "8b,   ,aa
 `"Ybbd8"' `"8bbdP"Y8  `"Ybbd8"' 88       88  `"Ybbd8"'

########################################################
########################################################
*/

Test:CacheCount()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new rows, fields, results;

	ASSERT_FALSE(cache_get_row_count(rows));
	ASSERT_FALSE(cache_get_field_count(fields));
	ASSERT_FALSE(cache_get_result_count(results));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_result_count(results));
	ASSERT(results == 1);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());
	
	ASSERT_FALSE(cache_get_row_count(rows));
	ASSERT_FALSE(cache_get_field_count(fields));
	ASSERT_FALSE(cache_get_result_count(results));

	TeardownConnection(sql);
}

Test:CacheGetFieldName()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest[32];
	new fields;

	ASSERT_FALSE(cache_get_field_name(0, dest));
	
	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_field_name(0, dest));
	ASSERT(strcmp(dest, "number") == 0);
	ASSERT_TRUE(cache_get_field_name(1, dest));
	ASSERT(strcmp(dest, "text") == 0);
	ASSERT_TRUE(cache_get_field_name(2, dest));
	ASSERT(strcmp(dest, "float") == 0);
	ASSERT_FALSE(cache_get_field_name(3, dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());
	
	ASSERT_FALSE(cache_get_field_name(0, dest));

	TeardownConnection(sql);
}

Test:CacheGetFieldType()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new fields;

	ASSERT(cache_get_field_type(0) == MYSQL_TYPE_INVALID);

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT(cache_get_field_type(0) == MYSQL_TYPE_LONG);
	ASSERT(cache_get_field_type(1) == MYSQL_TYPE_VAR_STRING);
	ASSERT(cache_get_field_type(2) == MYSQL_TYPE_FLOAT);
	ASSERT(cache_get_field_type(3) == MYSQL_TYPE_INVALID);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT(cache_get_field_type(0) == MYSQL_TYPE_INVALID);

	TeardownConnection(sql);
}

Test:CacheSetResult()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest;
	new results;

	ASSERT_FALSE(cache_set_result(0));

	ASSERT((cache = mysql_query(sql, "SELECT 1; SELECT 2; SELECT 4;")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_result_count(results));
	ASSERT(results == 3);
	ASSERT_TRUE(cache_get_value_index_int(0, 0, dest));
	ASSERT(dest == 1);
	ASSERT_TRUE(cache_set_result(1));
	ASSERT_TRUE(cache_get_value_index_int(0, 0, dest));
	ASSERT(dest == 2);
	ASSERT_TRUE(cache_set_result(2));
	ASSERT_TRUE(cache_get_value_index_int(0, 0, dest));
	ASSERT(dest == 4);
	ASSERT_FALSE(cache_set_result(3));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_set_result(0));

	TeardownConnection(sql);
}

Test:CacheGetValueIndex()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest[32];
	new rows, fields;

	ASSERT_FALSE(cache_get_value_index(0, 0, dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_value_index(0, 0, dest));
	ASSERT(strcmp(dest, "1") == 0);
	ASSERT_TRUE(cache_get_value_index(5, 0, dest));
	ASSERT(strcmp(dest, "6") == 0);
	ASSERT_TRUE(cache_get_value_index(3, 1, dest));
	ASSERT(strcmp(dest, "asdf") == 0);
	ASSERT_TRUE(cache_get_value_index(9, 2, dest));
	ASSERT(strcmp(dest, "3.14", false, 4) == 0);
	ASSERT_FALSE(cache_get_value_index(10, 2, dest));
	ASSERT_FALSE(cache_get_value_index(2, 3, dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_value_index(0, 0, dest));

	TeardownConnection(sql);
}

Test:CacheGetValueIndexInt()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest;
	new rows, fields;

	ASSERT_FALSE(cache_get_value_index_int(0, 0, dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_value_index_int(4, 0, dest));
	ASSERT(dest == 5);
	ASSERT_TRUE(cache_get_value_index_int(9, 0, dest));
	ASSERT(dest == 10);
	ASSERT_FALSE(cache_get_value_index_int(10, 2, dest));
	ASSERT_FALSE(cache_get_value_index_int(2, 3, dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_value_index_int(0, 0, dest));

	TeardownConnection(sql);
}

Test:CacheGetValueIndexFloat()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new Float:dest;
	new rows, fields;

	ASSERT_FALSE(cache_get_value_index_float(0, 0, dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_value_index_float(6, 0, dest));
	ASSERT_TRUE(FloatCmp(dest, 7.0));
	ASSERT_TRUE(cache_get_value_index_float(0, 0, dest));
	ASSERT_TRUE(FloatCmp(dest, 1.0));
	ASSERT_TRUE(cache_get_value_index_float(6, 2, dest));
	ASSERT_TRUE(FloatCmp(dest, 3.14211));
	ASSERT_FALSE(cache_get_value_index_float(10, 2, dest));
	ASSERT_FALSE(cache_get_value_index_float(2, 3, dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_value_index_float(0, 0, dest));

	TeardownConnection(sql);
}

Test:CacheIsValueIndexNull()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new bool:dest = true;
	new rows, fields;

	ASSERT_FALSE(cache_is_value_index_null(0, 0, dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test2")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 2);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 2);
	ASSERT_TRUE(cache_is_value_index_null(1, 0, dest));
	ASSERT(dest == false);
	ASSERT_TRUE(cache_is_value_index_null(0, 1, dest));
	ASSERT(dest == true);
	ASSERT_TRUE(cache_is_value_index_null(1, 1, dest));
	ASSERT(dest == false);
	ASSERT_FALSE(cache_is_value_index_null(5, 1, dest));
	ASSERT_FALSE(cache_is_value_index_null(1, 3, dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_is_value_index_null(0, 0, dest));

	TeardownConnection(sql);
}

Test:CacheGetValueName()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest[32];
	new rows, fields;

	ASSERT_FALSE(cache_get_value_name(0, "text", dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_value_name(1, "number", dest));
	ASSERT(strcmp(dest, "2") == 0);
	ASSERT_TRUE(cache_get_value_name(8, "number", dest));
	ASSERT(strcmp(dest, "9") == 0);
	ASSERT_TRUE(cache_get_value_name(0, "text", dest));
	ASSERT(strcmp(dest, "asdf") == 0);
	ASSERT_TRUE(cache_get_value_name(9, "float", dest));
	ASSERT(strcmp(dest, "3.14", false, 4) == 0);
	ASSERT_FALSE(cache_get_value_name(10, "float", dest));
	ASSERT_FALSE(cache_get_value_name(2, "bananarama", dest));
	ASSERT_FALSE(cache_get_value_name(2, "", dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_value_name(0, "text", dest));

	TeardownConnection(sql);
}

Test:CacheGetValueNameInt()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest;
	new rows, fields;

	ASSERT_FALSE(cache_get_value_name_int(0, "text", dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_value_name_int(8, "number", dest));
	ASSERT(dest == 9);
	ASSERT_TRUE(cache_get_value_name_int(0, "number", dest));
	ASSERT(dest == 1);
	ASSERT_FALSE(cache_get_value_name_int(10, "float", dest));
	ASSERT_FALSE(cache_get_value_name_int(5, "chickenwings", dest));
	ASSERT_FALSE(cache_get_value_name_int(6, "", dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_value_name_int(0, "text", dest));

	TeardownConnection(sql);
}

Test:CacheGetValueNameFloat()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new Float:dest;
	new rows, fields;

	ASSERT_FALSE(cache_get_value_name_float(0, "text", dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_value_name_float(2, "number", dest));
	ASSERT_TRUE(FloatCmp(dest, 3.0));
	ASSERT_TRUE(cache_get_value_name_float(9, "number", dest));
	ASSERT_TRUE(FloatCmp(dest, 10.0));
	ASSERT_TRUE(cache_get_value_name_float(4, "float", dest));
	ASSERT_TRUE(FloatCmp(dest, 3.14211));
	ASSERT_FALSE(cache_get_value_name_float(10, "float", dest));
	ASSERT_FALSE(cache_get_value_name_float(5, "chickenwings", dest));
	ASSERT_FALSE(cache_get_value_name_float(6, "", dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_value_name_float(0, "text", dest));

	TeardownConnection(sql);
}

Test:CacheIsValueNameNull()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new bool:dest = true;
	new rows, fields;

	ASSERT_FALSE(cache_is_value_name_null(0, "text", dest));

	ASSERT((cache = mysql_query(sql, "SELECT * FROM test2")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 2);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 2);
	ASSERT_TRUE(cache_is_value_name_null(1, "id", dest));
	ASSERT(dest == false);
	ASSERT_TRUE(cache_is_value_name_null(0, "text", dest));
	ASSERT(dest == true);
	ASSERT_TRUE(cache_is_value_name_null(1, "text", dest));
	ASSERT(dest == false);
	ASSERT_FALSE(cache_is_value_name_null(5, "text", dest));
	ASSERT_FALSE(cache_is_value_name_null(1, "pancakes", dest));
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_is_value_name_null(0, "text", dest));

	TeardownConnection(sql);
}

Test:CacheSave()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache[3];
	new num_rows, num_fields;

	ASSERT(cache_save() == MYSQL_INVALID_CACHE);
	ASSERT_FALSE(cache_set_active(MYSQL_INVALID_CACHE));
	ASSERT_FALSE(cache_is_valid(MYSQL_INVALID_CACHE));
	ASSERT_FALSE(cache_delete(MYSQL_INVALID_CACHE));
	ASSERT_TRUE(cache_unset_active());
	
	ASSERT((cache[0] = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache[0]));
	ASSERT((cache[1] = cache_save()) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_valid(cache[1]));
	ASSERT(cache[0] != cache[1]);
	ASSERT((cache[2] = mysql_query(sql, "SELECT * FROM test2")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache[2]));
	ASSERT_TRUE(cache_delete(cache[0]));
	ASSERT_FALSE(cache_is_valid(cache[0]));
	ASSERT_FALSE(cache_delete(cache[0]));
	ASSERT_TRUE(cache_is_any_active());
	ASSERT((num_rows = cache_num_rows()) == 2);
	ASSERT((num_fields = cache_num_fields()) == 2);
	ASSERT_TRUE(cache_set_active(cache[1]));
	ASSERT_TRUE(cache_is_any_active());
	ASSERT(cache_num_rows() != num_rows);
	ASSERT(cache_num_fields() != num_fields);
	ASSERT((cache[0] = cache_save()) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_valid(cache[0]));
	ASSERT_TRUE(cache_unset_active());
	ASSERT_FALSE(cache_is_any_active());
	ASSERT_TRUE(cache_delete(cache[1]));
	ASSERT_FALSE(cache_is_valid(cache[1]));
	ASSERT_FALSE(cache_is_any_active());
	ASSERT((cache[1] = cache_save()) == MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_set_active(cache[0]));
	ASSERT_TRUE(cache_is_any_active());
	ASSERT(cache_num_rows() != num_rows);
	ASSERT(cache_num_fields() != num_fields);
	ASSERT_TRUE(cache_delete(cache[0]));
	ASSERT_FALSE(cache_is_valid(cache[0]));
	ASSERT_FALSE(cache_is_any_active());
	ASSERT_TRUE(cache_delete(cache[2]));
	ASSERT_FALSE(cache_is_valid(cache[2]));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT(cache_save() == MYSQL_INVALID_CACHE);

	ASSERT_FALSE(cache_set_active(Cache:-999999999));
	ASSERT_FALSE(cache_is_valid(Cache:-999999999));
	ASSERT_FALSE(cache_delete(Cache:-999999999));
	
	TeardownConnection(sql);
}

Test:CacheAffectedRows()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;

	ASSERT(cache_affected_rows() == -1);

	ASSERT((cache = mysql_query(sql, "UPDATE test SET `text` = 'fdsa'")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_affected_rows() == 10);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());
	
	ASSERT((cache = mysql_query(sql, "DELETE FROM test WHERE `number` % 2 = 1")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_affected_rows() == 5);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT(cache_affected_rows() == -1);
	
	TeardownConnection(sql);
}

Test:CacheWarningCount()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	
	ASSERT(cache_warning_count() == -1);

	ASSERT((cache = mysql_query(sql, "DROP TABLE IF EXISTS `nope`")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_warning_count() == 1);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());
	
	ASSERT((cache = mysql_query(sql, "SELECT 'banana'")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_warning_count() == 0);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT(cache_warning_count() == -1);

	TeardownConnection(sql);
}

Test:CacheInsertId()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;

	ASSERT(cache_insert_id() == -1);

	ASSERT((cache = mysql_query(sql, "INSERT INTO test (`float`, `text`) VALUES (2.56412, 'fdsa')")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_insert_id() == 100);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT(cache_insert_id() == -1);

	TeardownConnection(sql);
}

Test:CacheGetQueryExecTime()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;

	ASSERT(cache_get_query_exec_time(MILLISECONDS) == -1);
	ASSERT(cache_get_query_exec_time(MICROSECONDS) == -1);

	ASSERT((cache = mysql_query(sql, "SELECT SLEEP(3)")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT(cache_get_query_exec_time(MILLISECONDS) >= 2999);
	ASSERT(cache_get_query_exec_time(MICROSECONDS) >= 2999999);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT(cache_get_query_exec_time(MILLISECONDS) == -1);
	ASSERT(cache_get_query_exec_time(MICROSECONDS) == -1);

	TeardownConnection(sql);
}

Test:CacheGetQueryString()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest[256];

	ASSERT_FALSE(cache_get_query_string(dest));

	ASSERT((cache = mysql_query(sql, "SELECT 3, 5, 'banana', 'strawberry'")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_any_active());
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_query_string(dest));
	ASSERT(strcmp(dest, "SELECT 3, 5, 'banana', 'strawberry'") == 0);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	ASSERT_FALSE(cache_is_any_active());

	ASSERT_FALSE(cache_get_query_string(dest));

	TeardownConnection(sql);
}






/*
########################################################
########################################################
                                                         
                                                     88  
                                                     88  
                                                     88  
88,dPYba,,adPYba,  8b       d8 ,adPPYba,  ,adPPYb,d8 88  
88P'   "88"    "8a `8b     d8' I8[    "" a8"    `Y88 88  
88      88      88  `8b   d8'   `"Y8ba,  8b       88 88  
88      88      88   `8b,d8'   aa    ]8I "8a    ,d88 88  
88      88      88     Y88'    `"YbbdP"'  `"YbbdP'88 88  
                       d8'                        88     
                      d8'                         88     

########################################################
########################################################
*/


/*


            ,adPPYb,d8 88       88  ,adPPYba, 8b,dPPYba, 8b       d8
           a8"    `Y88 88       88 a8P_____88 88P'   "Y8 `8b     d8'
           8b       88 88       88 8PP""""""" 88          `8b   d8'
           "8a    ,d88 "8a,   ,a88 "8b,   ,aa 88           `8b,d8'
            `"YbbdP'88  `"YbbdP'Y8  `"Ybbd8"' 88             Y88'
                    88                                       d8'
888888888888        88                                      d8'
*/
Test:UnthreadedQuery()
{
	new MySQL:sql = SetupConnection();
	new Cache:cache;
	new dest_str[32];
	new dest_int;
	new Float:dest_float;
	new rows, fields, results;
	
	ASSERT(sql != MYSQL_INVALID_HANDLE);

	ASSERT(mysql_query(MYSQL_INVALID_HANDLE, "SELECT 1") == MYSQL_INVALID_CACHE);
	
	ASSERT(mysql_query(sql, "SELECT 1", false) == MYSQL_INVALID_CACHE);
	
	ASSERT( (cache = mysql_query(sql, "SELECT 1")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_value_index_int(0, 0, dest_int));
	ASSERT(dest_int == 1);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	
	ASSERT( (cache = mysql_query(sql, "SELECT * FROM test")) != MYSQL_INVALID_CACHE);
	ASSERT_TRUE(cache_is_valid(cache));
	ASSERT_TRUE(cache_get_row_count(rows));
	ASSERT(rows == 10);
	ASSERT_TRUE(cache_get_field_count(fields));
	ASSERT(fields == 3);
	ASSERT_TRUE(cache_get_result_count(results));
	ASSERT(results == 1);
	ASSERT_TRUE(cache_get_value_index_int(4, 0, dest_int));
	ASSERT(dest_int == 5);
	dest_int = 0;
	ASSERT_TRUE(cache_get_value_name_int(4, "number", dest_int));
	ASSERT(dest_int == 5);
	ASSERT_TRUE(cache_get_value_index_float(4, 2, dest_float));
	ASSERT_TRUE(FloatCmp(dest_float, 3.14211));
	dest_float = 0.0;
	ASSERT_TRUE(cache_get_value_name_float(4, "float", dest_float));
	ASSERT_TRUE(FloatCmp(dest_float, 3.14211));
	ASSERT_TRUE(cache_get_value_index(4, 1, dest_str));
	ASSERT(strcmp(dest_str, "asdf") == 0);
	ASSERT_TRUE(cache_get_value_name(4, "text", dest_str));
	ASSERT(strcmp(dest_str, "asdf") == 0);
	ASSERT_TRUE(cache_delete(cache));
	ASSERT_FALSE(cache_is_valid(cache));
	
	TeardownConnection(sql);
}





/*


             ,d
             88
           MM88MMM ,adPPYb,d8 88       88  ,adPPYba, 8b,dPPYba, 8b       d8
             88   a8"    `Y88 88       88 a8P_____88 88P'   "Y8 `8b     d8'
             88   8b       88 88       88 8PP""""""" 88          `8b   d8'
             88,  "8a    ,d88 "8a,   ,a88 "8b,   ,aa 88           `8b,d8'
             "Y888 `"YbbdP'88  `"YbbdP'Y8  `"Ybbd8"' 88             Y88'
                           88                                       d8'
888888888888               88                                      d8'
*/
Test:TQueryFail()
{
    new MySQL:sql = mysql_connect(
	    MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);

	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	
	ASSERT_FALSE(mysql_tquery(MYSQL_INVALID_HANDLE, "SELECT 1"));
	ASSERT_TRUE (mysql_tquery(sql, "SELECT 1"));
	
	ASSERT_FALSE(mysql_tquery(sql, "SELECT 1", "NonExistingCallback"));
	ASSERT_FALSE(mysql_tquery(sql, "SELECT 1", "NonExistingCallback", "dfs", 1));
	ASSERT_FALSE(mysql_tquery(sql, "SELECT 1", "ValidCallback", "dfs", 1));
	ASSERT_FALSE(mysql_tquery(sql, "SELECT 1", "ValidCallback", "dfs", 1, 3.452, "asdf", 2322));
 	ASSERT_TRUE (mysql_tquery(sql, "SELECT 1", "ValidCallback", "dfs", 1, 3.452, "asdf"));
 	ASSERT_TRUE (mysql_tquery(sql, "SELECT 1", "ValidCallback", "dfs", 1, 3.452, ""));
 	ASSERT_FALSE(mysql_tquery(sql, "SELECT 1", "ValidCallback", "dfy", 1, 3.452, "asdf"));
 	ASSERT_FALSE(mysql_tquery(sql, "SELECT 1", "ValidCallback", "daf", 1, {3,4,5}, 3.14));
 	ASSERT_TRUE (mysql_tquery(sql, "SELECT 1", "ValidCallback", "dad", 1, {3,4,5}, 3));
	
	ASSERT_TRUE(mysql_close(sql));
}





/*





             8b,dPPYba,   ,adPPYb,d8 88       88  ,adPPYba, 8b,dPPYba, 8b       d8
             88P'    "8a a8"    `Y88 88       88 a8P_____88 88P'   "Y8 `8b     d8'
             88       d8 8b       88 88       88 8PP""""""" 88          `8b   d8'
             88b,   ,a8" "8a    ,d88 "8a,   ,a88 "8b,   ,aa 88           `8b,d8'
             88`YbbdP"'   `"YbbdP'88  `"YbbdP'Y8  `"Ybbd8"' 88             Y88'
             88                   88                                       d8'
888888888888 88                   88                                      d8'
*/

Test:PQueryFail()
{
    new MySQL:sql = mysql_connect(
	    MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);

	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);

	ASSERT_FALSE(mysql_pquery(MYSQL_INVALID_HANDLE, "SELECT 1"));
	ASSERT_TRUE (mysql_pquery(sql, "SELECT 1"));

	ASSERT_FALSE(mysql_pquery(sql, "SELECT 1", "NonExistingCallback"));
	ASSERT_FALSE(mysql_pquery(sql, "SELECT 1", "NonExistingCallback", "dfs", 1));
	ASSERT_FALSE(mysql_pquery(sql, "SELECT 1", "ValidCallback", "dfs", 1));
	ASSERT_FALSE(mysql_pquery(sql, "SELECT 1", "ValidCallback", "dfs", 1, 3.452, "asdf", 2322));
 	ASSERT_TRUE (mysql_pquery(sql, "SELECT 1", "ValidCallback", "dfs", 1, 3.452, "asdf"));
 	ASSERT_FALSE(mysql_pquery(sql, "SELECT 1", "ValidCallback", "dfy", 1, 3.452, "asdf"));
 	ASSERT_FALSE(mysql_pquery(sql, "SELECT 1", "ValidCallback", "daf", 1, {3,4,5}, 3.14));
 	ASSERT_TRUE (mysql_pquery(sql, "SELECT 1", "ValidCallback", "dad", 1, {3,4,5}, 3));
	
	ASSERT_TRUE(mysql_close(sql));
}





/*
                                                                                         
                                                                                         
                                                                                  ,d     
                                                                                  88     
            ,adPPYba,  ,adPPYba,  8b,dPPYba,  8b,dPPYba,   ,adPPYba,  ,adPPYba, MM88MMM  
           a8"     "" a8"     "8a 88P'   `"8a 88P'   `"8a a8P_____88 a8"     ""   88     
           8b         8b       d8 88       88 88       88 8PP""""""" 8b           88     
           "8a,   ,aa "8a,   ,a8" 88       88 88       88 "8b,   ,aa "8a,   ,aa   88,    
            `"Ybbd8"'  `"YbbdP"'  88       88 88       88  `"Ybbd8"'  `"Ybbd8"'   "Y888  
                                                                                         
888888888888                                                                             

*/

Test:ConnectionFailHost()
{
	new MySQL:sql;

	sql = mysql_connect(
		"", MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);
	ASSERT(sql == MYSQL_INVALID_HANDLE);
	
	sql = mysql_connect(
		"wronghost", MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 2005); //2005 == unknown host
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}

Test:ConnectionFailUser()
{
	new MySQL:sql;

	sql = mysql_connect(
		MYSQL_HOSTNAME, "", MYSQL_PASSWORD, MYSQL_DATABASE);
	ASSERT(sql == MYSQL_INVALID_HANDLE);
	
	sql = mysql_connect(
		MYSQL_HOSTNAME, "wronguser", MYSQL_PASSWORD, MYSQL_DATABASE);
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}

Test:ConnectionFailPassword()
{
	new MySQL:sql;

	//empty password is valid, won't connect though
	//sql = mysql_connect(
	//	MYSQL_HOSTNAME, MYSQL_USERNAME, "", MYSQL_DATABASE);
	//ASSERT(sql != MYSQL_INVALID_HANDLE);
	//ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);

	sql = mysql_connect(
		MYSQL_HOSTNAME, MYSQL_USERNAME, "wrongpass", MYSQL_DATABASE);
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}

Test:ConnectionFailDatabase()
{
	new MySQL:sql;

	sql = mysql_connect(
		MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD, "");
	ASSERT(sql == MYSQL_INVALID_HANDLE);

	sql = mysql_connect(
		MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD, "wrongdb");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 1049); //1049 == unknown database
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}

Test:ConnectionSuccess()
{
	new MySQL:sql = mysql_connect(
	    MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);
	    
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*
                                                                                         
                                                                                         
                                                                                  ,d     
                                                                                  88     
            ,adPPYba,  ,adPPYba,  8b,dPPYba,  8b,dPPYba,   ,adPPYba,  ,adPPYba, MM88MMM  
           a8"     "" a8"     "8a 88P'   `"8a 88P'   `"8a a8P_____88 a8"     ""   88     
           8b         8b       d8 88       88 88       88 8PP""""""" 8b           88     
           "8a,   ,aa "8a,   ,a8" 88       88 88       88 "8b,   ,aa "8a,   ,aa   88,    
            `"Ybbd8"'  `"YbbdP"'  88       88 88       88  `"Ybbd8"'  `"Ybbd8"'   "Y888  
                                                                                         
888888888888                                                                             
                                     
              ad88 88 88             
             d8"   "" 88             
             88       88             
           MM88MMM 88 88  ,adPPYba,  
             88    88 88 a8P_____88  
             88    88 88 8PP"""""""  
             88    88 88 "8b,   ,aa  
             88    88 88  `"Ybbd8"'  
                                     
888888888888                         

*/

Test:ConnectionFileFail()
{
	new MySQL:sql;
	
	sql = mysql_connect_file("notexistent.ini");
	ASSERT(sql == MYSQL_INVALID_HANDLE);

	sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);
	ASSERT_TRUE(mysql_close(sql));
	
	for(new i; i < 5; i++)
	{
	    new file[32];
	    format(file, sizeof file, "mysql-invalid%d.ini", i+1);
		sql = mysql_connect_file(file);
		ASSERT(sql == MYSQL_INVALID_HANDLE);
	}
	return 1;
}

Test:ConnectionFileSuccess()
{
	new MySQL:sql = mysql_connect_file();
	
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*
                                                           
                      88                                   
                      88                                   
                      88                                   
            ,adPPYba, 88  ,adPPYba,  ,adPPYba,  ,adPPYba,  
           a8"     "" 88 a8"     "8a I8[    "" a8P_____88  
           8b         88 8b       d8  `"Y8ba,  8PP"""""""  
           "8a,   ,aa 88 "8a,   ,a8" aa    ]8I "8b,   ,aa  
            `"Ybbd8"' 88  `"YbbdP"'  `"YbbdP"'  `"Ybbd8"'  
                                                           
888888888888                                               
*/

Test:ConnectionCloseFail()
{
	ASSERT_FALSE(mysql_close(MYSQL_INVALID_HANDLE));
	return 1;
}





/*
                                                                                                                                        
                                                                                                                                    88  
                                                                                                                                    88  
                                                                                                                                    88  
           88       88 8b,dPPYba,  8b,dPPYba,  8b,dPPYba,  ,adPPYba,   ,adPPYba,  ,adPPYba, ,adPPYba, ,adPPYba,  ,adPPYba,  ,adPPYb,88  
           88       88 88P'   `"8a 88P'    "8a 88P'   "Y8 a8"     "8a a8"     "" a8P_____88 I8[    "" I8[    "" a8P_____88 a8"    `Y88  
           88       88 88       88 88       d8 88         8b       d8 8b         8PP"""""""  `"Y8ba,   `"Y8ba,  8PP""""""" 8b       88  
           "8a,   ,a88 88       88 88b,   ,a8" 88         "8a,   ,a8" "8a,   ,aa "8b,   ,aa aa    ]8I aa    ]8I "8b,   ,aa "8a,   ,d88  
            `"YbbdP'Y8 88       88 88`YbbdP"'  88          `"YbbdP"'   `"Ybbd8"'  `"Ybbd8"' `"YbbdP"' `"YbbdP"'  `"Ybbd8"'  `"8bbdP"Y8  
                                   88                                                                                                   
888888888888                       88                                                                                                   
                                                                                  
                                                         88                       
                                                         ""                       
                                                                                  
            ,adPPYb,d8 88       88  ,adPPYba, 8b,dPPYba, 88  ,adPPYba, ,adPPYba,  
           a8"    `Y88 88       88 a8P_____88 88P'   "Y8 88 a8P_____88 I8[    ""  
           8b       88 88       88 8PP""""""" 88         88 8PP"""""""  `"Y8ba,   
           "8a    ,d88 "8a,   ,a88 "8b,   ,aa 88         88 "8b,   ,aa aa    ]8I  
            `"YbbdP'88  `"YbbdP'Y8  `"Ybbd8"' 88         88  `"Ybbd8"' `"YbbdP"'  
                    88                                                            
888888888888        88                                                            
*/

Test:ConnectionUnprocQueries()
{
	ASSERT(mysql_unprocessed_queries(MYSQL_INVALID_HANDLE) == -1);
	
	new MySQL:sql = mysql_connect_file();
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	ASSERT(mysql_unprocessed_queries(sql) == 0);
	ASSERT_TRUE(mysql_tquery(sql, "SELECT SLEEP(1)"));
	ASSERT(mysql_unprocessed_queries(sql) == 1);
	
	new tc = GetTickCount();
	while((GetTickCount() - tc) < 1100)
	{ }
	
	ASSERT(mysql_unprocessed_queries(sql) == 0);
	
	mysql_tquery(sql, "SELECT SLEEP(3)");
	mysql_pquery(sql, "SELECT SLEEP(3)");
	mysql_pquery(sql, "SELECT SLEEP(3)");
	
	ASSERT(mysql_unprocessed_queries(sql) == 3);
	
	tc = GetTickCount();
	while((GetTickCount() - tc) < 3300)
	{ }
	
	ASSERT(mysql_unprocessed_queries(sql) == 0);
	
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*
                                                                 
                       88             88                     88  
                       88             88                     88  
                       88             88                     88  
            ,adPPYb,d8 88  ,adPPYba,  88,dPPYba,  ,adPPYYba, 88  
           a8"    `Y88 88 a8"     "8a 88P'    "8a ""     `Y8 88  
           8b       88 88 8b       d8 88       d8 ,adPPPPP88 88  
           "8a,   ,d88 88 "8a,   ,a8" 88b,   ,a8" 88,    ,88 88  
            `"YbbdP"Y8 88  `"YbbdP"'  8Y"Ybbd8"'  `"8bbdP"Y8 88  
            aa,    ,88                                           
888888888888 "Y8bbdP"                                            
                                                                                
                                          88                                    
                                    ,d    ""                                    
                                    88                                          
            ,adPPYba,  8b,dPPYba, MM88MMM 88  ,adPPYba,  8b,dPPYba,  ,adPPYba,  
           a8"     "8a 88P'    "8a  88    88 a8"     "8a 88P'   `"8a I8[    ""  
           8b       d8 88       d8  88    88 8b       d8 88       88  `"Y8ba,   
           "8a,   ,a8" 88b,   ,a8"  88,   88 "8a,   ,a8" 88       88 aa    ]8I  
            `"YbbdP"'  88`YbbdP"'   "Y888 88  `"YbbdP"'  88       88 `"YbbdP"'  
                       88                                                       
888888888888           88                                                       
*/

Test:GlobalOptions()
{
	ASSERT_FALSE(mysql_global_options(E_MYSQL_GLOBAL_OPTION:54321, 432));
	ASSERT_TRUE(mysql_global_options(DUPLICATE_CONNECTIONS, false));
	return 1;
}





/*
                                         
                                         
                                  ,d     
                                  88     
           ,adPPYba,  ,adPPYba, MM88MMM  
           I8[    "" a8P_____88   88     
            `"Y8ba,  8PP"""""""   88     
           aa    ]8I "8b,   ,aa   88,    
           `"YbbdP"'  `"Ybbd8"'   "Y888  
                                         
888888888888                             
                                                                      
                                          88                          
                                    ,d    ""                          
                                    88                                
            ,adPPYba,  8b,dPPYba, MM88MMM 88  ,adPPYba,  8b,dPPYba,   
           a8"     "8a 88P'    "8a  88    88 a8"     "8a 88P'   `"8a  
           8b       d8 88       d8  88    88 8b       d8 88       88  
           "8a,   ,a8" 88b,   ,a8"  88,   88 "8a,   ,a8" 88       88  
            `"YbbdP"'  88`YbbdP"'   "Y888 88  `"YbbdP"'  88       88  
                       88                                             
888888888888           88                                             
*/

Test:ConnectionOption()
{
	ASSERT_FALSE(mysql_set_option(MySQLOpt:54321, AUTO_RECONNECT, true));
	
	new MySQLOpt:option = mysql_init_options();
	
	ASSERT(_:option != 0);
	ASSERT_TRUE(mysql_set_option(option, AUTO_RECONNECT, true));
	ASSERT_TRUE(mysql_set_option(option, POOL_SIZE, 4));
	ASSERT_TRUE(mysql_set_option(option, SSL_KEY_FILE, "banana"));
	
	ASSERT_FALSE(mysql_set_option(option, E_MYSQL_OPTION:54321, true));
	ASSERT_FALSE(mysql_set_option(option, AUTO_RECONNECT));
	return 1;
}





/*
                                                                     
                                                                     
                                                                     
                                                                     
            ,adPPYba, 8b,dPPYba, 8b,dPPYba, 8b,dPPYba,   ,adPPYba,   
           a8P_____88 88P'   "Y8 88P'   "Y8 88P'   `"8a a8"     "8a  
           8PP""""""" 88         88         88       88 8b       d8  
           "8b,   ,aa 88         88         88       88 "8a,   ,a8"  
            `"Ybbd8"' 88         88         88       88  `"YbbdP"'   
                                                                     
888888888888                                                         
*/

Test:ConnectionErrno()
{
	ASSERT(mysql_errno(MYSQL_INVALID_HANDLE) == -1);
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	
	mysql_query(sql, "SELECT 1", false);
	ASSERT(mysql_errno(sql) == 0);
	
	mysql_query(sql, "INVALIDSTATEMENT", false);
	ASSERT(mysql_errno(sql) == 1064);
	
	ASSERT_TRUE(mysql_close(sql));
	ASSERT(mysql_errno(sql) == -1);
	return 1;
}


Test:ConnectionErrorStr()
{
	new error_msg[256];
	ASSERT_FALSE(mysql_error(error_msg, sizeof error_msg, MYSQL_INVALID_HANDLE));
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);
	error_msg[0] = '\0';
	ASSERT_TRUE(mysql_error(error_msg, sizeof error_msg, sql));
	ASSERT(strlen(error_msg) > 0);
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	
	mysql_query(sql, "SELECT 1", false);
	ASSERT(mysql_errno(sql) == 0);
	error_msg[0] = '\0';
	ASSERT_TRUE(mysql_error(error_msg, sizeof error_msg, sql));
	ASSERT(strlen(error_msg) == 0);
	
	mysql_query(sql, "INVALIDSTATEMENT", false);
	ASSERT(mysql_errno(sql) == 1064);
	error_msg[0] = '\0';
	ASSERT_TRUE(mysql_error(error_msg, sizeof error_msg, sql));
	ASSERT(strlen(error_msg) > 0);
	
	ASSERT_TRUE(mysql_close(sql));
	ASSERT_FALSE(mysql_error(error_msg, sizeof error_msg, sql));
	return 1;
}



/*
                                                                                
              ad88                                                              
             d8"                                                         ,d     
             88                                                          88     
           MM88MMM ,adPPYba,  8b,dPPYba, 88,dPYba,,adPYba,  ,adPPYYba, MM88MMM  
             88   a8"     "8a 88P'   "Y8 88P'   "88"    "8a ""     `Y8   88     
             88   8b       d8 88         88      88      88 ,adPPPPP88   88     
             88   "8a,   ,a8" 88         88      88      88 88,    ,88   88,    
             88    `"YbbdP"'  88         88      88      88 `"8bbdP"Y8   "Y888  
                                                                                
888888888888                                                                    
*/

Test:ConnectionFormat()
{
	new dest[512];
	ASSERT_FALSE(mysql_format(MYSQL_INVALID_HANDLE, dest, sizeof dest, "asdf"));
	
	new MySQL:sql = mysql_connect_file();
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	
	ASSERT_FALSE(mysql_format(sql, dest, 0, "asdf"));
	ASSERT_FALSE(mysql_format(sql, dest, sizeof dest, "%d %d", 3));
	ASSERT_FALSE(mysql_format(sql, dest, sizeof dest, "%j %k", 3, 2));
	
	new format_str[512], req_res[512];
	
	dest[0] = '\0';
	format_str = "%%d %d %i %4d %06d";
	req_res = "%d 1234 -4321   12 000999";
	ASSERT(mysql_format(sql, dest, sizeof dest, format_str, 1234, -4321, 12, 999) == strlen(req_res));
	ASSERT(strcmp(dest, req_res) == 0);
	
	//can't check floats since they're not precise
	
	dest[0] = '\0';
	format_str = "%%s %s %s";
	req_res = "%s MyString 123456789";
	ASSERT(mysql_format(sql, dest, sizeof dest, format_str, "MyString", "123456789") == strlen(req_res));
	ASSERT(strcmp(dest, req_res) == 0);
	
	dest[0] = '\0';
	format_str = "%%e %e %e";
	req_res = "%e 123 \\\"Some ol\\' \\\\stri\\ng";
	new escape_param[] = "\"Some ol' \\stri\ng";
	ASSERT(mysql_format(sql, dest, sizeof dest, format_str, "123", escape_param) == strlen(req_res));
	ASSERT(strcmp(dest, req_res) == 0);
	
	dest[0] = '\0';
	format_str = "%%x %x %x";
	req_res = "%x beaff ffde4d";
	ASSERT(mysql_format(sql, dest, sizeof dest, format_str, 781055, 16768589) == strlen(req_res));
	ASSERT(strcmp(dest, req_res) == 0);
	
	dest[0] = '\0';
	format_str = "%%X %X %X";
	req_res = "%X BEAFF FFDE4D";
	ASSERT(mysql_format(sql, dest, sizeof dest, format_str, 781055, 16768589) == strlen(req_res));
	ASSERT(strcmp(dest, req_res) == 0);
	
	dest[0] = '\0';
	format_str = "%%b %b %b";
	req_res = "%b 10111110101011111111 111111111101111001001101";
	ASSERT(mysql_format(sql, dest, sizeof dest, format_str, 781055, 16768589) == strlen(req_res));
	ASSERT(strcmp(dest, req_res) == 0);
	
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*                                                                              
                                                                              
                                                                              
                                                                              
            ,adPPYba, ,adPPYba,  ,adPPYba, ,adPPYYba, 8b,dPPYba,   ,adPPYba,  
           a8P_____88 I8[    "" a8"     "" ""     `Y8 88P'    "8a a8P_____88  
           8PP"""""""  `"Y8ba,  8b         ,adPPPPP88 88       d8 8PP"""""""  
           "8b,   ,aa aa    ]8I "8a,   ,aa 88,    ,88 88b,   ,a8" "8b,   ,aa  
            `"Ybbd8"' `"YbbdP"'  `"Ybbd8"' `"8bbdP"Y8 88`YbbdP"'   `"Ybbd8"'  
                                                      88                      
888888888888                                          88                      
                                                                    
                                        88                          
                       ,d               ""                          
                       88                                           
           ,adPPYba, MM88MMM 8b,dPPYba, 88 8b,dPPYba,   ,adPPYb,d8  
           I8[    ""   88    88P'   "Y8 88 88P'   `"8a a8"    `Y88  
            `"Y8ba,    88    88         88 88       88 8b       88  
           aa    ]8I   88,   88         88 88       88 "8a,   ,d88  
           `"YbbdP"'   "Y888 88         88 88       88  `"YbbdP"Y8  
                                                        aa,    ,88  
888888888888                                             "Y8bbdP"   
*/

Test:StringEscape()
{
	new dest[128];
	
	ASSERT(mysql_escape_string("test", dest, .handle = MYSQL_INVALID_HANDLE) == -1);
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) != 0);
	ASSERT(mysql_escape_string("test", dest, .handle = sql) == -1);
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	
	new short_dest[5];
	ASSERT(mysql_escape_string("-----longstring-----", short_dest, .handle = sql) == -1);
	
	new src[128] = "\"Some ol' \\stri\ng";
	new desired_dest[128] = "\\\"Some ol\\' \\\\stri\\ng";
	ASSERT(mysql_escape_string(src, dest, .handle = sql) == strlen(desired_dest));
	ASSERT(strcmp(dest, desired_dest) == 0);
	
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*
                                         
                                         
                                  ,d     
                                  88     
           ,adPPYba,  ,adPPYba, MM88MMM  
           I8[    "" a8P_____88   88     
            `"Y8ba,  8PP"""""""   88     
           aa    ]8I "8b,   ,aa   88,    
           `"YbbdP"'  `"Ybbd8"'   "Y888  
                                         
888888888888                             
                                                                                      
                      88                                                              
                      88                                                       ,d     
                      88                                                       88     
            ,adPPYba, 88,dPPYba,  ,adPPYYba, 8b,dPPYba, ,adPPYba,  ,adPPYba, MM88MMM  
           a8"     "" 88P'    "8a ""     `Y8 88P'   "Y8 I8[    "" a8P_____88   88     
           8b         88       88 ,adPPPPP88 88          `"Y8ba,  8PP"""""""   88     
           "8a,   ,aa 88       88 88,    ,88 88         aa    ]8I "8b,   ,aa   88,    
            `"Ybbd8"' 88       88 `"8bbdP"Y8 88         `"YbbdP"'  `"Ybbd8"'   "Y888  
                                                                                      
888888888888                                                                          
*/

Test:ConnectionCharsetSet()
{
	ASSERT_FALSE(mysql_set_charset("asdf", MYSQL_INVALID_HANDLE));
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) != 0);
	ASSERT_FALSE(mysql_set_charset("utf8", sql));
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	
	ASSERT_TRUE(mysql_set_charset("utf8", sql));
	ASSERT_FALSE(mysql_set_charset("", sql));
	ASSERT_FALSE(mysql_set_charset("invalid-charset", sql));
	
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*
                                           
                                           
                                    ,d     
                                    88     
            ,adPPYb,d8  ,adPPYba, MM88MMM  
           a8"    `Y88 a8P_____88   88     
           8b       88 8PP"""""""   88     
           "8a,   ,d88 "8b,   ,aa   88,    
            `"YbbdP"Y8  `"Ybbd8"'   "Y888  
            aa,    ,88                     
888888888888 "Y8bbdP"                      
                                                                                      
                      88                                                              
                      88                                                       ,d     
                      88                                                       88     
            ,adPPYba, 88,dPPYba,  ,adPPYYba, 8b,dPPYba, ,adPPYba,  ,adPPYba, MM88MMM  
           a8"     "" 88P'    "8a ""     `Y8 88P'   "Y8 I8[    "" a8P_____88   88     
           8b         88       88 ,adPPPPP88 88          `"Y8ba,  8PP"""""""   88     
           "8a,   ,aa 88       88 88,    ,88 88         aa    ]8I "8b,   ,aa   88,    
            `"Ybbd8"' 88       88 `"8bbdP"Y8 88         `"YbbdP"'  `"Ybbd8"'   "Y888  
                                                                                      
888888888888                                                                          
*/

Test:ConnectionCharsetGet()
{
	new dest[64];
	
	ASSERT_FALSE(mysql_get_charset(dest, .handle = MYSQL_INVALID_HANDLE));
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) != 0);
	ASSERT_FALSE(mysql_get_charset(dest, .handle = sql));
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	
	ASSERT_TRUE(mysql_get_charset(dest, .handle = sql));
	
	new short_dest[2];
	ASSERT_FALSE(mysql_get_charset(short_dest, .handle = sql));
	
	ASSERT_TRUE(mysql_set_charset("utf8", sql));
	ASSERT_TRUE(mysql_get_charset(dest, .handle = sql));
	ASSERT(strcmp(dest, "utf8") == 0);
	
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





/*
													
                                                 
                       ,d                 ,d     
                       88                 88     
           ,adPPYba, MM88MMM ,adPPYYba, MM88MMM  
           I8[    ""   88    ""     `Y8   88     
            `"Y8ba,    88    ,adPPPPP88   88     
           aa    ]8I   88,   88,    ,88   88,    
           `"YbbdP"'   "Y888 `"8bbdP"Y8   "Y888  
                                                 
888888888888                                     
*/

Test:ConnectionStat()
{
	new dest[256];
	
	ASSERT_FALSE(mysql_stat(dest, .handle = MYSQL_INVALID_HANDLE));
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) != 0);
	ASSERT_FALSE(mysql_stat(dest, .handle = sql));
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	
	ASSERT_TRUE(mysql_stat(dest, .handle = sql));
	
	new short_dest[5];
	ASSERT_FALSE(mysql_stat(short_dest, .handle = sql));
	
	ASSERT_TRUE(mysql_close(sql));
	return 1;
}





public OnGameModeInit()
{
	mysql_log(ALL);
	
	new
	    test_counter = 0,
		fail_counter = 0;
	new amxhdr[AMX_HDR];
	GetAmxHeader(amxhdr);

	new num_publics = GetNumPublics(amxhdr);
	for (new i = 0; i < num_publics; i++)
	{
	    new pname[32];
	    GetPublicNameFromIndex(i, pname);
	    if(strfind(pname, "Test_") == 0)
	    {
	        //reset global ORM variables
	        g_Int = 999;
			g_Float = 999.999;
			g_String = "xxxxxx";
			
	        test_counter++;
	        printf("executing test \"%s\"...", pname);
	        CallLocalFunction(pname, "");
	        if(_g_Failed == true)
	        {
	            fail_counter++;
	            printf("failed.\n");
		 	}
		 	else
		 	{
	            printf("passed!\n");
		 	}
		 	_g_Failed = false;
 		}
	}

	if(fail_counter == 0)
	{
	    printf("All tests passed!");
	    SendRconCommand("exit");
	}
	else
	{
	    printf("%d/%d tests failed.", fail_counter, test_counter);
	}
	return 1;
}
main() {}

