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

/*boost::lockfree::queue<
		CMySQLQuery*, 
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<8192>
	> CCallback::m_CallbackQueue;*/
list<std::tuple<boost::unique_future<CMySQLQuery>, CMySQLHandle*>> CCallback::m_CallbackQueue;

list<AMX *> CCallback::m_AmxList;


void CCallback::ProcessCallbacks() 
{
	if (!m_CallbackQueue.empty())
	{
		auto i = m_CallbackQueue.begin();
		do
		{
			//auto FutureRes = boost::move(std::get<0>((*i)));
			auto &FutureRes = std::get<0>((*i));
			if (FutureRes.is_ready())
			{
				CMySQLQuery QueryObj = boost::move(FutureRes.get());
				
				for (list<AMX *>::iterator a = m_AmxList.begin(), end = m_AmxList.end(); a != end; ++a)
				{
					AMX *amx = (*a);
					cell amx_Ret;
					int amx_Index;
					cell amx_MemoryAddress = -1;

					if (amx_FindPublic(amx, QueryObj.Callback.Name.c_str(), &amx_Index) == AMX_ERR_NONE)
					{
						CLog::Get()->StartCallback(QueryObj.Callback.Name.c_str());

						int StringIndex = QueryObj.Callback.Format.length() - 1;
						while (!QueryObj.Callback.Params.empty() && StringIndex >= 0)
						{
							switch (QueryObj.Callback.Format.at(StringIndex))
							{
								case 'i':
								case 'd':
								{
									int val = 0;
									ConvertStrToInt(QueryObj.Callback.Params.top().c_str(), val);

									/*if (PassByReference == false)
										amx_Push(amx, (cell)val);
									else*/
									{
										cell tmpAddress;
										amx_PushArray(amx, &tmpAddress, NULL, (cell*)&val, 1);
										if (amx_MemoryAddress < NULL)
											amx_MemoryAddress = tmpAddress;
									}
								}
								break;
								case 'f':
								{
									float float_val = 0.0f;
									ConvertStrToFloat(QueryObj.Callback.Params.top().c_str(), float_val);
									cell FParam = amx_ftoc(float_val);

									/*if (PassByReference == false)
										amx_Push(amx, FParam);
									else*/
									{
										cell tmpAddress;
										amx_PushArray(amx, &tmpAddress, NULL, (cell*)&FParam, 1);
										if (amx_MemoryAddress < NULL)
											amx_MemoryAddress = tmpAddress;
									}
								}
								break;
								default:
								{
									cell tmpAddress;
									amx_PushString(amx, &tmpAddress, NULL, QueryObj.Callback.Params.top().c_str(), 0, 0);
									if (amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
								}
							}

							StringIndex--;
							QueryObj.Callback.Params.pop();
						}

						CMySQLHandle *ConnHandle = std::get<1>(*i);
						ConnHandle->SetActiveResult(boost::move(QueryObj.Result));
						//Query->Result = NULL;
						CMySQLHandle::ActiveHandle = ConnHandle;

						amx_Exec(amx, &amx_Ret, amx_Index);
						if (amx_MemoryAddress >= NULL)
							amx_Release(amx, amx_MemoryAddress);

						CMySQLHandle::ActiveHandle = nullptr;

						//if (ConnHandle->IsActiveResultSaved() == false)
							//ConnHandle->GetActiveResult()->Destroy();

						//Query->ConnHandle->SetActiveResult((CMySQLResult *)NULL);
						ConnHandle->SetQueryConnectionFree(QueryObj.Connection);

						CLog::Get()->EndCallback();

						break; //we have found our callback, exit loop
					}
				}


				i = m_CallbackQueue.erase(i);
			}
			else
			{
				//std::get<0>(*i) = boost::move(FutureRes);
			}
		} 
		while (!m_CallbackQueue.empty() && i != m_CallbackQueue.end() && ++i != m_CallbackQueue.end());
	}
}



void CCallback::AddAmx( AMX *amx ) 
{
	m_AmxList.push_back(amx);
}

void CCallback::EraseAmx( AMX *amx ) 
{
	for (list<AMX *>::iterator a = m_AmxList.begin(); a != m_AmxList.end(); ++a) 
	{
		if ( (*a) == amx) 
		{
			m_AmxList.erase(a);
			break;
		}
	}
}

void CCallback::ClearAll() {
	//CMySQLQuery *tmpQuery = NULL;
	//while(m_CallbackQueue.pop(tmpQuery))
	//	tmpQuery->Destroy();
}

void CCallback::FillCallbackParams(stack<string> &dest, string &format, AMX* amx, cell* params, const int ConstParamCount) 
{
	unsigned int ParamIdx = 1;
	cell *AddressPtr;

	for(string::iterator c = format.begin(), end = format.end(); c != end; ++c) 
	{
		if ( (*c) == 'd' || (*c) == 'i') 
		{
			amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
			char IntBuf[12]; //12 -> strlen of (-2^31) + '\0'
			ConvertIntToStr<10>((*AddressPtr), IntBuf);
			dest.push(IntBuf);
		} 
		else if ( (*c) == 's' || (*c) == 'z') 
		{
			char *StrBuf = NULL;
			amx_StrParam(amx, params[ConstParamCount + ParamIdx], StrBuf);
			dest.push(StrBuf == NULL ? string() : StrBuf);
		} 
		else if ( (*c) == 'f') 
		{
			amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
			char FloatBuf[84]; //84 -> strlen of (2^(2^7)) + '\0'
			ConvertFloatToStr(amx_ctof(*AddressPtr), FloatBuf);
			dest.push(FloatBuf);
		} 
		else 
			dest.push("NULL");

		ParamIdx++;
	}
}
