#pragma once

#include <samplog/PluginLogger.hpp>
#include "CSingleton.hpp"
#include "CError.hpp"

#include <format.h>
#include <atomic>

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
	void Log(const LOGLEVEL &level, const std::string &fmt, Args &&...args)
	{
		m_Logger.load(std::memory_order_acquire)
			->Log(level, fmt::format(fmt, std::forward<Args>(args)...));
	}
	template<typename... Args>
	void Log(AMX * const amx, const LOGLEVEL &level, const std::string &fmt, Args &&...args)
	{
		m_Logger.load(std::memory_order_acquire)
			->Log(amx, level, fmt::format(fmt, std::forward<Args>(args)...));
	}
	
	inline bool LogNativeCall(AMX * const amx, const std::string &name, const std::string &params_format)
	{
		return m_Logger.load(std::memory_order_acquire)
			->LogNativeCall(amx, name, params_format);
	}

	template<typename T>
	inline void LogError(AMX * const amx, const std::string &func, const CError<T> &error)
	{
		Log(amx, LOGLEVEL::ERROR, "{}: {} error: {}",
			func.c_str(), error.module().c_str(), error.msg().c_str());
	}
	inline void LogError(AMX * const amx, const std::string &func, const std::string &msg)
	{
		Log(amx, LOGLEVEL::ERROR, "{}: {}", func.c_str(), msg.c_str());
	}
	template<typename... Args>
	inline void LogError(AMX * const amx, const std::string &func, 
		const std::string &format, Args &&...args)
	{
		LogError(amx, func, fmt::format(format, std::forward<Args>(args)...));
	}

	inline void LogWarning(AMX * const amx, const std::string &func, const std::string &msg)
	{
		Log(amx, LOGLEVEL::WARNING, "{}: {}", func.c_str(), msg.c_str());
	}
	template<typename... Args>
	inline void LogError(AMX * const amx, const std::string &func,
		const std::string &format, Args &&...args)
	{
		LogError(amx, func, fmt::format(format, std::forward<Args>(args)...));
	}

private:
	std::atomic<PluginLogger_t> m_Logger;
};
