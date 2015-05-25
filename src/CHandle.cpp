#include "CQuery.hpp"
#include "CHandle.hpp"
#include "CConnection.hpp"
#include "COptions.hpp"
#include "misc.hpp"

#include <fstream>
#include <boost/spirit/include/qi.hpp>

using namespace boost::spirit;

#include "mysql.hpp"


CHandle::~CHandle()
{
	if (m_MainConnection != nullptr)
		delete m_MainConnection;

	if (m_ThreadedConnection != nullptr)
		delete m_ThreadedConnection;

	if (m_ConnectionPool != nullptr)
		delete m_ConnectionPool;
}

bool CHandle::Execute(ExecutionType type, Query_t query)
{
	bool return_val = false;
	if (query)
	{
		switch (type)
		{
		case ExecutionType::THREADED:
			if (m_ThreadedConnection != nullptr)
				return_val = m_ThreadedConnection->Queue(query);
			break;
		case ExecutionType::PARALLEL:
			if (m_ConnectionPool != nullptr)
				return_val = m_ConnectionPool->Queue(query);
			break;
		case ExecutionType::UNTHREADED:
			if (m_MainConnection != nullptr)
				return_val = m_MainConnection->Execute(query);
			break;

		default:
			; //TODO: log error
		}
	}
	return return_val;
}

bool CHandle::GetErrorId(unsigned int &errorid)
{
	if (m_MainConnection == nullptr)
		return false;

	string unused_errormsg;
	return m_MainConnection->GetError(errorid, unused_errormsg);
}

bool CHandle::EscapeString(const string &src, string &dest)
{
	if (m_MainConnection == nullptr)
		return false;

	return m_MainConnection->EscapeString(src.c_str(), dest);
}

bool CHandle::SetCharacterSet(string charset)
{
	if (m_MainConnection == nullptr)
		return false;

	return
		m_MainConnection->SetCharset(charset)
		&& m_ThreadedConnection->SetCharset(charset)
		&& m_ConnectionPool != nullptr ? m_ConnectionPool->SetCharset(charset) : true;
}

bool CHandle::GetCharacterSet(string &charset)
{
	if (m_MainConnection == nullptr)
		return false;

	return m_MainConnection->GetCharset(charset);
}

bool CHandle::GetStatus(string &stat)
{
	if (m_MainConnection == nullptr)
		return false;

	return m_MainConnection->GetStatus(stat);
}



CHandle *CHandleManager::Create(string host, string user, string pass, string db,
	const COptions *options, CHandle::Error &error)
{
	error = CHandle::Error::NONE;
	if (host.empty())
	{
		error = CHandle::Error::EMPTY_HOST;
		return nullptr;
	}

	if (user.empty())
	{
		error = CHandle::Error::EMPTY_USER;
		return nullptr;
	}

	if (pass.empty())
		;//TODO: warning

	if (db.empty())
	{
		error = CHandle::Error::EMPTY_DATABASE;
		return nullptr;
	}

	if (options == nullptr)
	{
		error = CHandle::Error::INVALID_OPTIONS;
		return nullptr;
	}


	HandleId_t id = 1;
	while (m_Handles.find(id) != m_Handles.end())
		id++;

	CHandle *handle = new CHandle(id);
	
	handle->m_MainConnection = new CConnection(host, user, pass, db, options);
	handle->m_ThreadedConnection = new CThreadedConnection(host, user, pass, db, options);

	auto pool_size = options->GetOption<unsigned int>(COptions::Type::POOL_SIZE);
	if (pool_size != 0)
		handle->m_ConnectionPool = new CConnectionPool(pool_size, host, user, pass, db, options);

	m_Handles.emplace(id, handle);
	return handle;
}

CHandle *CHandleManager::CreateFromFile(string file_path, CHandle::Error &error)
{
	error = CHandle::Error::NONE;

	std::ifstream file(file_path);
	if (file.fail())
	{
		error = CHandle::Error::INVALID_FILE;
		return nullptr;
	}

	string hostname, username, password, database;
	auto options_id = COptionManager::Get()->Create();
	COptions *options = COptionManager::Get()->GetOptionHandle(options_id);

	const std::unordered_map<string, function<void(string &)>> assign_map{
		{ "hostname", [&](string &val_str) { hostname = val_str; } },
		{ "username", [&](string &val_str) { username = val_str; } },
		{ "password", [&](string &val_str) { password = val_str; } },
		{ "database", [&](string &val_str) { database = val_str; } },
		{ "auto_reconnect", [&](string &val_str) 
			{
				bool val;
				if (ConvertStrToData(val_str, val))
					options->SetOption(COptions::Type::AUTO_RECONNECT, val);
			} 
		},
		{ "multi_statements", [&](string &val_str) 
			{
				bool val;
				if (ConvertStrToData(val_str, val))
					options->SetOption(COptions::Type::MULTI_STATEMENTS, val);
			} 
		},
		{ "pool_size", [&](string &val_str) 
			{
				unsigned int val = 0;
				if (ConvertStrToData(val_str, val))
					options->SetOption(COptions::Type::POOL_SIZE, val);
			} 
		},
		{ "server_port", [&](string &val_str) 
			{
				unsigned int val = 0;
				if (ConvertStrToData(val_str, val))
					options->SetOption(COptions::Type::SERVER_PORT, val);
			} 
		}
	};

	while (file.good())
	{
		string line;
		std::getline(file, line);

		if (line.empty())
			continue;

		//erase comment from line
		size_t comment_pos = line.find_first_of("#;");
		if (comment_pos != string::npos)
			line.erase(comment_pos);

		std::string field, data;
		if (qi::parse(line.begin(), line.end(),
			qi::skip(qi::space)[
				qi::as_string[+qi::char_("a-z_")] >> qi::lit('=') >> qi::as_string[+qi::graph]
			],
			field, data))
		{
			auto field_it = assign_map.find(field);
			if (field_it != assign_map.end() && data.empty() == false)
			{
				field_it->second(data);
			}
			else
			{
				error = CHandle::Error::INVALID_FIELD;
				return nullptr;
			}
		}
		else
		{
			error = CHandle::Error::SYNTAX_ERROR;
			return nullptr;
		}
	}

	return Create(hostname, username, password, database, options, error);
}

bool CHandleManager::Destroy(CHandle *handle)
{
	if (handle == nullptr)
		return false;

	if (m_Handles.erase(handle->GetId()) == 0)
		return false;


	delete handle;
	return true;
}
