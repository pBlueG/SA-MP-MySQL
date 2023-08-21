#pragma once

#include "LogLevel.hpp"

#include <cstdint>
#include <string>
#include <vector>


typedef struct tagAMX AMX;
typedef int32_t cell;

namespace samplog
{
	struct AmxFuncCallInfo
	{
		int line;
		const char *file;
		const char *function;
	};

	class ILogger
	{
	public:
		virtual bool IsLogLevel(LogLevel log_level) const = 0;

		virtual bool LogNativeCall(AMX * const amx, cell * const params,
			std::string name, std::string params_format) = 0;

		virtual bool Log(LogLevel level, std::string msg,
			std::vector<AmxFuncCallInfo> const &call_info) = 0;

		virtual bool Log(LogLevel level, std::string msg) = 0;

		virtual void Destroy() = 0;
		virtual ~ILogger() = default;
	};
}
