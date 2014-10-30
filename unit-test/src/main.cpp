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

