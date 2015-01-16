#include "CCallback.hpp"

#include <map>


Callback_t CCallback::Create(
	AMX *amx, string name, string format, cell *params, cell param_offset, CCallback::Error &error)
{
	error = CCallback::Error::NONE;
	if (amx == nullptr)
	{
		error = CCallback::Error::INVALID_AMX;
		return nullptr;
	}

	if (params == nullptr)
	{
		error = CCallback::Error::INVALID_PARAMETERS;
		return nullptr;
	}

	if (name.empty())
	{
		error = CCallback::Error::EMPTY_NAME;
		return nullptr;
	}


	int cb_idx = -1;
	if (amx_FindPublic(amx, name.c_str(), &cb_idx) != AMX_ERR_NONE)
	{
		error = CCallback::Error::NOT_FOUND;
		return nullptr;
	}


	CCallback::ParamList_t param_list;
	if (format.empty() == false)
	{
		if (static_cast<cell>(params[0] / sizeof(cell)) < param_offset)
		{
			error = CCallback::Error::INVALID_PARAM_OFFSET;
			return nullptr;
		}


		cell param_idx = 0;
		cell *address_ptr = nullptr;
		cell *array_addr_ptr = nullptr;

		for (auto c = format.begin(); c != format.end(); ++c)
		{
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
						//TODO: error: invalid array size
						return nullptr;
					}
					cell *copied_array = static_cast<cell *>(malloc(value * sizeof(cell)));
					memcpy(copied_array, array_addr_ptr, value * sizeof(cell));

					param_list.push(std::make_tuple('a', std::make_tuple(copied_array, value)));
					array_addr_ptr = nullptr;
				}
				param_list.push(std::make_tuple('c', value));
			}	break;
			case 'f': //float
			case 'b': //bool
				if (array_addr_ptr != nullptr)
				{
					//TODO: error: expected 'd'/'i' specifier for array size
					return nullptr;
				}
				amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
				param_list.push(std::make_tuple('c', *address_ptr));
				break;
			case 's': //string
				if (array_addr_ptr != nullptr)
				{
					//TODO: error: expected 'd'/'i' specifier for array size
					return nullptr;
				}
				param_list.push(std::make_tuple('s', amx_GetCppString(amx, params[param_offset + param_idx])));
				break;
			case 'a': //array
				if (array_addr_ptr != nullptr)
				{
					//TODO: error: expected 'd'/'i' specifier for array size
					return nullptr;
				}
				amx_GetAddr(amx, params[param_offset + param_idx], &array_addr_ptr);
				break;
			case 'r': //reference
				if (array_addr_ptr != nullptr)
				{
					//TODO: error: expected 'd'/'i' specifier for array size
					return nullptr;
				}
				amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
				param_list.push(std::make_tuple('r', address_ptr));
				break;
			default:
				error = CCallback::Error::INVALID_FORMAT_SPECIFIER;
				return nullptr;
			}
			param_idx++;
		}

		if (array_addr_ptr != nullptr)
		{
			//TODO: error: no length specified after 'a' specifier
			return nullptr;
		}
	}

	return std::make_shared<CCallback>(amx, cb_idx, std::move(param_list));
}


bool CCallback::Execute()
{
	if (CCallbackManager::Get()->IsValidAmx(m_AmxInstance) == false)
		return false;

	if (m_Executed == true)
		return false; //can't execute the same callback more than once because m_Params is emptied


	cell amx_address = -1;
	while (m_Params.empty() == false)
	{
		cell tmp_addr;
		auto &param = m_Params.top();
		boost::any &param_val = std::get<1>(param);
		switch (std::get<0>(param))
		{
			case 'c': //cell
				amx_Push(m_AmxInstance, boost::any_cast<cell>(param_val));
				break;
			case 's': //string
				amx_PushString(m_AmxInstance, &tmp_addr, nullptr,
					boost::any_cast<string>(param_val).c_str(), 0, 0);

				if (amx_address < NULL)
					amx_address = tmp_addr;
				break;
			case 'a': //array
			{
				auto array_tuple = boost::any_cast<tuple<cell *, cell>>(param_val);
				cell *array_addr = std::get<0>(array_tuple);
				cell array_size = std::get<1>(array_tuple);
				amx_PushArray(m_AmxInstance, &tmp_addr, nullptr, array_addr, array_size);
				free(array_addr);

				if (amx_address < NULL)
					amx_address = tmp_addr;
			} break;
			case 'r': //reference
				amx_PushAddress(m_AmxInstance, boost::any_cast<cell *>(param_val));
				break;
		}
		m_Params.pop();
	}

	amx_Exec(m_AmxInstance, nullptr, m_AmxCallbackIndex);
	if (amx_address >= NULL)
		amx_Release(m_AmxInstance, amx_address);

	m_Executed = true;
	return true;
}

bool CCallbackManager::GetErrorString(CCallback::Error error, string &dest)
{
	static const std::map<CCallback::Error, string> error_list{
			{ CCallback::Error::INVALID_AMX, "Invalid AMX" },
			{ CCallback::Error::INVALID_PARAMETERS, "Invalid parameters" },
			{ CCallback::Error::INVALID_PARAM_OFFSET, "Parameter count does not match format specifier length" },
			{ CCallback::Error::INVALID_FORMAT_SPECIFIER, "Invalid format specifier" },
			{ CCallback::Error::EMPTY_NAME, "Empty name specified" },
			{ CCallback::Error::NOT_FOUND, "Callback does not exist" }
	};

	auto error_it = error_list.find(error);
	if (error_it != error_list.end())
	{
		dest = error_it->second;
		return true;
	}
	return false;
}
