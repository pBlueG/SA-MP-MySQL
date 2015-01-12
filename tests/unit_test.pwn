#include <a_samp>
#include <a_mysql>



#define TEST_ORM 1

#define NUM_HANDLES 10


#define FAIL(%0) return printf("[ERROR] "%0)
#define SUCCESS(%0) printf("[OK] "%0)

new sql = -1;
public OnGameModeInit()
{
	/*mysql_log(LOG_ERROR | LOG_WARNING, LOG_TYPE_TEXT);
	mysql_option(E_MYSQL_OPTION:99999, 123456);
	mysql_log(LOG_NONE, LOG_TYPE_HTML);
	mysql_option(E_MYSQL_OPTION:99999, 654321);
	mysql_log(LOG_NONE, LOG_TYPE_TEXT);
	mysql_option(E_MYSQL_OPTION:99999, 654321);
	mysql_log(LOG_ERROR | LOG_WARNING, LOG_TYPE_HTML);
	mysql_option(E_MYSQL_OPTION:99999, 123456);
	mysql_log(LOG_ALL, LOG_TYPE_TEXT);
	mysql_option(E_MYSQL_OPTION:99999, 36987);
	mysql_log(LOG_ALL, LOG_TYPE_HTML);
	mysql_option(E_MYSQL_OPTION:99999, 36987);
	mysql_log(LOG_NONE, LOG_TYPE_HTML);*/

	//connection-should-fail test
	sql = Check_mysql_connect(true);
	Check_mysql_close(sql);
	
	
	//normal functions test
	sql = Check_mysql_connect();
    Check_mysql_reconnect(sql);
	Check_mysql_get_charset(sql);
	Check_mysql_stat(sql);
    Check_mysql_unprocessed_queries(sql, 0);
    Check_mysql_escape_string(sql);
	Check_mysql_close(sql);


	//check if duplicate connections are working
	Check_duplicate_connections();


	//normal functions test with multiple connections at once
	mysql_option(DUPLICATE_CONNECTIONS, true); //allow duplicate connections for this test

	new tmp_handles[NUM_HANDLES];
	for(new i=0; i < NUM_HANDLES; ++i)
	    tmp_handles[i] = Check_mysql_connect();
	for(new i=NUM_HANDLES-1; i >= 0; --i) //run loop backwards
	{
		for(new r=0; r < 10; ++r)
		{
		    Check_mysql_reconnect(tmp_handles[i]);
			Check_mysql_get_charset(tmp_handles[i]);
			Check_mysql_stat(tmp_handles[i]);
		    Check_mysql_unprocessed_queries(tmp_handles[i], 0);
    		Check_mysql_escape_string(tmp_handles[i]);
  		}
	}
	for(new i=0; i < NUM_HANDLES; ++i)
	    Check_mysql_close(tmp_handles[i]);

	mysql_option(DUPLICATE_CONNECTIONS, false); //disallow duplicate connections to restore default behavior


	//establish normal connection for further tests
	sql = Check_mysql_connect();


	//format test
	Check_mysql_format();


	Check_mysql_tquery_callback();

	Check_mysql_tquery_callb_params();

	//SELECT query test
	printf("starting select data test...");
	SetTimer("SelectDataTest", 0, false);
	
	//OnQueryError test
	printf("starting query error callback test...");
	SetTimer("QueryErrorCallbackTest", 0, false);
	
	return 1;
} 

public OnQueryError(errorid, error[], callback[], query[], connectionHandle)
{
	if(strfind(query, "banana_invalid") != -1)
	{
		if(errorid != 1054)
			FAIL("query error callback test failed: invalid error id for field error check (is %d, should be %d)\n\t%s", errorid, 1054, error);
		else
			SUCCESS("query error callback test ok - field error check");
	}
	else if(strfind(query, "XSELECT") != -1)
	{
		if(errorid != 1064)
			FAIL("query error callback test failed: invalid error id for syntax error check (is %d, should be %d)\n\t%s", errorid, 1064, error);
		else
			SUCCESS("query error callback test ok - syntax error check");
	}
	return 1;
}

forward QueryErrorCallbackTest();
public QueryErrorCallbackTest()
{
	mysql_tquery(sql, "SELECT `banana_invalid` FROM `data`", "ValidCallback", "sdf", "str1", 22, 3.14);
	mysql_tquery(sql, "XSELECT `null` FROM `data`");
	return 1;
}

