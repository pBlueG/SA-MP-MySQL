#include "CLog.hpp"


CLog::CLog()
	: m_Logger(samplog::CreatePluginLogger("mysql"))
{ }
