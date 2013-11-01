#pragma once

#include "main.h"
#include "CCallback.h"
#include "CMySQLHandle.h"
#include "CMySQLQuery.h"
#include "CMySQLResult.h"
//#include "COrm.h"
#include "CLog.h"

#include "misc.h"
#include <cstdio>

#include <chrono>


list<tuple<future<CMySQLQuery>, CMySQLHandle*>> CCallback::m_CallbackQueue;
mutex CCallback::m_QueueMtx;

list<AMX *> CCallback::m_AmxList;



void CCallback::ProcessCallbacks() 
{
	if (!m_CallbackQueue.empty())
	{
		std::lock_guard<mutex> LockGuard(m_QueueMtx);
		auto i = m_CallbackQueue.begin();
		do
		{
			auto &FutureRes = std::get<0>((*i));

			if (FutureRes.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				CMySQLQuery QueryObj = std::move(FutureRes.get());
				bool PassByRef = (QueryObj.Callback.Name.find("FJ37DH3JG") != string::npos);
				
				for (auto &amx : m_AmxList)
				{
					cell amx_Ret;
					int amx_Index;
					cell amx_MemoryAddress = -1;

					if (amx_FindPublic(amx, QueryObj.Callback.Name.c_str(), &amx_Index) == AMX_ERR_NONE)
					{
						CLog::Get()->StartCallback(QueryObj.Callback.Name.c_str());

						while (!QueryObj.Callback.Params.empty())
						{
							boost::variant<cell, string> value = std::move(QueryObj.Callback.Params.top());
							if (value.type() == typeid(cell))
							{
								if (PassByRef)
								{
									cell tmpAddress;
									amx_PushArray(amx, &tmpAddress, NULL, (cell*)&boost::get<cell>(value), 1);
									if (amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
								}
								else
									amx_Push(amx, boost::get<cell>(value));
							}
							else
							{
								cell tmpAddress;
								amx_PushString(amx, &tmpAddress, NULL, boost::get<string>(value).c_str(), 0, 0);
								if (amx_MemoryAddress < NULL)
									amx_MemoryAddress = tmpAddress;
							}

							QueryObj.Callback.Params.pop();
						}

						CMySQLHandle *ConnHandle = std::get<1>(*i);
						ConnHandle->DecreaseQueryCounter();
						ConnHandle->SetActiveResult(QueryObj.Result);
						//QueryObj.Result = nullptr;
						CMySQLHandle::ActiveHandle = ConnHandle;

						amx_Exec(amx, &amx_Ret, amx_Index);
						if (amx_MemoryAddress >= NULL)
							amx_Release(amx, amx_MemoryAddress);

						CMySQLHandle::ActiveHandle = nullptr;

						if (ConnHandle->IsActiveResultSaved() == false)
							delete ConnHandle->GetActiveResult();

						ConnHandle->SetActiveResult(nullptr);

						CLog::Get()->EndCallback();

						break; //we have found our callback, exit loop
					}
				}


				i = m_CallbackQueue.erase(i);
			}
			else
			{
				//if(mysql_options.execute-in-order == true)
					//return ;
			}
			
		} 
		while (!m_CallbackQueue.empty() && i != m_CallbackQueue.end() && ++i != m_CallbackQueue.end());
	}
}



void CCallback::AddAmx(AMX *amx) 
{
	m_AmxList.push_back(amx);
}

void CCallback::EraseAmx(AMX *amx) 
{
	for (auto a = m_AmxList.begin(), end = m_AmxList.end(); a != end; ++a)
	{
		if ( (*a) == amx) 
		{
			m_AmxList.erase(a);
			break;
		}
	}
}

void CCallback::ClearAll() 
{
	m_CallbackQueue.clear();
}

void CCallback::FillCallbackParams(stack<boost::variant<cell, string>> &dest, const char *format, AMX* amx, cell* params, const int ConstParamCount)
{
	if (format == nullptr || !(*format))
		return ;

	unsigned int ParamIdx = 1;
	cell *AddressPtr = nullptr;

	do
	{
		char *StrBuf = nullptr;
		switch (*format)
		{
			case 'd':
			case 'i':
			case 'f':
				amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
				dest.push(*AddressPtr);
				break;
			case 'z':
			case 's':
				amx_StrParam(amx, params[ConstParamCount + ParamIdx], StrBuf);
				dest.push(StrBuf == nullptr ? string() : string(StrBuf));
				break;
			default:
				dest.push(string("NULL"));
		}
		ParamIdx++;
	} while (*(++format));
}
