#define BOOST_TEST_MODULE Default
#include <boost/test/unit_test.hpp>

#include "../../src/CQuery.h"
#include "../../src/CConnection.h"
#include "../../src/CHandle.h"
#include "../../src/CCallback.h"
#include "../../src/CResult.h"


struct GlobalSetup
{
	GlobalSetup()
	{
		mysql_library_init(0, NULL, NULL);
	}

	~GlobalSetup()
	{
		CHandleManager::CSingleton::Destroy();
		mysql_library_end();
	}
};

BOOST_GLOBAL_FIXTURE(GlobalSetup)

BOOST_AUTO_TEST_SUITE(Handle)
//{
	static const string
		host = "localhost",
		user = "root",
		password = "1234",
		database = "mysql_test";
	static const size_t
		port = 3306,
		pool_size = 2;


	struct HandleData
	{
		CHandle::Error handle_error;
		CHandle *handle = reinterpret_cast<CHandle *>(0xCCCCCCCC);
	};

	BOOST_FIXTURE_TEST_CASE(EmptyHost, HandleData)
	{
		//empty host -> fail
		handle = CHandleManager::Get()->Create("", user, password, database, port, pool_size, handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::EMPTY_HOST);
		BOOST_CHECK(handle == nullptr);
	}
	BOOST_FIXTURE_TEST_CASE(EmptyUsername, HandleData)
	{
		//empty username -> fail
		handle = CHandleManager::Get()->Create(host, "", password, database, port, pool_size, handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::EMPTY_USER);// , L"wrong handle-error");
		BOOST_CHECK(handle == nullptr);// , L"handle pointer not null");
	}
	BOOST_FIXTURE_TEST_CASE(EmptyPassword, HandleData)
	{
		//empty password -> pass
		handle = CHandleManager::Get()->Create(host, user, "", database, port, pool_size, handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::NONE);//, L"wrong handle-error");
		BOOST_CHECK(handle != nullptr);// , L"handle pointer is null");
		BOOST_CHECK_MESSAGE(handle->GetId() == 1, L"wrong handle id");
		BOOST_CHECK(CHandleManager::Get()->Destroy(handle));// , L"error while destroying handle");
	}
	BOOST_FIXTURE_TEST_CASE(EmptyDatabase, HandleData)
	{
		//empty database -> fail
		handle = CHandleManager::Get()->Create(host, user, password, "", port, pool_size, handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::EMPTY_DATABASE);//, L"wrong handle-error");
		BOOST_CHECK(handle == nullptr);//, L"handle pointer not null");
	}
	BOOST_FIXTURE_TEST_CASE(InvalidPort, HandleData)
	{
		//invalid port -> fail
		handle = CHandleManager::Get()->Create(host, user, password, database, 543214, pool_size, handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::INVALID_PORT);//, L"wrong handle-error");
		BOOST_CHECK(handle == nullptr);//, L"handle pointer not null");
	}
	BOOST_FIXTURE_TEST_CASE(InvalidPoolSize, HandleData)
	{
		//invalid pool size (> 32) -> fail
		handle = CHandleManager::Get()->Create(host, user, password, database, port, 33, handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::INVALID_POOL_SIZE);//, L"wrong handle-error");
		BOOST_CHECK(handle == nullptr);//, L"handle pointer not null");
	}
//}
BOOST_AUTO_TEST_SUITE_END()
