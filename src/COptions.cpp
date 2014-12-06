#include "COptions.h"


COptions::COptions()
{
	m_Options[Type::AUTO_RECONNECT] = true;
	m_Options[Type::MULTI_STATEMENTS] = false;
	m_Options[Type::POOL_SIZE] = 2u;
	m_Options[Type::SERVER_PORT] = 3306u;
}


COptionManager::COptionManager()
{
	m_GlobalOptions[EGlobalOption::DUPLICATE_CONNECTIONS] = false;

	//create default options instance with id = 0
	m_Options.emplace(0, new COptions);
}

COptionManager::~COptionManager()
{
	for (auto o : m_Options)
		delete o.second;
}

COptions::Id_t COptionManager::Create()
{
	COptions::Id_t id = 1;
	while (m_Options.find(id) != m_Options.end())
		id++;

	m_Options.emplace(id, new COptions);
	return id;
}
