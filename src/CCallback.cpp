#include "CCallback.hpp"
#include "CLog.hpp"

#include <map>


const string CCallback::ModuleName{ "callback" };


Callback_t CCallback::Create(AMX *amx, const char *name, const char *format,
							 cell *params, cell param_offset, 
							 CError<CCallback> &error)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CCallback::Create(amx={}, name='{}', format='{}', params={}, param_offset={})",
					 static_cast<const void *>(amx),
					 name ? name : "(nullptr)",
					 format ? format : "(nullptr)",
					 static_cast<const void *>(params), param_offset);

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

	if (name == nullptr || strlen(name) == 0)
	{
		error.set(Error::EMPTY_NAME, "empty name specified");
		return nullptr;
	}

	int cb_idx = -1;
	if (amx_FindPublic(amx, name, &cb_idx) != AMX_ERR_NONE)
	{
		error.set(Error::NOT_FOUND, "callback \"{}\" does not exist", name);
		return nullptr;
	}

	CLog::Get()->Log(LogLevel::DEBUG, 
					 "CCallback::Create - callback index for '{}': {}",
					 name, cb_idx);


	const size_t num_params = (format == nullptr) ? 0 : strlen(format);
	if ((params[0] / sizeof(cell) - (param_offset - 1)) != num_params)
	{
		error.set(Error::INVALID_PARAM_COUNT,
				  "parameter count does not match format specifier length");
		return nullptr;
	}

	ParamList_t param_list;
	if (num_params != 0)
	{
		cell param_idx = 0;
		cell *address_ptr = nullptr;
		cell *array_addr_ptr = nullptr;

		do
		{
			if (array_addr_ptr != nullptr && (*format) != 'd' && (*format) != 'i')
			{
				error.set(Error::EXPECTED_ARRAY_SIZE,
						  "expected 'd'/'i' specifier for array size (got '{}' instead)", 
						  *format);
				return nullptr;
			}

			CLog::Get()->Log(LogLevel::DEBUG,
							 "processing specifier '{}' with parameter index {}",
							 *format, param_idx);

			switch (*format)
			{
				case 'd': //decimal
				case 'i': //integer
				{
					amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
					cell value = *address_ptr;
					if (array_addr_ptr != nullptr)
					{
						CLog::Get()->Log(LogLevel::DEBUG, "expecting array size");
						if (value <= 0)
						{
							error.set(Error::INVALID_ARRAY_SIZE, 
									  "invalid array size '{}'", value);
							return nullptr;
						}
						cell *copied_array = 
							static_cast<cell *>(malloc(value * sizeof(cell)));
						memcpy(copied_array, array_addr_ptr, value * sizeof(cell));

						param_list.push_front(
							std::make_tuple(
								'a', std::make_tuple(
									copied_array, value)));
						CLog::Get()->Log(LogLevel::DEBUG, "pushed array");
						array_addr_ptr = nullptr;
					}

					param_list.push_front(std::make_tuple('c', *address_ptr));
					CLog::Get()->Log(LogLevel::DEBUG, 
									 "retrieved and pushed value '{}'", value);
				}
				break;
				case 'f': //float
				case 'b': //bool
					amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
					param_list.push_front(std::make_tuple('c', *address_ptr));
					CLog::Get()->Log(LogLevel::DEBUG, 
									 "retrieved and pushed value '{}'", *address_ptr);
					break;
				case 's': //string
				{
					const char *str = nullptr;
					amx_StrParam(amx, params[param_offset + param_idx], str);
					param_list.push_front(std::make_tuple('s', string(str ? str : "")));
					CLog::Get()->Log(LogLevel::DEBUG, "retrieved and pushed value '{}'", str ? str : "");
				}
				break;
				case 'a': //array
					amx_GetAddr(amx, params[param_offset + param_idx], &array_addr_ptr);
					CLog::Get()->Log(LogLevel::DEBUG, "retrieved array '{}'",
									 static_cast<const void*>(array_addr_ptr));
					break;
				case 'r': //reference
					amx_GetAddr(amx, params[param_offset + param_idx], &address_ptr);
					param_list.push_front(std::make_tuple('r', address_ptr));
					CLog::Get()->Log(LogLevel::DEBUG, "retrieved and pushed reference '{}'",
									 static_cast<const void*>(array_addr_ptr));
					break;
				default:
					error.set(Error::INVALID_FORMAT_SPECIFIER, 
							  "invalid format specifier '{}'", *format);
					return nullptr;
			}
			param_idx++;
		}
		while (*(++format) != '\0');

		if (array_addr_ptr != nullptr)
		{
			error.set(Error::NO_ARRAY_SIZE, 
					  "no array size specified after 'a' specifier");
			return nullptr;
		}
	}

	CLog::Get()->Log(LogLevel::INFO, 
					 "Callback '{}' set up for delayed execution.", name);
	CLog::Get()->Log(LogLevel::DEBUG, 
					 "created delayed callback with {} parameter{}",
					 param_list.size(), param_list.size() > 1 ? "s" : "");
	return std::make_shared<CCallback>(amx, cb_idx, std::move(param_list));
}

