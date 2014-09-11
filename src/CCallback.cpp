#include "CCallback.h"


//TODO: add boost::system::error reference for error handling
CCallback *CCallbackManager::Create(AMX *amx, string name, string format, cell *params, cell param_offset)
{
	if (amx == nullptr || params == nullptr || name.empty())
		return nullptr;

	int cb_idx = -1;
	if (amx_FindPublic(amx, name.c_str(), &cb_idx) != AMX_ERR_NONE)
		return nullptr;


	CCallback::ParamList_t param_list;
	if (format.empty() == false)
	{
		if (static_cast<cell>(params[0] / sizeof(cell)) < param_offset)
			return nullptr;


		cell param_idx = 0;
		cell *address_ptr = nullptr;

		for (auto &c : format)
		{
			switch (c)
			{
			case 'd':
			case 'i':
			case 'f':
			case 'b':
				amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
				param_list.push(*address_ptr);
				break;
			case 's':
				param_list.push(amx_GetCppString(amx, params[param_offset + param_idx]));
				break;
			case 'a':
				//TODO: support for arrays
				break;
			default:
				//"Error while creating callback structure: Unrecognized format specifier '%c'", c);
				return nullptr;
			}
			param_idx++;
		}
	}

	return new CCallback(amx, cb_idx, std::move(param_list));
}


bool CCallback::Execute()
{
	if (CCallbackManager::Get()->IsValidAmx(m_AmxInstance) == false)
		return false;


	cell amx_address = -1;
	while (m_Params.empty() == false)
	{
		auto &param = m_Params.top();
		if (param.type() == typeid(cell))
		{
			amx_Push(m_AmxInstance, boost::get<cell>(param));
		}
		else
		{
			cell tmp_addr;
			amx_PushString(m_AmxInstance, &tmp_addr, nullptr,
				boost::get<string>(param).c_str(), 0, 0);

			if (amx_address < NULL)
				amx_address = tmp_addr;
		}
		m_Params.pop();
	}

	if (m_PreExecute)
		m_PreExecute();

	amx_Exec(m_AmxInstance, nullptr, m_AmxCallbackIndex);
	if (amx_address >= NULL)
		amx_Release(m_AmxInstance, amx_address);

	if (m_PostExecute)
		m_PostExecute();

	return true;
}


void CCallbackManager::Process()
{
	while (m_Callbacks.empty() == false)
	{
		CCallback *callback = m_Callbacks.front();
		m_Callbacks.pop();

		callback->Execute();

		delete callback;
	}
}
