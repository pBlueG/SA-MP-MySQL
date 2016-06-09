#include <fstream>
#pragma warning (disable: 4348) //silence boost spirit warnings
#include <boost/spirit/include/qi.hpp>

#include "mysql.hpp"

#include "CQuery.hpp"
#include "CHandle.hpp"
#include "CConnection.hpp"
#include "COptions.hpp"
#include "misc.hpp"


const string CHandle::ModuleName{ "handle" };


CHandle::~CHandle()
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::~CHandle(this={})",
		static_cast<const void *>(this));

	if (m_MainConnection != nullptr)
		delete m_MainConnection;

	if (m_ThreadedConnection != nullptr)
		delete m_ThreadedConnection;

	if (m_ConnectionPool != nullptr)
		delete m_ConnectionPool;
}

bool CHandle::Execute(ExecutionType type, Query_t query)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::Execute(this={}, type={}, query={})",
		static_cast<const void *>(this), 
		static_cast<std::underlying_type<ExecutionType>::type>(type),
		static_cast<const void *>(query.get()));

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
		}
	}
	
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::Execute - return value: {}", return_val);
	return return_val;
}

bool CHandle::GetErrorId(unsigned int &errorid)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::GetErrorId(this={})",
		static_cast<const void *>(this));

	if (m_MainConnection == nullptr)
		return false;

	string unused_errormsg;
	bool return_val = m_MainConnection->GetError(errorid, unused_errormsg);

	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::GetErrorId - return value: {}, error id: '{}', error msg: '{}'", 
		return_val, errorid, unused_errormsg);

	return return_val;
}

bool CHandle::EscapeString(const char *src, StringEscapeResult_t &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::EscapeString(this={}, src='{}')",
		static_cast<const void *>(this), src ? src : "(nullptr)");

	if (m_MainConnection == nullptr)
		return false;

	bool return_val = m_MainConnection->EscapeString(src, dest);

	auto &escaped_str = std::get<0>(dest);
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::EscapeString - return value: {}, escaped string: '{}'",
		return_val, escaped_str ? escaped_str.get() : "(nullptr)");

	return return_val;
}

bool CHandle::SetCharacterSet(string charset)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::SetCharacterSet(this={}, charset='{}')",
		static_cast<const void *>(this), charset);

	if (m_MainConnection == nullptr)
		return false;

	return
		m_MainConnection->SetCharset(charset)
		&& m_ThreadedConnection->SetCharset(charset)
		&& ( (m_ConnectionPool != nullptr) ? m_ConnectionPool->SetCharset(charset) : true);
}

bool CHandle::GetCharacterSet(string &charset)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::GetCharacterSet(this={})",
		static_cast<const void *>(this));

	if (m_MainConnection == nullptr)
		return false;

	return m_MainConnection->GetCharset(charset);
}

bool CHandle::GetStatus(string &stat)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::GetStatus(this={})",
		static_cast<const void *>(this));

	if (m_MainConnection == nullptr)
		return false;

	return m_MainConnection->GetStatus(stat);
}

unsigned int CHandle::GetUnprocessedQueryCount()
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandle::GetUnprocessedQueryCount(this={})",
		static_cast<const void *>(this));

	unsigned int count = m_ThreadedConnection->GetUnprocessedQueryCount();
	if (m_ConnectionPool != nullptr)
		count += m_ConnectionPool->GetUnprocessedQueryCount();

	return count;
}



