#include "CQuery.h"
#include "CHandle.h"
#include "CConnection.h"
#include "COptions.h"

#ifdef WIN32
	#define NOMINMAX //goddamnit Microsoft
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif


CHandle::~CHandle()
{
	if (m_MainConnection != nullptr)
		delete m_MainConnection;

	if (m_ThreadedConnection != nullptr)
		delete m_ThreadedConnection;

	if (m_ConnectionPool != nullptr)
		delete m_ConnectionPool;
}

bool CHandle::Execute(ExecutionType type, CQuery::Type_t query)
{
	if (query)
	{
		switch (type)
		{
		case ExecutionType::THREADED:
			return m_ThreadedConnection != nullptr ? m_ThreadedConnection->Queue(query) : false;

		case ExecutionType::PARALLEL:
			return m_ConnectionPool != nullptr ? m_ConnectionPool->Queue(query) : false;

		case ExecutionType::UNTHREADED:
			return m_MainConnection != nullptr ? m_MainConnection->Execute(query) : false;
		}
	}

	return false;
}


CHandle *CHandleManager::Create(string host, string user, string pass, string db,
	const COptions *options, CHandle::Error &error)
{
	error = CHandle::Error::NONE;
	if (host.empty())
	{
		error = CHandle::Error::EMPTY_HOST;
		return nullptr;
	}

	if (user.empty())
	{
		error = CHandle::Error::EMPTY_USER;
		return nullptr;
	}

	if (pass.empty())
		;//TODO: warning

	if (db.empty())
	{
		error = CHandle::Error::EMPTY_DATABASE;
		return nullptr;
	}

	if (options == nullptr)
	{
		error = CHandle::Error::INVALID_OPTIONS;
		return nullptr;
	}


	CHandle::Id_t id = 1;
	while (m_Handles.find(id) != m_Handles.end())
		id++;

	CHandle *handle = new CHandle(id);
	
	handle->m_MainConnection = new CConnection(host, user, pass, db, options);
	handle->m_ThreadedConnection = new CThreadedConnection(host, user, pass, db, options);

	auto pool_size = options->GetOption<unsigned int>(COptions::Type::POOL_SIZE);
	if (pool_size != 0)
		handle->m_ConnectionPool = new CConnectionPool(pool_size, host, user, pass, db, options);

	m_Handles.emplace(id, handle);
	return handle;
}

bool CHandleManager::Destroy(CHandle *handle)
{
	if (handle == nullptr)
		return false;

	if (m_Handles.erase(handle->GetId()) == 0)
		return false;


	delete handle;
	return true;
}
