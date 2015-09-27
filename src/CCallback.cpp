#include "CCallback.hpp"
#include "CLog.hpp"

#include <map>


const string CCallback::ModuleName{ "callback" };


Callback_t CCallback::Create(AMX *amx, string name, string format, 
	cell *params, cell param_offset, CError<CCallback> &error)
{
	CLog::Get()->Log(LOGLEVEL::DEBUG,
		"CCallback::Create(amx={}, name='{}', format='{}, params={}, param_offset={})",
		static_cast<const void *>(amx), name, format, static_cast<const void *>(params), param_offset);

	CLog::Get()->Log(LOGLEVEL::INFO, "Setting up callback '{}' for delayed execution...", name);
	
	if (amx == nullptr)
	{
		error.set(Error::INVALID_AMX, "invalid AMX");
		return nullptr;
	}

	if (params == nullptr)
	{
		error.set(Error::INVALID_PARAMETERS, "invalid parameters");
		return nullptr;
	}

	if (name.empty())
	{
		error.set(Error::EMPTY_NAME, "empty name specified");
		return nullptr;
	}

	int cb_idx = -1;
	if (amx_FindPublic(amx, name.c_str(), &cb_idx) != AMX_ERR_NONE)
	{
		error.set(Error::NOT_FOUND, "callback \"{}\" does not exist", name);
		return nullptr;
	}

	CLog::Get()->Log(LOGLEVEL::DEBUG, "CCallback::Create - callback index for '{}': {}",
		name, cb_idx);


	ParamList_t param_list(format.length());
	if (format.empty() == false)
	{
		if (static_cast<cell>(params[0] / sizeof(cell)) < param_offset)
		{
			error.set(Error::INVALID_PARAM_OFFSET, 
				"parameter count does not match format specifier length");
			return nullptr;
		}


		cell param_idx = 0;
		cell *address_ptr = nullptr;
		cell *array_addr_ptr = nullptr;

		for (auto c = format.begin(); c != format.end(); ++c)
		{
			if (array_addr_ptr != nullptr && (*c) != 'd' && (*c) != 'i')
			{
				error.set(Error::EXPECTED_ARRAY_SIZE, 
					"expected 'd'/'i' specifier for array size (got '{}' instead)", *c);
				return nullptr;
			}

			switch (*c)
			{
			case 'd': //decimal
			case 'i': //integer
			{
				amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
				cell value = *address_ptr;
				if (array_addr_ptr != nullptr)
				{
					if (value <= 0)
					{
						error.set(Error::INVALID_ARRAY_SIZE, "invalid array size '{}'", value);
						return nullptr;
					}
					cell *copied_array = static_cast<cell *>(malloc(value * sizeof(cell)));
					memcpy(copied_array, array_addr_ptr, value * sizeof(cell));

					param_list.push_front(std::make_tuple('a', std::make_tuple(copied_array, value)));
					array_addr_ptr = nullptr;
				}
				param_list.push_front(std::make_tuple('c', value));
			}	break;
			case 'f': //float
			case 'b': //bool
				amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
				param_list.push_front(std::make_tuple('c', *address_ptr));
				break;
			case 's': //string
				param_list.push_front(std::make_tuple('s', amx_GetCppString(amx, params[param_offset + param_idx])));
				break;
			case 'a': //array
				amx_GetAddr(amx, params[param_offset + param_idx], &array_addr_ptr);
				break;
			case 'r': //reference
				amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
				param_list.push_front(std::make_tuple('r', address_ptr));
				break;
			default:
				error.set(Error::INVALID_FORMAT_SPECIFIER, "invalid format specifier '{}'", *c);
				return nullptr;
			}
			param_idx++;
		}

		if (array_addr_ptr != nullptr)
		{
			error.set(Error::NO_ARRAY_SIZE, "no array size specified after 'a' specifier");
			return nullptr;
		}
	}

	CLog::Get()->Log(LOGLEVEL::INFO, "Callback successfully set up.");

	return std::make_shared<CCallback>(amx, cb_idx, std::move(param_list));
}


bool CCallback::Execute()
{
	CLog::Get()->Log(LOGLEVEL::DEBUG, "CCallback::Execute(amx={}, index={}, num_params={})",
		static_cast<const void *>(m_AmxInstance), m_AmxCallbackIndex, m_Params.size());

	//the user could unload a filterscript between CCallback creation and
	//execution, so we better check if the AMX instance is still valid
	if (CCallbackManager::Get()->IsValidAmx(m_AmxInstance) == false)
	{
		CLog::Get()->Log(LOGLEVEL::ERROR, "CCallback::Execute - invalid AMX instance");
		return false;
	}

	char callback_name[sNAMEMAX + 1];
	if (amx_GetPublic(m_AmxInstance, m_AmxCallbackIndex, callback_name) == AMX_ERR_NONE)
	{
		CLog::Get()->Log(LOGLEVEL::INFO, "Executing callback '{}' with {} parameter{}...",
			callback_name, m_Params.size(), m_Params.size() > 1 ? "s" : ""); 
	}


	cell amx_address = -1;
	for(auto &i : m_Params)
	{
		cell tmp_addr;
		boost::any &param_val = std::get<1>(i);
		switch (std::get<0>(i))
		{
			case 'c': //cell
				amx_Push(m_AmxInstance, boost::any_cast<cell>(param_val));
				break;
			case 's': //string
				amx_PushString(m_AmxInstance, &tmp_addr, nullptr,
					boost::any_cast<string>(param_val).c_str(), 0, 0);

				if (amx_address < 0)
					amx_address = tmp_addr;
				break;
			case 'a': //array
			{
				auto array_tuple = boost::any_cast<tuple<cell *, cell>>(param_val);
				cell *array_addr = std::get<0>(array_tuple);
				cell array_size = std::get<1>(array_tuple);
				amx_PushArray(m_AmxInstance, &tmp_addr, nullptr, array_addr, array_size);
				free(array_addr);

				if (amx_address < 0)
					amx_address = tmp_addr;
			} break;
			case 'r': //reference
				amx_PushAddress(m_AmxInstance, boost::any_cast<cell *>(param_val));
				break;
		}
	}

	amx_Exec(m_AmxInstance, nullptr, m_AmxCallbackIndex);
	if (amx_address >= 0)
		amx_Release(m_AmxInstance, amx_address);

	CLog::Get()->Log(LOGLEVEL::INFO, "Callback successfully executed.");

	return true;
}
