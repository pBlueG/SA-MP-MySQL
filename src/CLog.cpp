#include "CLog.hpp"

#include <samplog/DebugInfo.hpp>


CLog::CLog()
	: m_Logger(samplog::CreatePluginLogger("mysql"))
{ }

void CDebugInfoManager::Update(AMX * const amx, std::string &&func)
{
	m_Amx = amx;
	m_NativeName.assign(std::move(func));

	char
		cfile[256],
		cfunc[64];
	m_Available =
		samplog::GetLastAmxFile(amx, cfile) &&
		samplog::GetLastAmxLine(amx, m_Info.line) &&
		samplog::GetLastAmxFunction(amx, cfunc);

	m_Info.file.assign(cfile);
	m_Info.function.assign(cfunc);
}

void CDebugInfoManager::Clear()
{
	m_Amx = nullptr;
	m_NativeName.clear();
	m_Info.line = 0;
	m_Info.function.clear();
	m_Info.file.clear();
}