Handle_t CHandleManager::Create(const char *host, const char *user, 
	const char *pass, const char *db, const COptions *options, CError<CHandle> &error)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandleManager::Create(this={}, host='{}', user='{}', pass='****', db='{}', options={})",
		static_cast<const void *>(this), 
		host ? host : "(nullptr)", 
		user ? user : "(nullptr)", 
		db ? db : "(nullptr)", 
		static_cast<const void *>(options));
	CLog::Get()->Log(LogLevel::INFO, "Creating new connection handle...");

	if (host == nullptr || strlen(host) == 0)
	{
		error.set(CHandle::Error::EMPTY_HOST, "no hostname specified");
		return nullptr;
	}

	if (user == nullptr || strlen(user) == 0)
	{
		error.set(CHandle::Error::EMPTY_USER, "no username specified");
		return nullptr;
	}

	if (pass == nullptr)
		pass = "";
	if (strlen(pass) == 0)
		CLog::Get()->LogNative(LogLevel::WARNING, "no password specified");

	if (db == nullptr || strlen(db) == 0)
	{
		error.set(CHandle::Error::EMPTY_DATABASE, "no database specified");
		return nullptr;
	}

	if (options == nullptr)
	{
		error.set(CHandle::Error::INVALID_OPTIONS, "invalid option-handler");
		return nullptr;
	}


	HandleId_t id = 1;
	while (m_Handles.find(id) != m_Handles.end())
		++id;

	Handle_t handle = new CHandle(id);
	
	handle->m_MainConnection = new CConnection(host, user, pass, db, options);
	handle->m_ThreadedConnection = new CThreadedConnection(host, user, pass, db, options);

	auto pool_size = options->GetOption<unsigned int>(COptions::Type::POOL_SIZE);
	if (pool_size != 0)
		handle->m_ConnectionPool = new CConnectionPool(pool_size, host, user, pass, db, options);

	m_Handles.emplace(id, handle);

	CLog::Get()->Log(LogLevel::INFO, "Connection handle with id '{}' successfully created.", id);
	CLog::Get()->Log(LogLevel::DEBUG, "CHandleManager::Create - new handle = {}",
		static_cast<const void *>(handle));

	return handle;
}

Handle_t CHandleManager::CreateFromFile(string file_path, CError<CHandle> &error)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandleManager::CreateFromFile(this={}, file_path='{}')",
		static_cast<const void *>(this), file_path);

	std::ifstream file(file_path);
	if (file.fail())
	{
		error.set(CHandle::Error::INVALID_FILE,
			"invalid connection file specified (file: \"{}\")", file_path);
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
		},
		{ "ssl_enable", [&](string &val_str) 
			{
				bool val;
				if (ConvertStrToData(val_str, val))
					options->SetOption(COptions::Type::SSL_ENABLE, val);
			} 
		},
		{ "ssl_key_file", [&](string &val_str) 
			{ options->SetOption(COptions::Type::SSL_KEY_FILE, val_str); } 
		},
		{ "ssl_cert_file", [&](string &val_str) 
			{ options->SetOption(COptions::Type::SSL_CERT_FILE, val_str); } 
		},
		{ "ssl_ca_file", [&](string &val_str)
			{ options->SetOption(COptions::Type::SSL_CA_FILE, val_str); }
		},
		{ "ssl_ca_path", [&](string &val_str)
			{ options->SetOption(COptions::Type::SSL_CA_PATH, val_str); }
		},
		{ "ssl_cipher", [&](string &val_str)
			{ options->SetOption(COptions::Type::SSL_CIPHER, val_str); }
		},
	};

	while (file.good())
	{
		string line;
		std::getline(file, line);

		//erase prepending whitespace
		size_t first_char_pos = line.find_first_not_of(" \t");
		if (first_char_pos != string::npos)
			line.erase(0, first_char_pos);

		//erase comment from line
		size_t comment_pos = line.find_first_of("#;");
		if (comment_pos != string::npos)
			line.erase(comment_pos);
		
		if (line.empty())
			continue;

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
				error.set(CHandle::Error::UNKNOWN_FIELD, 
					"unknown field in connection file (field: \"{}\")", field);
				return nullptr;
			}
		}
		else
		{
			error.set(CHandle::Error::SYNTAX_ERROR,
				"syntax error in connection file (line: \"{}\")", line);
			return nullptr;
		}
	}

	CLog::Get()->Log(LogLevel::DEBUG, "CHandleManager::CreateFromFile - new options = {} (id '{}')",
		static_cast<const void *>(options), options_id);

	return Create(hostname.c_str(), username.c_str(), password.c_str(), database.c_str(), options, error);
}

bool CHandleManager::Destroy(Handle_t &handle)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CHandleManager::Destroy(this={}, handle={})",
		static_cast<const void *>(this), static_cast<const void *>(handle));

	if (handle == nullptr)
		return false;

	if (m_Handles.erase(handle->GetId()) == 0)
		return false;


	delete handle;
	return true;
}
