#define RUN_TESTS
#include <a_samp>
#include <a_mysql>
#include <YSI_Core\y_testing>


#define TEST_ORM 1

#define MYSQL_HOSTNAME "localhost"
#define MYSQL_USERNAME "root"
#define MYSQL_PASSWORD "1234"
#define MYSQL_DATABASE "test"


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
}

Test:ConnectionFailPassword()
{
	new MySQL:sql;

	//empty password is valid, won't connect though
	sql = mysql_connect(
		MYSQL_HOSTNAME, MYSQL_USERNAME, "", MYSQL_DATABASE);
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);

	sql = mysql_connect(
		MYSQL_HOSTNAME, MYSQL_USERNAME, "wrongpass", MYSQL_DATABASE);
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == ER_ACCESS_DENIED_ERROR);
	ASSERT_TRUE(mysql_close(sql));
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
}

Test:ConnectionSuccess()
{
	new MySQL:sql = mysql_connect(
	    MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);
	    
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	ASSERT_TRUE(mysql_close(sql));
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
}

Test:ConnectionFileSuccess()
{
	new MySQL:sql = mysql_connect_file();
	
	ASSERT(sql != MYSQL_INVALID_HANDLE);
	ASSERT(mysql_errno(sql) == 0);
	ASSERT_TRUE(mysql_close(sql));
}

main() {}

