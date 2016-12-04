#pragma once

#include <samplog/PluginLogger.h>
#include "CSingleton.hpp"
#include "CError.hpp"

#include <fmt/format.h>

using samplog::PluginLogger_t;
using samplog::LogLevel;
using samplog::AmxFuncCallInfo;


class CDebugInfoManager : public CSingleton<CDebugInfoManager>
{
	friend class CSingleton<CDebugInfoManager>;
	friend class CScopedDebugInfo;
private:
	CDebugInfoManager() = default;
	~CDebugInfoManager() = default;

private:
	bool m_Available = false;

	AMX *m_Amx = nullptr;
	std::vector<AmxFuncCallInfo> m_Info;
	const char *m_NativeName = nullptr;

private:
	void Update(AMX * const amx, const char *func);
	void Clear();

public:
	inline AMX * const GetCurrentAmx()
	{
		return m_Amx;
	}
	inline const decltype(m_Info) &GetCurrentInfo()
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
	void SetLogLevel(LogLevel level);
	inline bool IsLogLevel(LogLevel level)
	{
		return m_Logger.IsLogLevel(level);
	}

	template<typename... Args>
	inline void Log(LogLevel level, const char *format, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		string str = format;
		if (sizeof...(args) != 0)
			str = fmt::format(format, std::forward<Args>(args)...);

		m_Logger.Log(level, str.c_str());
	}

	template<typename... Args>
	inline void Log(LogLevel level, std::vector<AmxFuncCallInfo> const &callinfo,
					const char *format, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		string str = format;
		if (sizeof...(args) != 0)
			str = fmt::format(format, std::forward<Args>(args)...);

		m_Logger.CLogger::Log(level, str.c_str(), callinfo);
	}

	// should only be called in native functions
	template<typename... Args>
	void LogNative(LogLevel level, const char *fmt, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		if (CDebugInfoManager::Get()->GetCurrentAmx() == nullptr)
			return; //do nothing, since we're not called from within a native func

		string msg = fmt::format("{}: {}",
			CDebugInfoManager::Get()->GetCurrentNativeName(),
			fmt::format(fmt, std::forward<Args>(args)...));

		if (CDebugInfoManager::Get()->IsInfoAvailable())
			Log(level, CDebugInfoManager::Get()->GetCurrentInfo(), msg.c_str());
		else
			Log(level, msg.c_str());
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
public:
	CScopedDebugInfo(AMX * const amx, const char *func, 
					 const char *params_format = "");
	~CScopedDebugInfo()
	{
		CDebugInfoManager::Get()->Clear();
	}
	CScopedDebugInfo(const CScopedDebugInfo &rhs) = delete;
};
