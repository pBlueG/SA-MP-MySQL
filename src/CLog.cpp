#include "CLog.hpp"

#include <amx/amx.h>


void CDebugInfoManager::Update(AMX * const amx, const char *func)
{
	m_Amx = amx;
	m_NativeName = func;

	m_Available =
		samplog::GetLastAmxLine(amx, m_Info.line) &&
		samplog::GetLastAmxFile(amx, m_Info.file) &&
		samplog::GetLastAmxFunction(amx, m_Info.function);
}

void CDebugInfoManager::Clear()
{
	m_Amx = nullptr;
	m_NativeName = nullptr;
	m_Available = false;
	m_Info.line = 0;
	m_Info.function = nullptr;
	m_Info.file = nullptr;
}

CScopedDebugInfo::CScopedDebugInfo(AMX * const amx, const char *func,
								   const char *params_format /* = ""*/)
{
	uint16_t amx_flags = 0;
	amx_Flags(amx, &amx_flags);
	m_HasAmxDebugSymbols = (amx_flags & AMX_FLAG_DEBUG) == AMX_FLAG_DEBUG;

	if (m_HasAmxDebugSymbols)
		CDebugInfoManager::Get()->Update(amx, func);

	auto &logger = CLog::Get()->m_Logger;
	if (logger.IsLogLevel(LogLevel::DEBUG))
		logger.LogNativeCall(amx, func, params_format);
}
