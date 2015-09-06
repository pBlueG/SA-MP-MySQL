#pragma once

#include <samplog/PluginLogger.hpp>
#include "CSingleton.hpp"
#include "CError.hpp"

#include <format.h>
#include <atomic>

using samplog::PluginLogger_t;
using samplog::LOGLEVEL;


struct DebugInfo
{
	long line = 0;
	std::string
		function,
		file;
};

class CDebugInfoManager : public CSingleton<CDebugInfoManager>
{
	friend class CSingleton<CDebugInfoManager>;
	friend class CScopedDebugInfo;
private:
	CDebugInfoManager() = default;
	~CDebugInfoManager() = default;

public:
	inline AMX * const GetCurrentAmx()
	{
		return m_Amx;
	}
	inline const DebugInfo &GetCurrentInfo()
	{
		return m_Info;
	}
	inline bool IsInfoAvailable()
	{
		return m_Available;
	}
	inline std::string &GetCurrentNativeName()
	{
		return m_NativeName;
	}

private:
	void Update(AMX * const amx, std::string &&func);
	void Clear();

private:
	bool m_Available = false;

	AMX *m_Amx = nullptr;
	DebugInfo m_Info;
	std::string m_NativeName;
};


class CLog : public CSingleton<CLog>
{
	friend class CSingleton<CLog>;
	friend class CScopedDebugInfo;
private:
	CLog();
	~CLog() = default;

public:
	template<typename... Args>
	inline void Log(const LOGLEVEL &level, const std::string &format, Args &&...args)
	{
		m_Logger->Log(level, fmt::format(format, std::forward<Args>(args)...));
	}

	// should ONLY be called in AMX thread!
	template<typename... Args>
	void LogNative(const LOGLEVEL &level, const std::string &fmt, Args &&...args)
	{
		string msg = fmt::format("{}: {}", 
			CDebugInfoManager::Get()->GetCurrentNativeName(), 
			fmt::format(fmt, std::forward<Args>(args)...));
		if (CDebugInfoManager::Get()->IsInfoAvailable())
		{
			const DebugInfo &info = CDebugInfoManager::Get()->GetCurrentInfo();
			m_Logger->LogEx(level, msg, info.line, info.file, info.function);
		}
		else
		{
			m_Logger->Log(level, msg);
		}
	}

	template<typename T>
	inline void LogNative(const CError<T> &error)
	{
		LogNative(LOGLEVEL::ERROR, "{} error: {}", error.module(), error.msg());
	}

private:
	PluginLogger_t m_Logger;

};


class CScopedDebugInfo
{
public:
	CScopedDebugInfo(AMX * const amx, std::string &&func, std::string params_format = std::string())
	{
		CLog::Get()->m_Logger->LogNativeCall(amx, func, params_format);
		CDebugInfoManager::Get()->Update(amx, std::move(func));
	}
	~CScopedDebugInfo()
	{
		CDebugInfoManager::Get()->Clear();
	}
	CScopedDebugInfo(const CScopedDebugInfo &rhs) = delete;
};
