#include "COptions.hpp"


COptions::COptions()
{
	m_Options[Type::AUTO_RECONNECT] = true;
	m_Options[Type::MULTI_STATEMENTS] = false;
	m_Options[Type::POOL_SIZE] = 2u;
	m_Options[Type::SERVER_PORT] = 3306u;
	m_Options[Type::SSL_ENABLE] = false;
	m_Options[Type::SSL_KEY_FILE] = string();
	m_Options[Type::SSL_CERT_FILE] = string();
	m_Options[Type::SSL_CA_FILE] = string();
	m_Options[Type::SSL_CA_PATH] = string();
	m_Options[Type::SSL_CIPHER] = string();
}


COptionManager::COptionManager()
{
	m_GlobalOptions[GlobalOption::DUPLICATE_CONNECTIONS] = false;
	m_GlobalOptions[GlobalOption::DUPLICATE_CONNECTION_WARNING] = true;

	//create default options instance with id = 0
	m_Options.emplace(0, new COptions);
}

COptionManager::~COptionManager()
{
	for (auto o : m_Options)
		delete o.second;
}

OptionsId_t COptionManager::Create()
{
	OptionsId_t id = 1;
	while (m_Options.find(id) != m_Options.end())
		id++;

	m_Options.emplace(id, new COptions);
	return id;
}
