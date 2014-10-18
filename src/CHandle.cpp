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
		return m_QueryQueue.nonblocking_push_back(query) == boost::queue_op_status::success;

	case ExecutionType::PARALLEL:
		return m_ParallelQueue.nonblocking_push_back(query) == boost::queue_op_status::success;

	case ExecutionType::UNTHREADED:
		return m_MainConnection->Execute(query);
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
	auto query_thread_func = [=](boost::sync_bounded_queue<CQuery *> &query_queue) mutable
	{
		mysql_thread_init();

		CConnection connection(host, user, pass, db, port, true);
		host.clear();
		user.clear();
		pass.clear();
		db.clear();
		
		while (handle->m_ThreadsRunning)
		{
			while (query_queue.empty() == false)
			{
				CQuery *query = nullptr;
				if (query_queue.nonblocking_pull_front(query) == boost::queue_op_status::success)
				{
					if (connection.Execute(query))
					{
						//CFunctionDispatcher::Get()->Dispatch(std::bind(&CQuery::CallCallback, query));
					}
				}
			}
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		}

		mysql_thread_end();
	};

	handle->m_QueryThread = new boost::thread(std::bind(query_thread_func, std::ref(handle->m_QueryQueue)));
	for (size_t i = 0; i != pool_size; ++i)
		handle->m_ParallelThreads.create_thread(std::bind(query_thread_func, std::ref(handle->m_ParallelQueue)));
	

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
