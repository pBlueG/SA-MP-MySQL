#pragma once

#include "CSingleton.h"


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


class CLog : public CSingleton<CLog>
{
public:
	void Initialize(const char *logfile);
	int LogFunction(unsigned loglevel, char *funcname, char *msg, ...);
	int LogText(unsigned int loglevel, char* text);
	void StartCallback(const char *cbname);
	void EndCallback();

	void SetLogLevel(unsigned int loglevel) 
	{
		
	}
	bool IsLogLevel(unsigned int loglevel) 
	{
		return false;
	}
	void SetLogType(unsigned int logtype);
	
private:
	static CLog *m_Instance;
	
	struct m_SLogData 
	{
		
	};


	CLog() = default;
	~CLog() = default;

	void ProcessLog();
};
