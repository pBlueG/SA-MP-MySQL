#define BOOST_TEST_MODULE Default
#include <boost/test/unit_test.hpp>

#ifdef WIN32
	#define NOMINMAX
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif

#include "../../src/CQuery.h"
#include "../../src/CConnection.h"
#include "../../src/CHandle.h"
#include "../../src/CCallback.h"
#include "../../src/CResult.h"
#include "../../src/COptions.h"
#include "../../src/CDispatcher.h"

#include <vector>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>


struct GlobalSetup
{
	GlobalSetup()
	{
		mysql_library_init(0, NULL, NULL);
	}

	~GlobalSetup()
	{
		CHandleManager::CSingleton::Destroy();
		CCallbackManager::CSingleton::Destroy();
		CResultSetManager::CSingleton::Destroy();
		CDispatcher::CSingleton::Destroy();
		COptionManager::CSingleton::Destroy();
		
		mysql_library_end();
	}
};
BOOST_GLOBAL_FIXTURE(GlobalSetup)


/*
 *	handle tests
 */
BOOST_AUTO_TEST_SUITE(Handle)


static const string
	host = "localhost",
	user = "root",
	password = "1234",
	database = "mysql_test";

struct HandleData
{
	CHandle::Error handle_error;
	CHandle *handle = reinterpret_cast<CHandle *>(0xCCCCCCCC);
};


