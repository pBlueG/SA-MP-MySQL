#include "CHandle.h"
#include "CQuery.h"
#include "CConnection.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif



bool CHandle::Execute(ExecutionType type, CQuery *query)
{
	if (type == ExecutionType::INVALID || query == nullptr)
		return false;

	switch (type)
	{
	case ExecutionType::THREADED:
		return m_ThreadedConnection != nullptr ? m_ThreadedConnection->Queue(query) : false;

	case ExecutionType::PARALLEL:
		return m_ConnectionPool != nullptr ? m_ConnectionPool->Queue(query) : false;

	case ExecutionType::UNTHREADED:
		return m_MainConnection != nullptr ? m_MainConnection->Execute(query) : false;
	}

	return false;
}


CHandle *CHandleManager::Create(string host, string user, string pass, string db,
	size_t port, size_t pool_size)
{
	CHandle::Id_t id = 1;

	while (m_Handles.find(id) != m_Handles.end())
		id++;


	CHandle *handle = new CHandle(id);
	
	handle->m_MainConnection = new CConnection(host, user, pass, db, port, true);
	handle->m_ThreadedConnection = new CThreadedConnection(host, user, pass, db, port);
	if (pool_size > 0 && pool_size < 32)
		handle->m_ConnectionPool = new CConnectionPool(pool_size, host, user, pass, db, port);

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
