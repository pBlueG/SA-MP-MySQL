#pragma once

#include "Api.hpp"

#include <string>


namespace samplog
{
	class PluginLogger
	{
	public:
		explicit PluginLogger(std::string pluginname) :
			_logger(Api::Get()->CreateLogger(pluginname.insert(0, "plugins/").c_str()))
		{ }
		~PluginLogger() = default;
		PluginLogger(PluginLogger const &rhs) = delete;
		PluginLogger& operator=(PluginLogger const &rhs) = delete;
		PluginLogger(PluginLogger &&other) = delete;
		PluginLogger& operator=(PluginLogger &&other) = delete;

	private:
		Logger_t _logger;

	public:
		inline bool IsLogLevel(LogLevel log_level)
		{
			return _logger->IsLogLevel(log_level);
		}

		inline bool Log(LogLevel level, const char *msg)
		{
			return _logger->Log(level, msg);
		}

		inline bool Log(LogLevel level, const char *msg,
			std::vector<AmxFuncCallInfo> const &call_info)
		{
			return _logger->Log(level, msg, call_info);
		}

		inline bool Log(AMX * const amx, const LogLevel level, const char *msg)
		{
			std::vector<AmxFuncCallInfo> call_info;
			return Api::Get()->GetAmxFunctionCallTrace(amx, call_info)
				&& _logger->Log(level, msg, call_info);
		}

		inline bool LogNativeCall(AMX * const amx, cell * const params,
			const char *name, const char *params_format)
		{
			return _logger->LogNativeCall(amx, params, name, params_format);
		}

		inline bool operator()(LogLevel level, const char *msg)
		{
			return Log(level, msg);
		}

		inline bool operator()(AMX * const amx, const LogLevel level, const char *msg)
		{
			return Log(amx, level, msg);
		}

		inline bool operator()(AMX * const amx, cell * const params,
			const char *name, const char *params_format)
		{
			return LogNativeCall(amx, params, name, params_format);
		}
	};

	typedef PluginLogger PluginLogger_t;

}
