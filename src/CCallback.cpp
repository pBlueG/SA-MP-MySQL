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

boost::lockfree::queue<
		CMySQLQuery*, 
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<10000>
	> CCallback::m_CallbackQueue;

list<AMX *> CCallback::m_AmxList;


void CCallback::ProcessCallbacks() 
{
	CMySQLQuery *Query = NULL;
	while( (Query = GetNextQuery()) != NULL) 
	{
		CCallback *Callback = Query->Callback;
		 
		if(Callback != NULL && (Callback->Name.length() > 0 || Query->OrmObject != NULL) ) 
		{
			bool PassByReference = Query->Callback->IsInline;

			if(Query->OrmObject != NULL) //orm, update the variables with the given result
			{
				switch(Query->OrmQueryType) 
				{
					case ORM_QUERYTYPE_SELECT:
						Query->OrmObject->ApplySelectResult(Query->Result);
						break;

					case ORM_QUERYTYPE_INSERT:
						Query->OrmObject->ApplyInsertResult(Query->Result);
						break;
				}
			}

			for (list<AMX *>::iterator a = m_AmxList.begin(), end = m_AmxList.end(); a != end; ++a) 
			{
				AMX *amx = (*a);
				cell amx_Ret;
				int amx_Index;
				cell amx_MemoryAddress = -1;

				if (amx_FindPublic(amx, Callback->Name.c_str(), &amx_Index) == AMX_ERR_NONE) 
				{
					CLog::Get()->StartCallback(Callback->Name.c_str());

					int StringIndex = Callback->ParamFormat.length()-1; 
					while(!Callback->Parameters.empty() && StringIndex >= 0) 
					{
						switch(Callback->ParamFormat.at(StringIndex)) 
						{
							case 'i':
							case 'd': 
							{
								int val = 0;
								ConvertStrToInt(Callback->Parameters.top().c_str(), val);

								if(PassByReference == false)
									amx_Push(amx, (cell)val);
								else 
								{
									cell tmpAddress;
									amx_PushArray(amx, &tmpAddress, NULL, (cell*)&val, 1);
									if(amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
								}
							} 
							break;
							case 'f': 
							{
								float float_val = 0.0f;
								ConvertStrToFloat(Callback->Parameters.top().c_str(), float_val);
								cell FParam = amx_ftoc(float_val);
								
								if(PassByReference == false)
									amx_Push(amx, FParam);
								else 
								{
									cell tmpAddress;
									amx_PushArray(amx, &tmpAddress, NULL, (cell*)&FParam, 1);
									if(amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
								}
							} 
							break;
							default: 
							{
								cell tmpAddress;
								amx_PushString(amx, &tmpAddress, NULL, Callback->Parameters.top().c_str(), 0, 0);
								if(amx_MemoryAddress < NULL)
									amx_MemoryAddress = tmpAddress;
							}
						}

						StringIndex--;
						Callback->Parameters.pop();
					}


					Query->ConnHandle->SetActiveResult(Query->Result);
					Query->Result = NULL;

					amx_Exec(amx, &amx_Ret, amx_Index);
					if (amx_MemoryAddress >= NULL)
						amx_Release(amx, amx_MemoryAddress);

					if(Query->ConnHandle->IsActiveResultSaved() == false)
						delete Query->ConnHandle->GetActiveResult();

					Query->ConnHandle->SetActiveResult((CMySQLResult *)NULL);

					CLog::Get()->EndCallback();
					
					break; //we have found our callback, exit loop
				}
			}
		}
		Query->Destroy();
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
		if (( *a) == amx) 
		{
			m_AmxList.erase(a);
			break;
		}
	}
}

void CCallback::ClearAll() {
	CMySQLQuery *tmpQuery = NULL;
	while(m_CallbackQueue.pop(tmpQuery))
		tmpQuery->Destroy();
}

void CCallback::FillCallbackParams(AMX* amx, cell* params, const int ConstParamCount) 
{
	unsigned int ParamIdx = 1;
	cell *AddressPtr;

	for(string::iterator c = ParamFormat.begin(), end = ParamFormat.end(); c != end; ++c) 
	{
		if ( (*c) == 'd' || (*c) == 'i') 
		{
			amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
			char IntBuf[12]; //12 -> strlen of (-2^31) + '\0'
			ConvertIntToStr<10>((*AddressPtr), IntBuf);
			Parameters.push(IntBuf);
		} 
		else if ( (*c) == 's' || (*c) == 'z') 
		{
			char *StrBuf = NULL;
			amx_StrParam(amx, params[ConstParamCount + ParamIdx], StrBuf);
			Parameters.push(StrBuf == NULL ? string() : StrBuf);
		} 
		else if ( (*c) == 'f') 
		{
			amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
			char FloatBuf[84]; //84 -> strlen of (2^(2^7)) + '\0'
			ConvertFloatToStr(amx_ctof(*AddressPtr), FloatBuf);
			Parameters.push(FloatBuf);
		} 
		else 
			Parameters.push("NULL");

		ParamIdx++;
	}
}
