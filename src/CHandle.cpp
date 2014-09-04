#include "CHandle.h"


CHandle *CHandleFactory::Create(string host, string user, string pass, string db, 
	size_t port, size_t pool_size, bool reconnect)
{
	CHandle::Id_t id = 1;

	while (m_Handles.find(id) != m_Handles.end())
		id++;


	CHandle *handle = new CHandle(id);

	m_Handles.emplace(id, handle);

	return handle;
}

bool CHandleFactory::Destroy(CHandle *handle)
{
	if (handle == nullptr)
		return false;

	if (m_Handles.erase(handle->GetId()) == 0)
		return false;


	delete handle;
	return true;
}

