#pragma once

#include "CSingleton.h"

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

#include "Types.h"

class CConnection;
class CThreadedConnection;
class CConnectionPool;


class CHandle
{
	friend class CHandleManager;
public: //type definitions
	enum class ExecutionType
	{
		INVALID,
		THREADED,
		PARALLEL,
		UNTHREADED
	};

	enum class Error
	{
		NONE,
		EMPTY_HOST,
		EMPTY_USER,
		EMPTY_DATABASE,
		INVALID_OPTIONS,
	};

private: //constructor / deconstructor
	CHandle(HandleId_t id) :
		m_Id(id)
	{ }
	~CHandle();

private: //variables
	const HandleId_t m_Id;

	CConnection *m_MainConnection = nullptr;
	CThreadedConnection *m_ThreadedConnection = nullptr;
	CConnectionPool *m_ConnectionPool = nullptr;

public: //functions
	inline HandleId_t GetId() const
	{
		return m_Id;
	}

	bool Execute(ExecutionType type, Query_t query);
	unsigned int GetErrorId();
	bool EscapeString(const string &src, string &dest);
	bool SetCharacterSet(string charset);
	bool GetCharacterSet(string &charset);
};

class DLL_API CHandleManager : public CSingleton<CHandleManager>
{
	friend class CSingleton<CHandleManager>;
private: //constructor / deconstructor
	CHandleManager() = default;
	~CHandleManager() = default;

private: //variables
	unordered_map<HandleId_t, CHandle *> m_Handles;

public: //functions
	CHandle *Create(string host, string user, string pass, string db, 
		const COptions *options, CHandle::Error &error);
	bool Destroy(CHandle *handle);

	inline bool IsValidHandle(const HandleId_t id)
	{
		return m_Handles.find(id) != m_Handles.end();
	}
	inline CHandle *GetHandle(const HandleId_t id)
	{
		return IsValidHandle(id) ? m_Handles.at(id) : nullptr;
	}

};