BOOST_FIXTURE_TEST_CASE(EmptyHost, HandleData)
{
	//empty host -> fail
	handle = CHandleManager::Get()->Create("", user, password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::EMPTY_HOST);
	BOOST_CHECK(handle == nullptr);
}
BOOST_FIXTURE_TEST_CASE(EmptyUsername, HandleData)
{
	//empty username -> fail
	handle = CHandleManager::Get()->Create(host, "", password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::EMPTY_USER);
	BOOST_CHECK(handle == nullptr);
}
BOOST_FIXTURE_TEST_CASE(EmptyPassword, HandleData)
{
	//empty password -> pass
	handle = CHandleManager::Get()->Create(host, user, "", database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::NONE);
	BOOST_CHECK(handle != nullptr);
	BOOST_CHECK_MESSAGE(handle->GetId() == 1, L"wrong handle id");
	BOOST_CHECK(CHandleManager::Get()->Destroy(handle));
}
BOOST_FIXTURE_TEST_CASE(EmptyDatabase, HandleData)
{
	//empty database -> fail
	handle = CHandleManager::Get()->Create(host, user, password, "", COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::EMPTY_DATABASE);
	BOOST_CHECK(handle == nullptr);
}
BOOST_FIXTURE_TEST_CASE(InvalidOptions, HandleData)
{
	//invalid options -> fail
	handle = CHandleManager::Get()->Create(host, user, password, database, nullptr, handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::INVALID_OPTIONS);
	BOOST_CHECK(handle == nullptr);
}
BOOST_FIXTURE_TEST_CASE(IdEnumeration, HandleData)
{
	//check if id's are correctly distributed

	//create first handle -> id should be 1
	CHandle
		*first_handle = handle = CHandleManager::Get()->Create(host, user, password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error),
		*second_handle = nullptr,
		*third_handle = nullptr;

	BOOST_CHECK(handle_error == CHandle::Error::NONE);
	BOOST_CHECK_EQUAL(handle->GetId(), 1);

	//create 9 other handles, id's should be 2-10
	std::vector<decltype(handle)> handles;
	for (size_t i = 2; i <= 10; ++i)
	{
		handle = CHandleManager::Get()->Create(host, user, password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::NONE);
		BOOST_CHECK_EQUAL(handle->GetId(), i);

		if (i != 2)
			handles.push_back(handle);
		else
			second_handle = handle;
	}

	//delete 3-10, 2 is 'second_handle'
	for (auto &h : handles)
	{
		BOOST_CHECK(CHandleManager::Get()->Destroy(h));
	}
	handles.clear();

	//create 3-10
	for (size_t i = 3; i <= 10; ++i)
	{
		handle = CHandleManager::Get()->Create(host, user, password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
		BOOST_CHECK(handle_error == CHandle::Error::NONE);
		BOOST_CHECK_EQUAL(handle->GetId(), i);

		if (i != 3)
			handles.push_back(handle);
		else
			third_handle = handle;
	}

	//delete 1 ('first_handle')
	BOOST_CHECK(CHandleManager::Get()->Destroy(first_handle));

	//re-create 1
	first_handle = handle = CHandleManager::Get()->Create(host, user, password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::NONE);
	BOOST_CHECK_EQUAL(handle->GetId(), 1);

	//delete 4-10 (3 is 'third_handle')
	for (auto &h : handles)
	{
		BOOST_CHECK(CHandleManager::Get()->Destroy(h));
	}
	handles.clear();

	//check if our three handles are present and valid
	BOOST_CHECK(first_handle != nullptr);
	BOOST_CHECK_EQUAL(first_handle->GetId(), 1);
	BOOST_CHECK(second_handle != nullptr);
	BOOST_CHECK_EQUAL(second_handle->GetId(), 2);
	BOOST_CHECK(third_handle != nullptr);
	BOOST_CHECK_EQUAL(third_handle->GetId(), 3);

	//delete 2
	BOOST_CHECK(CHandleManager::Get()->Destroy(second_handle));

	//re-create 2
	second_handle = handle = CHandleManager::Get()->Create(host, user, password, database, COptionManager::Get()->GetDefaultOptionHandle(), handle_error);
	BOOST_CHECK(handle_error == CHandle::Error::NONE);
	BOOST_CHECK_EQUAL(handle->GetId(), 2);

	//clean-up
	BOOST_CHECK(CHandleManager::Get()->Destroy(first_handle));
	BOOST_CHECK(CHandleManager::Get()->Destroy(second_handle));
	BOOST_CHECK(CHandleManager::Get()->Destroy(third_handle));

}

BOOST_AUTO_TEST_SUITE_END()



/*
*	query tests
*/
struct Connection
{
	CHandle::Error handle_error;
	CHandle *handle = nullptr;

	Connection()
	{
		handle = CHandleManager::Get()->Create("localhost", "root", "1234", "mysql_test",
			COptionManager::Get()->GetDefaultOptionHandle(), handle_error);

		BOOST_REQUIRE(handle_error == CHandle::Error::NONE);
		unsigned int errorid = 0;
		BOOST_REQUIRE_EQUAL(handle->GetErrorId(errorid), true);
		BOOST_REQUIRE_EQUAL(errorid, 0);
	}
	~Connection()
	{
		BOOST_CHECK_EQUAL(CHandleManager::Get()->Destroy(handle), true);
	}
};
BOOST_FIXTURE_TEST_SUITE(Query, Connection)

BOOST_AUTO_TEST_CASE(RawCheckThreaded)
{
	Query_t query = CQuery::Create("SELECT 42 AS `TestField`");
	query->OnExecutionFinished([](ResultSet_t res)
	{
		string dest;
		res->GetActiveResult()->GetRowData(0, 0, dest);
		BOOST_REQUIRE_EQUAL(dest, "42");
		dest.clear();

		res->GetActiveResult()->GetRowDataByName(0, "TestField", dest);
		BOOST_REQUIRE_EQUAL(dest, "42");
	});
	handle->Execute(CHandle::ExecutionType::THREADED, query);
	boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
	CDispatcher::Get()->Process();
}
BOOST_AUTO_TEST_CASE(Empty)
{
	Query_t query = CQuery::Create("");
	query->OnExecutionFinished([](ResultSet_t res)
	{
		BOOST_FAIL("exec callback called");
	});
	handle->Execute(CHandle::ExecutionType::THREADED, query);
	boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
	CDispatcher::Get()->Process();
}

BOOST_AUTO_TEST_SUITE_END()
