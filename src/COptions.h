#pragma once

#include "CSingleton.h"

#include <string>
#include <map>
#include <unordered_map>
#include <boost/variant.hpp>

using std::string;
using std::map;
using std::unordered_map;


class COptions
{
	friend class COptionManager;
public:
	using Id_t = unsigned int;

	enum class Type
	{
		AUTO_RECONNECT, //true
		MULTI_STATEMENTS, //false
		POOL_SIZE, //2
		SERVER_PORT //3306
		//TODO: SSL stuff
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
			{ Type::SERVER_PORT, typeid(unsigned int).hash_code() }
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

class DLL_API COptionManager : public CSingleton<COptionManager>
{
	friend class CSingleton<COptionManager>;
public:
	enum class EGlobalOption
	{
		DUPLICATE_CONNECTIONS, //false

	};

private:
	COptionManager();
	~COptionManager();

private:
	unordered_map<COptions::Id_t, COptions *> m_Options;
	map<EGlobalOption, bool> m_GlobalOptions;

public:
	COptions::Id_t Create();

	inline bool IsValidOptionHandle(const COptions::Id_t id)
	{
		return m_Options.find(id) != m_Options.end();
	}
	inline COptions *GetOptionHandle(COptions::Id_t id)
	{
		return IsValidOptionHandle(id) ? m_Options.at(id) : nullptr;
	}
	inline const COptions *GetDefaultOptionHandle()
	{
		return m_Options.at(0);
	}

	inline bool GetGlobalOption(EGlobalOption option)
	{
		return m_GlobalOptions[option];
	}
	inline void SetGlobalOption(EGlobalOption option, bool value)
	{
		m_GlobalOptions[option] = value;
	}
};
