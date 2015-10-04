#include <a_samp>
#include <a_mysql>
#include <amx\amx_header>



#define TEST_ORM 1

#define MYSQL_HOSTNAME "127.0.0.1"
#define MYSQL_USERNAME "root"
#define MYSQL_PASSWORD ""
#define MYSQL_DATABASE "test"



#define Test:%0() forward Test_%0();public Test_%0()

//cruelessly stolen from YSI's y_testing
#define _Y_TESTEQ(%0) "\"%0\""),0
#define _Y_TESTDQ:_Y_TESTEQ(%0"%1"%2) _Y_TESTDQ:_Y_TESTEQ(%0\x22;%1\x22;%2)
#define _Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0)%1) _Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0\x29;%1)
#define _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0(%1) _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0\x28;%1)

#define ASSERT(%0) if(!(%0))return printf("ASSERT FAILED: %s", _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0)

#define ASSERT_TRUE(%0) ASSERT(!!(%0))



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

public OnGameModeInit()
{
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
	        test_counter++;
	        printf("executing test \"%s\"...", pname);
	        if(CallLocalFunction(pname, "") == 0)
	        {
	            fail_counter++;
	            printf("failed.\n");
		 	}
		 	else
		 	{
	            printf("passed!\n");
		 	}
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

