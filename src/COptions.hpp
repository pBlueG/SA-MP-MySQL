#pragma once

#include "CSingleton.hpp"

#include <string>
#include <map>
#include <unordered_map>
#include <boost/variant.hpp>

using std::string;
using std::map;
using std::unordered_map;

#include "types.hpp"


class COptions
{
	friend class COptionManager;
public:
	enum class Type
	{
		AUTO_RECONNECT, //true
		MULTI_STATEMENTS, //false
		POOL_SIZE, //2
		SERVER_PORT, //3306
		SSL_ENABLE, //false
		SSL_KEY_FILE, //nullptr
		SSL_CERT_FILE, //nullptr
		SSL_CA_FILE, //nullptr
		SSL_CA_PATH, //nullptr
		SSL_CIPHER, //nullptr
	};

private:
	COptions();
	~COptions() = default;
	
private:
	map<Type, boost::variant<bool, unsigned int, string>> m_Options;

private: //helper function
	template <typename T>
	bool IsValidOptionType(Type option)
	{
		static const map<Type, size_t> type_map
		{
			{ Type::AUTO_RECONNECT, typeid(bool).hash_code() },
			{ Type::MULTI_STATEMENTS, typeid(bool).hash_code() },
			{ Type::POOL_SIZE, typeid(unsigned int).hash_code() },
			{ Type::SERVER_PORT, typeid(unsigned int).hash_code() },
			{ Type::SSL_ENABLE, typeid(bool).hash_code() },
			{ Type::SSL_KEY_FILE, typeid(string).hash_code() },
			{ Type::SSL_CERT_FILE, typeid(string).hash_code() },
			{ Type::SSL_CA_FILE, typeid(string).hash_code() },
			{ Type::SSL_CA_PATH, typeid(string).hash_code() },
			{ Type::SSL_CIPHER, typeid(string).hash_code() }
		};

		return type_map.at(option) == typeid(T).hash_code();
	}

public:
	template<typename T>
	inline bool SetOption(Type option, T value)
	{
		if (IsValidOptionType<T>(option))
		{
			m_Options[option] = value;
			return true;
		}
		return false;
	}
	template<typename T>
	inline T GetOption(Type option) const
	{
		return boost::get<T>(m_Options.at(option));
	}
	
};

class COptionManager : public CSingleton<COptionManager>
{
	friend class CSingleton<COptionManager>;
public:
	enum class GlobalOption
	{
		DUPLICATE_CONNECTIONS, //false
		DUPLICATE_CONNECTION_WARNING, //true
	};

private:
	COptionManager();
	~COptionManager();

private:
	unordered_map<OptionsId_t, COptions *> m_Options;
	map<GlobalOption, bool> m_GlobalOptions;

public:
	OptionsId_t Create();

	inline bool IsValidOptionHandle(const OptionsId_t id)
	{
		return m_Options.find(id) != m_Options.end();
	}
	inline COptions *GetOptionHandle(OptionsId_t id)
	{
		return IsValidOptionHandle(id) ? m_Options.at(id) : nullptr;
	}
	inline const COptions *GetDefaultOptionHandle()
	{
		return m_Options.at(0);
	}

	inline bool GetGlobalOption(GlobalOption option)
	{
		return m_GlobalOptions[option];
	}
	inline void SetGlobalOption(GlobalOption option, bool value)
	{
		m_GlobalOptions[option] = value;
	}
};
