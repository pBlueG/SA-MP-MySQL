#pragma once

#include "ILogger.hpp"
#include "export.h"

#include <vector>
#include <memory>
#include <functional>


namespace samplog
{
	namespace internal
	{
		static const int API_VERSION = 1;
		class IApi
		{
		public:
			virtual void RegisterAmx(AMX *amx) = 0;
			virtual void EraseAmx(AMX *amx) = 0;

			virtual bool GetLastAmxFunctionCall(AMX * const amx,
				AmxFuncCallInfo &destination) = 0;
			virtual bool GetAmxFunctionCallTrace(AMX * const amx,
				std::vector<AmxFuncCallInfo> &dest) = 0;

			virtual samplog::ILogger *CreateLogger(const char *module) = 0;

			virtual ~IApi() { }
		};
	}
}

extern "C" DLL_PUBLIC samplog::internal::IApi *samplog_GetApi(int version);
extern "C" DLL_PUBLIC void samplog_DestroyApi(samplog::internal::IApi *api);

#undef DLL_PUBLIC

namespace samplog
{
	using Logger_t = std::unique_ptr<ILogger, std::function<void(ILogger*)>>;

	class Api
	{
	private: // singleton
		enum SingletonAction
		{
			RETURN_LAZY,
			DESTROY
		};

		static Api *SingletonDo(SingletonAction action)
		{
			static Api *api = nullptr;
			switch (action)
			{
			case RETURN_LAZY:
				if (api == nullptr)
					api = new Api;
				break;
			case DESTROY:
				if (api != nullptr)
				{
					delete api;
					api = nullptr;
				}
				break;
			}
			return api;
		}

	public: // singleton
		static Api *Get()
		{
			return SingletonDo(RETURN_LAZY);
		}
		static void Destroy()
		{
			SingletonDo(DESTROY);
		}

	private: // api
		internal::IApi *_api;

	public: // api
		Api() : _api(samplog_GetApi(internal::API_VERSION))
		{ }
		~Api()
		{
			samplog_DestroyApi(_api);
		}
		Api(Api&&) = delete;
		Api& operator=(Api&&) = delete;
		Api(const Api&) = delete;
		Api& operator=(const Api&) = delete;


		inline void RegisterAmx(AMX *amx)
		{
			_api->RegisterAmx(amx);
		}
		inline void EraseAmx(AMX *amx)
		{
			_api->EraseAmx(amx);
		}

		inline bool GetLastAmxFunctionCall(AMX * const amx,
			AmxFuncCallInfo &destination)
		{
			return _api->GetLastAmxFunctionCall(amx, destination);
		}
		inline bool GetAmxFunctionCallTrace(AMX * const amx,
			std::vector<AmxFuncCallInfo> &dest)
		{
			return _api->GetAmxFunctionCallTrace(amx, dest);
		}

		inline Logger_t CreateLogger(const char *module_name)
		{
			return Logger_t(_api->CreateLogger(module_name), 
				std::mem_fn(&ILogger::Destroy));
		}
	};
}
