#include "CLog.hpp"

#include <amx/amx.h>


void CDebugInfoManager::Update(AMX * const amx, const char *func)
{
	m_Amx = amx;
	m_NativeName = func;
	m_Info.clear();
	m_Available = samplog::Api::Get()->GetAmxFunctionCallTrace(amx, m_Info);
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
