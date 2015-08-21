#pragma once

#include <samplog/PluginLogger.hpp>
#include "CSingleton.hpp"

using samplog::PluginLogger_t;
using samplog::LOGLEVEL;

class CLog : public CSingleton<CLog>
{
	friend class CSingleton<CLog>;
private:
	CLog();
	~CLog() = default;

public:
	template<typename... Args>
	void LogFormat(const LOGLEVEL &level, const std::string &fmt, Args ...args)
	{
		char dest[2048];
		sprintf(dest, fmt.c_str(), args...);
		m_Logger->Log(level, dest);
	}
	template<typename... Args>
	void LogFormat(AMX * const amx, const LOGLEVEL &level, const std::string &fmt, Args ...args)
	{
		char dest[2048];
		sprintf(dest, fmt.c_str(), args...);
		m_Logger->Log(amx, level, dest);
	}
	
	inline bool LogNativeCall(AMX * const amx, const std::string &name, const std::string &params_format)
	{
		return m_Logger->LogNativeCall(amx, name, params_format);
	}


private:
	PluginLogger_t m_Logger;
};
