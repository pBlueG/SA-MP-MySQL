#pragma once

#include "main.h"
#include "CCallback.h"
#include "CMySQLHandle.h"
#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "COrm.h"
#include "CLog.h"

#include "misc.h"
#include <cstdio>


list< tuple<shared_future<CMySQLQuery>, CMySQLHandle*> > CCallback::m_CallbackQueue;
mutex CCallback::m_QueueMtx;

list<AMX *> CCallback::m_AmxList;



void CCallback::ProcessCallbacks() 
{
	if (!m_CallbackQueue.empty())
	{
		boost::mutex::scoped_lock LockGuard(m_QueueMtx);
		list< tuple<shared_future<CMySQLQuery>, CMySQLHandle*> >::iterator i = m_CallbackQueue.begin();
		do
		{
			shared_future<CMySQLQuery> &future_res = boost::get<0>((*i));

			if (future_res.has_value())
			{
				CMySQLQuery QueryObj = boost::move(future_res.get());
				CMySQLHandle *Handle = boost::get<1>(*i);

				Handle->DecreaseQueryCounter();


				if (QueryObj.Orm.Object != NULL)
				{
					switch (QueryObj.Orm.Type)
					{
					case ORM_QUERYTYPE_SELECT:
						QueryObj.Orm.Object->ApplySelectResult(QueryObj.Result);
						break;

					case ORM_QUERYTYPE_INSERT:
						QueryObj.Orm.Object->ApplyInsertResult(QueryObj.Result);
						break;
					}
				}

				if (!QueryObj.Callback.Name.empty())
				{

					bool pass_by_ref = (QueryObj.Callback.Name.find("FJ37DH3JG") != string::npos);
					for (list<AMX *>::iterator a = m_AmxList.begin(), end = m_AmxList.end(); a != end; ++a)
					{
						AMX *amx = (*a);
						int amx_index;

						if (amx_FindPublic(amx, QueryObj.Callback.Name.c_str(), &amx_index) == AMX_ERR_NONE)
						{
							cell amx_mem_addr = -1;
							CLog::Get()->StartCallback(QueryObj.Callback.Name.c_str());

							while (!QueryObj.Callback.Params.empty())
							{
								boost::variant<cell, string> value = boost::move(QueryObj.Callback.Params.top());
								if (value.type() == typeid(cell))
								{
									if (pass_by_ref)
									{
										cell tmp_addr;
										amx_PushArray(amx, &tmp_addr, NULL, (cell*)&boost::get<cell>(value), 1);
										if (amx_mem_addr < NULL)
											amx_mem_addr = tmp_addr;
									}
									else
										amx_Push(amx, boost::get<cell>(value));
								}
								else
								{
									cell tmp_addr;
									amx_PushString(amx, &tmp_addr, NULL, boost::get<string>(value).c_str(), 0, 0);
									if (amx_mem_addr < NULL)
										amx_mem_addr = tmp_addr;
								}

								QueryObj.Callback.Params.pop();
							}

							Handle->SetActiveResult(QueryObj.Result);
							CMySQLHandle::ActiveHandle = Handle;

							cell amx_ret;
							amx_Exec(amx, &amx_ret, amx_index);
							if (amx_mem_addr >= NULL)
								amx_Release(amx, amx_mem_addr);

							CMySQLHandle::ActiveHandle = NULL;

							if (Handle->IsActiveResultSaved() == false)
								delete Handle->GetActiveResult();

							Handle->SetActiveResult((CMySQLResult*)NULL);

							CLog::Get()->EndCallback();

							break; //we have found our callback, exit loop
						}
					}
				}

				i = m_CallbackQueue.erase(i);
			}
			else
				return ;
			
		} while (!m_CallbackQueue.empty() && i != m_CallbackQueue.end() && ++i != m_CallbackQueue.end());
	}
}



void CCallback::AddAmx(AMX *amx) 
{
	m_AmxList.push_back(amx);
}

void CCallback::EraseAmx(AMX *amx) 
{
	for (list<AMX *>::iterator a = m_AmxList.begin(), end = m_AmxList.end(); a != end; ++a)
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

void CCallback::FillCallbackParams(stack< boost::variant<cell, string> > &dest, const char *format, AMX* amx, cell* params, const int ConstParamCount)
{
	if (format == NULL || !(*format))
		return ;

	unsigned int ParamIdx = 1;
	cell *AddressPtr = NULL;

	do
	{
		char *StrBuf = NULL;
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
				dest.push(StrBuf == NULL ? string() : string(StrBuf));
				break;
			default:
				dest.push(string("NULL"));
		}
		ParamIdx++;
	} while (*(++format));
}
