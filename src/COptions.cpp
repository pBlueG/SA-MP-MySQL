#include "COptions.h"


COptions::COptions()
{
	m_Options[EOption::DUPLICATE_CONNECTIONS] = false;
	m_Options[EOption::AUTO_RECONNECT] = true;

}

void COptions::SetOption(EOption option, bool value)
{
	m_Options.at(option) = value;

	//broadcast update to all connections
	for (auto &i : m_Interfaces)
		i->OnOptionUpdate(option, value);
}


IOptionActor::IOptionActor()
{
	COptions::Get()->RegisterInterface(this);
}

IOptionActor::~IOptionActor()
{
	COptions::Get()->EraseInterface(this);
}
