#pragma once

#include "CSingleton.hpp"

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

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
	CHandle(HandleId_t id, size_t myhash) :
		m_Id(id),
		m_MyHash(myhash)
	{ }
	~CHandle();

private: //variables
	const HandleId_t m_Id;
	const size_t m_MyHash;

	CConnection *m_MainConnection = nullptr;
	CThreadedConnection *m_ThreadedConnection = nullptr;
	CConnectionPool *m_ConnectionPool = nullptr;

public: //functions
	inline HandleId_t GetId() const
	{
		return m_Id;
	}

	bool Execute(ExecutionType type, Query_t query);
	bool GetErrorId(unsigned int &errorid);
	bool EscapeString(const char *src, string &dest);
	bool SetCharacterSet(string charset);
	bool GetCharacterSet(string &charset);
	bool GetStatus(string &stat);
	unsigned int GetUnprocessedQueryCount();
};

class CHandleManager : public CSingleton<CHandleManager>
{
	friend class CSingleton<CHandleManager>;
private: //constructor / deconstructor
	CHandleManager() = default;
	~CHandleManager() = default;

private: //variables
	unordered_map<HandleId_t, Handle_t> m_Handles;

public: //functions
	Handle_t Create(const char *host, const char *user, const char *pass, const char *db,
		const COptions *options, CError<CHandle> &error);
	Handle_t CreateFromFile(string file_path, CError<CHandle> &error);
	bool Destroy(Handle_t &handle);

	inline bool IsValidHandle(const HandleId_t id)
	{
		return m_Handles.find(id) != m_Handles.end();
	}
	inline Handle_t GetHandle(const HandleId_t id)
	{
		return IsValidHandle(id) ? m_Handles.at(id) : nullptr;
	}

};
