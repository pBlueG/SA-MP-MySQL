#include "CCallback.h"

#include <array>


namespace std
{
	std::error_condition make_error_condition(CCallback::Errors e)
	{
		static const CCallback::error_category category;
		return std::error_condition(static_cast<int>(e), category);
	}
}

CCallback *CCallbackManager::Create(AMX *amx, string name, string format, cell *params, cell param_offset,
	error_condition &error)
{
	if (amx == nullptr)
	{
		error = std::make_error_condition(CCallback::Errors::INVALID_AMX);
		return nullptr;
	}

	if (params == nullptr)
	{
		error = std::make_error_condition(CCallback::Errors::EMPTY_PARAMETERS);
		return nullptr;
	}

	if (name.empty())
	{
		error = std::make_error_condition(CCallback::Errors::EMPTY_NAME);
		return nullptr;
	}


	int cb_idx = -1;
	if (amx_FindPublic(amx, name.c_str(), &cb_idx) != AMX_ERR_NONE)
	{
		error = std::make_error_condition(CCallback::Errors::NOT_FOUND);
		return nullptr;
	}


	CCallback::ParamList_t param_list;
	if (format.empty() == false)
	{
		if (static_cast<cell>(params[0] / sizeof(cell)) < param_offset)
		{
			error = std::make_error_condition(CCallback::Errors::INVALID_PARAM_OFFSET);
			return nullptr;
		}


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
				error = std::make_error_condition(CCallback::Errors::INVALID_FORMAT_SPECIFIER);
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

string CCallback::error_category::message(int ev) const
{
	static const std::array<std::string, static_cast<size_t>(Errors::_NUM_ERRORS)> error_messages{
		"No name specified",
		"No parameters specified",
		"Invalid AMX",
		"Not found",
		"Invalid parameter offset",
		"Invalid format specifier"
	};

	if (ev < 0 || ev >= static_cast<int>(error_messages.size()))
		return "Unknown error";

	return error_messages.at(ev);
}
