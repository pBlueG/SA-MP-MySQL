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
#define _Y_TESTEQ(%0) "\"%0\""),_g_Failed=true
#define _Y_TESTDQ:_Y_TESTEQ(%0"%1"%2) _Y_TESTDQ:_Y_TESTEQ(%0\x22;%1\x22;%2)
#define _Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0)%1) _Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0\x29;%1)
#define _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0(%1) _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0\x28;%1)

#define ASSERT(%0) if(!(%0))printf("ASSERT FAILED: %s", _Y_TESTOB:_Y_TESTCB:_Y_TESTDQ:_Y_TESTEQ(%0)

#define ASSERT_TRUE(%0) ASSERT(!!(%0))
#define ASSERT_FALSE(%0) ASSERT(!(%0))

new bool:_g_Failed = false;

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
	
	ASSERT_TRUE(_:option != 0);
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
	
	ASSERT_FALSE(mysql_escape_string("test", dest, .handle = MYSQL_INVALID_HANDLE));
	
	new MySQL:sql = mysql_connect_file("mysql-invalid.ini");
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) != 0);
	ASSERT_FALSE(mysql_escape_string("test", dest, .handle = sql));
	ASSERT_TRUE(mysql_close(sql));
	
	sql = mysql_connect_file();
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	
	new short_dest[5];
	ASSERT_FALSE(mysql_escape_string("-----longstring-----", short_dest, .handle = sql));
	
	new src[128] = "\"Some ol' \\stri\ng";
	ASSERT_TRUE(mysql_escape_string(src, dest, .handle = sql));
	new desired_dest[128] = "\\\"Some ol\\' \\\\stri\\ng";
	ASSERT(strcmp(dest, desired_dest) == 0);
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