forward SelectDataTest(step);
public SelectDataTest(step)
{
	if(step != 1)
	{
		mysql_tquery(sql, "SELECT * FROM `data`", "SelectDataTest", "d", 1);
	}
	else
	{
		new data_list[][][] = 
		{
			{"tinyint", "111"},
			{"smallint", "2222"},
			{"mediumint", "3333333"},
			{"int", "555555555"},
			{"bigint", "66666666666666"},
			{"float", "3.14159"},
			{"double", "6.283185307179586"},
			{"bit", "\1"},
			{"bool", "33"},
			{"date", "2014-05-23"},
			{"datetime", "2014-05-23 16:22:40"},
			{"timestamp", "2014-05-23 16:28:56"},
			{"time", "13:33:37"},
			{"year", "2011"},
			{"char", "char-string-test"},
			{"varchar", "varchar-string-test"},
			{"text", "text-string-test"},
			{"enum", "b"},
			{"set", "1,5,9"},
			{"null", "NULL"}
		};

		if(cache_num_rows() == 1)
		{
			for(new i=0; i != sizeof(data_list); i++)
			{
				new data[2][64];
				cache_get_field_content(0, data_list[i][0], data[0]);
				for(new f=0; f != cache_num_fields(); f++)
				{
					new fname[32];
					cache_get_field_name(f, fname);
					if(strcmp(fname, data_list[i][0]) == 0)
					{
						cache_get_row(0, f, data[1]);
						break;
					}
				}

				if(strcmp(data[0], data[1]) != 0)
				{
					FAIL("select data test failed: 'cache_get_row' and 'cache_get_field_content' returned different values");
				}

				if(strcmp(data_list[i][1], data[0]) != 0)
				{
					FAIL("select data test failed: field '%s' should be '%s', but is '%s'", \
						data_list[i][0], data_list[i][1], data[0]);
				}
			}
		}
		else
			FAIL("select data test failed: not one row in table");

		SUCCESS("select data test ok");
	}
	return 1;
}

forward CallbackParamCheck(str1[], int2, Float:float3, int4, str5[]);
public CallbackParamCheck(str1[], int2, Float:float3, int4, str5[])
{
	if(strcmp(str1, "str1ng") != 0)
		FAIL("mysql_tquery callback param check string1 failed");

	if(int2 != 222)
		FAIL("mysql_tquery callback param check int2 failed");

	if(float3 != 3.1426)
		FAIL("mysql_tquery callback param check float3 failed");

	if(int4 != 44444)
		FAIL("mysql_tquery callback param check int4 failed");

	if(strcmp(str5, "fi5e") != 0)
		FAIL("mysql_tquery callback param check string5 failed");

	SUCCESS("mysql_tquery callback param check ok");
	return 1;
}

forward ValidCallback(string[], int, float);
public ValidCallback(string[], int, float)
{

	return 1;
}

Check_mysql_tquery_callb_params()
{
	mysql_tquery(sql, "SELECT 1", "CallbackParamCheck", "sdfds", "str1ng", 222, 3.1426, 44444, "fi5e");
	return 1;
}

Check_mysql_tquery_callback()
{
	if(mysql_tquery(sql, "SELECT 1", "InvalidCallback"))
		FAIL("mysql_tquery invalid callback check failed");

	if(mysql_tquery(sql, "SELECT 1", "ValidCallback", "sdf", "banana"))
		FAIL("mysql_tquery parameter count check #1 failed");

	if(mysql_tquery(sql, "SELECT 1", "ValidCallback", "sdf", "banana", 13, 3.14, 21))
		FAIL("mysql_tquery parameter count check #2 failed");

	if(mysql_tquery(sql, ""))
		FAIL("mysql_tquery empty query check failed");

	return 1;
}

