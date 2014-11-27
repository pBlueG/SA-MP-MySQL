#pragma once

#include "CSingleton.h"

#include <string>
#include <forward_list>
#include <map>

using std::string;
using std::forward_list;
using std::map;

class IOptionActor;


class COptions : public CSingleton<COptions>
{
	friend class CSingleton<COptions>;
public:
	enum class EOption
	{
		DUPLICATE_CONNECTIONS, //false
		AUTO_RECONNECT //true
	};

private:
	COptions();
	~COptions() = default;

private:
	forward_list<IOptionActor *> m_Interfaces;
	map<EOption, bool> m_Options;

public:
	inline void RegisterInterface(IOptionActor *intrface)
	{
		m_Interfaces.push_front(intrface);
	}
	inline void EraseInterface(IOptionActor *intrface)
	{
		m_Interfaces.remove(intrface);
	}

	inline bool GetOption(EOption option)
	{
		return m_Options.at(option);
	}
	void SetOption(EOption option, bool value);

};

class IOptionActor
{
	friend class COptions;
public:
	IOptionActor();
	virtual ~IOptionActor();

private:
	virtual void OnOptionUpdate(COptions::EOption option, bool value) = 0;
};