Callback_t CCallback::Create(CError<CCallback> &error, AMX *amx, 
							 const char *name, const char *format, ...)
{
	CLog::Get()->Log(LogLevel::DEBUG,
					 "CCallback::Create(amx={}, name='{}', format='{})",
					 static_cast<const void *>(amx),
					 name ? name : "(nullptr)",
					 format ? format : "(nullptr)");

	if (amx == nullptr)
	{
		error.set(Error::INVALID_AMX, "invalid AMX");
		return nullptr;
	}

	if (name == nullptr || strlen(name) == 0)
	{
		error.set(Error::EMPTY_NAME, "empty name specified");
		return nullptr;
	}

	int cb_idx = -1;
	if (amx_FindPublic(amx, name, &cb_idx) != AMX_ERR_NONE)
	{
		error.set(Error::NOT_FOUND, "callback \"{}\" does not exist", name);
		return nullptr;
	}

	CLog::Get()->Log(LogLevel::DEBUG, 
					 "CCallback::Create - callback index for '{}': {}",
					 name, cb_idx);


	size_t num_params = (format == nullptr) ? 0 : strlen(format);
	ParamList_t param_list;
	if (num_params != 0)
	{
		va_list args;
		va_start(args, format);

		for (; *format != '\0'; ++format)
		{
			CLog::Get()->Log(LogLevel::DEBUG, 
							 "processing specifier '{}'", *format);

			switch (*format)
			{
				case 'd': //decimal
				case 'i': //integer
				case 'b': //bool
				{
					cell value = va_arg(args, cell);
					param_list.push_front(std::make_tuple('c', value));
					CLog::Get()->Log(LogLevel::DEBUG, 
									 "retrieved and pushed value '{}'", value);
				}
				break;
				case 'f': //float
				{
					float float_value = static_cast<float>(va_arg(args, double));
					param_list.push_front(std::make_tuple('c', amx_ftoc(float_value)));
					CLog::Get()->Log(LogLevel::DEBUG, 
									 "retrieved and pushed value '{}'", float_value);
				}
				break;
				case 's': //string
				{
					const char *value = va_arg(args, const char*);
					param_list.push_front(std::make_tuple('s', string(value)));
					CLog::Get()->Log(LogLevel::DEBUG, 
									 "retrieved and pushed value '{}'", value);
				}
				break;
				default:
					error.set(Error::INVALID_FORMAT_SPECIFIER, 
							  "invalid format specifier '{}'", *format);
					return nullptr;
			}
		}

		va_end(args);
	}

	if (param_list.size() != num_params)
	{
		error.set(Error::INVALID_PARAM_COUNT,
				  "parameter count does not match format specifier length");
		return nullptr;
	}

	CLog::Get()->Log(LogLevel::INFO, 
					 "Callback '{}' set up for delayed execution.", name);
	CLog::Get()->Log(LogLevel::DEBUG, 
					 "created delayed callback with {} parameter{}",
					 param_list.size(), param_list.size() > 1 ? "s" : "");

	return std::make_shared<CCallback>(amx, cb_idx, std::move(param_list));
}


