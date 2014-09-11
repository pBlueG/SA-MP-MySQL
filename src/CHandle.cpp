#include "CHandle.h"
#include "CQuery.h"


bool CHandle::Execute(ExecutionType type, CQuery *query, CCallback *callback /*= nullptr*/)
{
	if (type == ExecutionType::INVALID || query == nullptr)
		return false;

	switch (type)
	{
	case ExecutionType::THREADED:
		//TODO:
		break;

	case ExecutionType::PARALLEL:
		//TODO:
		break;

	case ExecutionType::UNTHREADED:
		//TODO:
		break;
	}

	return true;
}


CHandle *CHandleManager::Create(string host, string user, string pass, string db,
	size_t port, size_t pool_size)
{
	CHandle::Id_t id = 1;

	while (m_Handles.find(id) != m_Handles.end())
		id++;


	CHandle *handle = new CHandle(id);

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
