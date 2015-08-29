#pragma once

#include "CSingleton.hpp"

#include <string>
#include <unordered_set>

using std::string;
using std::unordered_set;

#include <amx/amx.h>
#include "CError.hpp"
#include "types.hpp"

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
		//file errors
		INVALID_FILE,
		SYNTAX_ERROR,
		UNKNOWN_FIELD,
	};
	static const string ModuleName;
	

private: //constructor / deconstructor
	CHandle() = default;
	~CHandle();

private: //variables
	CConnection *m_MainConnection = nullptr;
	CThreadedConnection *m_ThreadedConnection = nullptr;
	CConnectionPool *m_ConnectionPool = nullptr;

public: //functions
	bool Execute(ExecutionType type, Query_t query);
	bool GetErrorId(unsigned int &errorid);
	bool EscapeString(const string &src, string &dest);
	bool SetCharacterSet(string charset);
	bool GetCharacterSet(string &charset);
	bool GetStatus(string &stat);
};

class CHandleManager : public CSingleton<CHandleManager>
{
	friend class CSingleton<CHandleManager>;
private: //constructor / deconstructor
	CHandleManager() = default;
	~CHandleManager() = default;

private: //variables
	unordered_set<Handle_t> m_Handles;

public: //functions
	Handle_t Create(string host, string user, string pass, string db, 
		const COptions *options, CError<CHandle> &error);
	Handle_t CreateFromFile(string file_path, CError<CHandle> &error);
	bool Destroy(Handle_t &handle);

	inline bool IsValidHandle(const Handle_t &handle)
	{
		return m_Handles.find(handle) != m_Handles.end();
	}
	inline Handle_t GetHandle(cell handle_cell)
	{
		CHandle *handle = reinterpret_cast<CHandle *>(handle_cell);
		return IsValidHandle(handle) ? handle : nullptr;
	}

};
