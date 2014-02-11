#include "main.h"
#include "CCallback.h"
#include "CMySQLHandle.h"
#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "COrm.h"
#include "CLog.h"
#include "misc.h"

#include <queue>


CCallback *CCallback::m_Instance = new CCallback;


void CCallback::ProcessCallbacks() 
{
	CMySQLQuery *query = NULL;
	while(m_CallbackQueue.pop(query))
	{
		if (query->Orm.Object != NULL)
		{
			switch (query->Orm.Type)
			{
			case ORM_QUERYTYPE_SELECT:
				query->Orm.Object->ApplySelectResult(query->Result);
				break;

			case ORM_QUERYTYPE_INSERT:
				query->Orm.Object->ApplyInsertResult(query->Result);
				break;
			}
		}

		if (!query->Callback.Name.empty())
		{
			const bool pass_by_ref = (query->Callback.Name.find("FJ37DH3JG") != string::npos);
			for (set<AMX *>::iterator a = m_AmxList.begin(), end = m_AmxList.end(); a != end; ++a)
			{
				AMX *amx = (*a);
				int amx_index;

				if (amx_FindPublic(amx, query->Callback.Name.c_str(), &amx_index) == AMX_ERR_NONE)
				{
					cell amx_mem_addr = -1;
					CLog::Get()->StartCallback(query->Callback.Name.c_str());

					while (!query->Callback.Params.empty())
					{
						boost::variant<cell, string> value = boost::move(query->Callback.Params.top());
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

						query->Callback.Params.pop();
					}

					query->Handle->SetActiveResult(query->Result);
					query->Result = NULL;

					cell amx_ret;
					amx_Exec(amx, &amx_ret, amx_index);
					if (amx_mem_addr >= NULL)
						amx_Release(amx, amx_mem_addr);

					if (query->Handle->IsActiveResultSaved() == false)
						delete query->Handle->GetActiveResult();

					query->Handle->SetActiveResult(static_cast<CMySQLResult *>(NULL));

					CLog::Get()->EndCallback();

					break; //we have found our callback, exit loop
				}
			}
		}

		delete query;
		query = NULL;
	}
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

void CCallback::ClearByHandle(CMySQLHandle *handle)
{
	std::queue<CMySQLQuery *> tmp_queue;
	CMySQLQuery *query = NULL;
	while(m_CallbackQueue.pop(query))
	{
		if(query->Handle != handle)
			tmp_queue.push(query);
		else
		{
			delete query->Result;
			delete query;
		}
	}

	while(!tmp_queue.empty())
	{
		m_CallbackQueue.push(tmp_queue.front());
		tmp_queue.pop();
	}
}