bool CCallback::Execute()
{
	CLog::Get()->Log(LogLevel::DEBUG, 
					 "CCallback::Execute(amx={}, index={}, num_params={})",
					 static_cast<const void *>(m_AmxInstance),
					 m_AmxCallbackIndex, m_Params.size());

	//the user could unload a filterscript between CCallback creation and
	//execution, so we better check if the AMX instance is still valid
	if (CCallbackManager::Get()->IsValidAmx(m_AmxInstance) == false)
	{
		CLog::Get()->Log(LogLevel::ERROR, 
						 "CCallback::Execute - invalid AMX instance");
		return false;
	}

	if (CLog::Get()->IsLogLevel(LogLevel::INFO))
	{
		char callback_name[sNAMEMAX + 1];
		if (amx_GetPublic(m_AmxInstance, m_AmxCallbackIndex, callback_name) == AMX_ERR_NONE)
		{
			CLog::Get()->Log(LogLevel::INFO, 
							 "Executing callback '{}' with {} parameter{}...",
							 callback_name, m_Params.size(), m_Params.size() > 1 ? "s" : "");
		}
	}

	cell amx_address = -1;
	for (auto &i : m_Params)
	{
		cell tmp_addr;
		boost::any &param_val = std::get<1>(i);
		char specifier = std::get<0>(i);

		CLog::Get()->Log(LogLevel::DEBUG, 
						 "processing internal specifier '{}'", specifier);
		switch (specifier)
		{
			case 'c': //cell
			{
				const cell value = boost::any_cast<cell>(param_val);
				amx_Push(m_AmxInstance, value);

				CLog::Get()->Log(LogLevel::DEBUG, 
								 "pushed value '{}' onto AMX stack", value);
			}
			break;
			case 's': //string
			{
				const string value = boost::any_cast<string>(param_val);
				amx_PushString(m_AmxInstance, &tmp_addr, nullptr,
							   value.c_str(), 0, 0);

				CLog::Get()->Log(LogLevel::DEBUG, 
								 "pushed value '{}' onto AMX stack", value);

				if (amx_address < 0)
					amx_address = tmp_addr;
			}
			break;
			case 'a': //array
			{
				auto array_tuple = boost::any_cast<tuple<cell *, cell>>(param_val);
				cell *array_addr = std::get<0>(array_tuple);
				cell array_size = std::get<1>(array_tuple);
				amx_PushArray(m_AmxInstance, &tmp_addr, nullptr, array_addr, array_size);
				free(array_addr);

				CLog::Get()->Log(LogLevel::DEBUG, 
								 "pushed array '{}' with size '{}' onto AMX stack",
								 static_cast<const void*>(array_addr), array_size);

				if (amx_address < 0)
					amx_address = tmp_addr;
			}
			break;
			case 'r': //reference
			{
				cell * const value = boost::any_cast<cell *>(param_val);
				amx_PushAddress(m_AmxInstance, value);

				CLog::Get()->Log(LogLevel::DEBUG, 
								 "pushed reference '{}' onto AMX stack",
								 static_cast<const void*>(value));
			}
			break;
		}
	}

	CLog::Get()->Log(LogLevel::DEBUG, 
					 "executing AMX callback with index '{}'", 
					 m_AmxCallbackIndex);

	int error = amx_Exec(m_AmxInstance, nullptr, m_AmxCallbackIndex);

	CLog::Get()->Log(LogLevel::DEBUG, 
					 "AMX callback executed with error '{}'", 
					 error);

	if (amx_address >= 0)
		amx_Release(m_AmxInstance, amx_address);

	CLog::Get()->Log(LogLevel::INFO, "Callback successfully executed.");

	return true;
}
