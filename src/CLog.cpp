#include "CLog.hpp"

#include <amx/amx.h>


void CDebugInfoManager::Update(AMX * const amx, const char *func)
{
	m_Amx = amx;
	m_NativeName = func;
	m_Info.clear();
	m_Available = samplog::GetAmxFunctionCallTrace(amx, m_Info);
}

void CDebugInfoManager::Clear()
{
	m_Amx = nullptr;
	m_NativeName = nullptr;
	m_Available = false;
}

CScopedDebugInfo::CScopedDebugInfo(AMX * const amx, const char *func,
	cell * const params, const char *params_format /* = ""*/)
{
	CDebugInfoManager::Get()->Update(amx, func);

	auto &logger = CLog::Get()->m_Logger;
	if (logger.IsLogLevel(LogLevel::DEBUG))
		logger.LogNativeCall(amx, params, func, params_format);
}

void CLog::SetLogLevel(LogLevel level)
{
	auto get_log_string = [this](string &dest)
	{
		dest.clear();
		if (m_Logger.IsLogLevel(LogLevel::DEBUG))
			dest += "debug, ";

		if (m_Logger.IsLogLevel(LogLevel::INFO))
			dest += "info, ";

		if (m_Logger.IsLogLevel(LogLevel::WARNING))
			dest += "warning, ";

		if (m_Logger.IsLogLevel(LogLevel::ERROR))
			dest += "error, ";

		if (dest.empty())
			dest = "none";
		else
			dest.erase(dest.length() - 2);
	};

	string old_loglevel_str;
	get_log_string(old_loglevel_str);

	m_Logger.SetLogLevel(level);

	string new_loglevel_str;
	get_log_string(new_loglevel_str);

	m_Logger.Log(LogLevel::INFO, fmt::format("changed log level from '{}' to '{}'",
		old_loglevel_str, new_loglevel_str).c_str());
}
