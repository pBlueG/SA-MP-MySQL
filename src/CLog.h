#pragma once
#ifndef INC_CLOG_H
#define INC_CLOG_H


#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>

using boost::atomic;
using boost::thread;
namespace this_thread = boost::this_thread;

enum e_LogLevel 
{
	LOG_NONE = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_DEBUG = 4
};

enum e_LogType 
{
	LOG_TYPE_TEXT = 1,
	LOG_TYPE_HTML = 2
};

enum e_LogInfo
{
	LOG_INFO_NONE,
	LOG_INFO_CALLBACK_BEGIN,
	LOG_INFO_CALLBACK_END,
	LOG_INFO_THREADED
};


class CLog 
{
public:
	static inline CLog *Get() 
	{
		if(m_Instance == NULL)
			m_Instance = new CLog;
		return m_Instance;
	}
	static inline void Destroy() 
	{
		delete m_Instance;
	}


	void Initialize(const char *logfile);
	int LogFunction(unsigned loglevel, char *funcname, char *msg, ...);
	int LogText(unsigned int loglevel, char* text);
	void StartCallback(const char *cbname);
	void EndCallback();

	void SetLogLevel(unsigned int loglevel) 
	{
		m_LogLevel = loglevel;
	}
	bool IsLogLevel(unsigned int loglevel) 
	{
		return !!(m_LogLevel & loglevel);
	}
	void SetLogType(unsigned int logtype);
	
private:
	static CLog *m_Instance;
	
	struct m_SLogData 
	{
		m_SLogData() :
			Status(LOG_NONE),
			Name(NULL), Msg(NULL),
			Info(LOG_INFO_NONE)
		{}

		unsigned int Status;
		char *Name, *Msg;
		
		unsigned int Info;

		~m_SLogData() 
		{
			free(Name);
			free(Msg);
		}
	};


	CLog() : 
		m_LogLevel(LOG_ERROR | LOG_WARNING), 
		m_LogThread(NULL), 
		m_LogThreadAlive(true),
		m_LogType(LOG_TYPE_TEXT)
	{}
	~CLog();

	void ProcessLog();

	
	char m_LogFileName[32];
	unsigned int m_LogType;
	unsigned int m_LogLevel;

	thread *m_LogThread;
	atomic<bool> m_LogThreadAlive;
	thread::id m_MainThreadID;

	boost::lockfree::queue<
			m_SLogData*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<32678>
		> m_LogQueue;
};


#endif // INC_CLOG_H
