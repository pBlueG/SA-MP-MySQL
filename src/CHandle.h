#pragma once

#include "CSingleton.h"

#include <string>
#include <unordered_map>
#include <boost/thread/thread.hpp>
#include <boost/thread/sync_bounded_queue.hpp>
#include <boost/atomic.hpp>

using std::string;
using std::unordered_map;
using boost::atomic;

class CConnection;
class CQuery;
class CCallback;


class CHandle
{
	friend class CHandleManager;
public: //type definitions
	using Id_t = unsigned int;

	enum class ExecutionType
	{
		INVALID,
		THREADED,
		PARALLEL,
		UNTHREADED
	};


private: //constructor / deconstructor
	CHandle(Id_t id) :
		m_Id(id),
		m_ThreadsRunning(true),
		m_QueryQueue(32768),
		m_ParallelQueue(32768)
	{ }
	~CHandle() = default;


private: //variables
	const Id_t m_Id;

	boost::thread *m_QueryThread = nullptr;
	boost::thread_group m_ParallelThreads;
	atomic<bool> m_ThreadsRunning;

	//query queues
	boost::sync_bounded_queue<CQuery *> m_QueryQueue;
	boost::sync_bounded_queue<CQuery *> m_ParallelQueue;

	CConnection
		*m_MainConnection = nullptr,
		*m_ThreadConnection = nullptr;



public: //functions
	bool Execute(ExecutionType type, CQuery *query);


	inline Id_t GetId() const
	{
		return m_Id;
	}

};

class CHandleManager : public CSingleton<CHandleManager>
{
	friend class CSingleton<CHandleManager>;
private: //constructor / deconstructor
	CHandleManager() = default;
	~CHandleManager() = default;


private: //variables
	unordered_map<CHandle::Id_t, CHandle *> m_Handles;


public: //functions
	CHandle *Create(string host, string user, string pass, string db, 
		size_t port, size_t pool_size);
	bool Destroy(CHandle * handle);

	inline bool IsValidHandle(const CHandle::Id_t id)
	{
		return m_Handles.find(id) != m_Handles.end();
	}
	inline CHandle *GetHandle(const CHandle::Id_t id)
	{
		return IsValidHandle(id) ? m_Handles.at(id) : nullptr;
	}
};