Check_mysql_format()
{
	new 
		format_str[256],
		should_be_str[256];
	new format_ret_val;


	format_ret_val = mysql_format(sql, format_str, sizeof(format_str), 
		"int[%i] int-w04[%04d] int[%D] float[%f] float-p2[%.2f] string[%s] escape[%e] hex[%x] HEX[%X] bin[%b]",
	    12, 23, 24, 133.721, 244.123456, "<StrIn2g>", "es'ca\\pe\"me\"", 0xFFDEAD, 0xBEAFFF, 0b11001010);
	format(should_be_str, sizeof should_be_str, "int[12] int-w04[0023] int[24] float[133.721] float-p2[244.12] string[<StrIn2g>] escape[es\\'ca\\\\pe\\\"me\\\"] hex[ffdead] HEX[BEAFFF] bin[11001010]");
	
	if(strcmp(format_str, should_be_str) != 0)
		FAIL("mysql_format specifier check failed\n%s\n%s", format_str, should_be_str);

	if(format_ret_val != strlen(format_str))
		FAIL("mysql_format return value doesn't match actual return string length (specifier check)\n\tis: %d; should: %d", format_ret_val, strlen(should_be_str));


	format_ret_val = mysql_format(sql, format_str, 12, "01234567890123456789");
	format(should_be_str, sizeof should_be_str, "01234567890");

	if(strcmp(format_str, should_be_str) != 0)
		FAIL("mysql_format string size check #1 failed");

	if(format_ret_val != strlen(format_str))
		FAIL("mysql_format return value doesn't match actual return string length (string size check #1)\n\tis: %d; should: %d", format_ret_val, strlen(should_be_str));


	format_ret_val = mysql_format(sql, format_str, 12, "%s", "01234567890123456789");
	format(should_be_str, sizeof should_be_str, "01234567890");

	if(strcmp(format_str, should_be_str) != 0)
		FAIL("mysql_format string size check #2 failed");

	if(format_ret_val != strlen(format_str))
		FAIL("mysql_format return value doesn't match actual return string length (string size check #2)\n\tis: %d; should: %d", format_ret_val, strlen(should_be_str));


	format_ret_val = mysql_format(sql, format_str, 8, "%d", 1234567890);
	format(should_be_str, sizeof should_be_str, "1234567");

	if(strcmp(format_str, should_be_str) != 0)
		FAIL("mysql_format integer length check failed");

	if(format_ret_val != strlen(format_str))
		FAIL("mysql_format return value doesn't match actual return string length (integer length check)\n\tis: %d; should: %d", format_ret_val, strlen(should_be_str));

	return 1;
}
Check_duplicate_connections()
{
	mysql_option(DUPLICATE_CONNECTIONS, true);
	new
		con_a = Check_mysql_connect(),
		con_b = Check_mysql_connect();
	
	if(con_a == con_b)
		FAIL("duplicate connections recognized when they shouldn't have been");

	Check_mysql_close(con_a);
	Check_mysql_close(con_b);


	mysql_option(DUPLICATE_CONNECTIONS, false);
	con_a = Check_mysql_connect();
	con_b = Check_mysql_connect();

	if(con_a != con_b)
		FAIL("duplicate connections were not recognized when they should have been");
	return 1;
}

Check_mysql_escape_string(handle)
{
	static const str[32] = "es'ca\\pe\"me\"";
	new tmp[64];
	mysql_escape_string(str, tmp, handle);
	if(strcmp(tmp, "es\\'ca\\\\pe\\\"me\\\"") != 0)
	    FAIL("escaping a string failed");
	return 1;
}
Check_mysql_connect(bool:fail = false)
{
	new tmp_handle;
	if(fail == true)
	{
	    tmp_handle = mysql_connect("127.0.0.11", "roott", "mysql_test", "1234");
	    if(mysql_errno(tmp_handle) == 0)
	    	FAIL("connection successful even though it should fail");
 	}
	else
	{
	    tmp_handle = mysql_connect("127.0.0.1", "root", "mysql_test", "1234");
		if(mysql_errno(tmp_handle) != 0)
		    FAIL("couldn't establish normal connection");
  	}
	return tmp_handle;
}
Check_mysql_close(handle)
{
	mysql_close(handle);
	if(mysql_errno(handle) == 0)
	    FAIL("mysql_errno returned 0 even though invalid handle");
	return 1;
}
Check_mysql_stat(handle)
{
	new tmp[64];
	mysql_stat(tmp, handle);
	if(mysql_errno(handle) != 0 || strlen(tmp) == 0 || strcmp(tmp, "NULL") == 0)
	    FAIL("mysql_stat failed");
	return 1;
}

Check_mysql_unprocessed_queries(handle, should_be)
{
    new unpr_queries = mysql_unprocessed_queries(handle);
    if(unpr_queries != should_be)
	    FAIL("mysql_unprocessed_queries is '%d' (should be '%d')", unpr_queries, should_be);
	return 1;
}

Check_mysql_get_charset(handle)
{
	new tmp[64];
	mysql_get_charset(tmp, handle);
	if(mysql_errno(handle) != 0 || strlen(tmp) == 0 || strcmp(tmp, "NULL") == 0)
	    FAIL("mysql_get_charset failed");
	return 1;
}

Check_mysql_reconnect(handle)
{
    mysql_reconnect(handle);
	if(mysql_errno(handle) != 0)
	    FAIL("mysql_reconnect failed");
	return 1;
}


public OnGameModeExit()
{
	Check_mysql_close(sql);
	return 1;
}


main() {}
