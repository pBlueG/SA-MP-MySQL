#pragma once

#include <samplog/PluginLogger.h>
#include "CSingleton.hpp"
#include "CError.hpp"

#include <fmt/format.h>

using samplog::PluginLogger_t;
using samplog::LogLevel;


struct DebugInfo
{
	int line = 0;
	const char
		*function = nullptr,
		*file = nullptr;
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
	inline const char *GetCurrentNativeName()
	{
		return m_NativeName;
	}

private:
	void Update(AMX * const amx, const char *func);
	void Clear();

private:
	bool m_Available = false;

	AMX *m_Amx = nullptr;
	DebugInfo m_Info;
	const char *m_NativeName = nullptr;
};


class CLog : public CSingleton<CLog>
{
	friend class CSingleton<CLog>;
	friend class CScopedDebugInfo;
private:
	CLog() :
		m_Logger("mysql")
	{ }
	~CLog() = default;

public:
	inline void SetLogLevel(LogLevel level)
	{
		m_Logger.SetLogLevel(level);
	}
	inline bool IsLogLevel(LogLevel level)
	{
		return m_Logger.IsLogLevel(level);
	}

	template<typename... Args>
	inline void Log(LogLevel level, const char *format, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		m_Logger.Log(level,
					 fmt::format(format, std::forward<Args>(args)...).c_str());
	}

	template<typename... Args>
	inline void Log(LogLevel level, const DebugInfo &dbginfo,
					const char *format, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		m_Logger.CLogger::Log(level,
							  fmt::format(format, std::forward<Args>(args)...).c_str(),
							  dbginfo.line, dbginfo.file, dbginfo.function);
	}

	// should only be called in native functions
	template<typename... Args>
	void LogNative(LogLevel level, const char *fmt, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		if (CDebugInfoManager::Get()->GetCurrentAmx() == nullptr)
			return; //do nothing, since we're not called from within a native func

		Log(level,
			CDebugInfoManager::Get()->GetCurrentInfo(),
			fmt::format("{}: {}",
						CDebugInfoManager::Get()->GetCurrentNativeName(),
						fmt::format(fmt, std::forward<Args>(args)...)).c_str());
	}

	template<typename T>
	inline void LogNative(const CError<T> &error)
	{
		LogNative(LogLevel::ERROR, "{} error: {}",
				  error.module(), error.msg());
	}

private:
	PluginLogger_t m_Logger;

};


class CScopedDebugInfo
{
private:
	bool m_HasAmxDebugSymbols = false;
public:
	CScopedDebugInfo(AMX * const amx, const char *func, 
					 const char *params_format = "");
	~CScopedDebugInfo()
	{
		if (m_HasAmxDebugSymbols)
			CDebugInfoManager::Get()->Clear();
	}
	CScopedDebugInfo(const CScopedDebugInfo &rhs) = delete;
};
