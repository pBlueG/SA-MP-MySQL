#include "COrm.hpp"
#include "CHandle.hpp"
#include "CResult.hpp"
#include "CLog.hpp"
#include "misc.hpp"
#include <fmt/format.h>
#include <cstring>
#include <memory>
#include <amx/amx.h>

#ifdef NO_DATA
#undef NO_DATA //thanks M$
#endif
#ifdef DELETE
#undef DELETE //goddammit
#endif


const string COrm::ModuleName{ "orm" };

string COrm::Variable::GetValueAsString()
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::Variable::GetValueAsString(this={})",
		static_cast<const void *>(this));

	string res;
	switch (m_Type)
	{
	case COrm::Variable::Type::INVALID:
		return "INVALID";
	case COrm::Variable::Type::INT:
		return fmt::FormatInt(*(m_VariableAddr)).str();
	case COrm::Variable::Type::FLOAT:
		ConvertDataToStr(amx_ctof(*m_VariableAddr), res);
		break;
	case COrm::Variable::Type::STRING:
		std::unique_ptr<char[]> buffer(new char[m_VarMaxLen]);
		amx_GetString(buffer.get(), m_VariableAddr, 0, m_VarMaxLen);
		buffer[m_VarMaxLen - 1] = '\0';
		res.assign(buffer.get());
		break;
	}
	return res;
}

void COrm::Variable::SetValue(const char *value)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::Variable::SetValue(this={}, value='{}')",
		static_cast<const void *>(this), value ? value : "(nullptr)");

	switch (m_Type)
	{
	case COrm::Variable::Type::INT:
		ConvertStrToData(value, (*m_VariableAddr));
		break;
	case COrm::Variable::Type::FLOAT: {
		float dest = 0.0f;
		if (ConvertStrToData(value, dest))
			(*m_VariableAddr) = amx_ftoc(dest);
		} break;
	case COrm::Variable::Type::STRING:
		amx_SetString(m_VariableAddr, 
			value != nullptr ? value : "NULL", 0, 0, m_VarMaxLen);
		break;
	}
}

CError<COrm> COrm::AddVariable(Variable::Type type,
	const char *name, cell *var_addr, size_t var_maxlen)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::AddVariable(this={}, type={}, name='{}', var_addr={}, var_maxlen={})",
		static_cast<const void *>(this), static_cast<std::underlying_type<decltype(type)>::type>(type), 
		name ? name : "(nullptr)", static_cast<const void *>(var_addr), var_maxlen);
	
	if (type == Variable::Type::INVALID)
		return { Error::INVALID_VARIABLE_TYPE, "invalid variable type" };

	if (name == nullptr || strlen(name) == 0)
		return { Error::EMPTY_VARIABLE_NAME, "empty variable name" };

	if (var_addr == nullptr)
		return { Error::INVALID_PAWN_ADDRESS, "invalid variable PAWN address" };

	if (type == Variable::Type::STRING && var_maxlen == 0)
		return { Error::INVALID_MAX_LEN, "invalid maximal length for string type variable" };

	bool duplicate = false;
	for (auto &v : m_Variables)
	{
		if (v.GetName().compare(name) == 0)
		{
			duplicate = true;
			break;
		}
	}

	if (duplicate || m_KeyVariable.GetName().compare(name) == 0)
		return { Error::DUPLICATE_VARIABLE, "variable is already registered" };

	
	m_Variables.push_back(Variable(type, name, var_addr, var_maxlen));
	return {};
}

CError<COrm> COrm::RemoveVariable(const char *name)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::RemoveVariable(this={}, name='{}')",
		static_cast<const void *>(this), name ? name : "(nullptr)");
	
	if (name == nullptr || strlen(name) == 0)
		return { Error::EMPTY_VARIABLE_NAME, "empty variable name" };

	if (m_KeyVariable.GetName().compare(name) == 0)
	{
		m_KeyVariable = Variable(); // unset/clear key variable
	}
	else
	{
		auto v = std::find_if(m_Variables.begin(), m_Variables.end(), 
			[name](const Variable &v) -> bool
			{
				return v.GetName().compare(name) == 0;
			});
		
		if (v == m_Variables.end())
			return { Error::UNKNOWN_VARIABLE, "variable not found" };
		
		m_Variables.erase(v);
	}
	return {};
}

void COrm::ClearAllVariables()
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::ClearAllVariables(this={})",
		static_cast<const void *>(this));
	
	for (auto &v : m_Variables)
		v.Clear();

	m_KeyVariable.Clear();
}

CError<COrm> COrm::SetKeyVariable(const char *name)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::SetKeyVariable(this={}, name='{}')",
		static_cast<const void *>(this), name ? name : "(nullptr)");
	
	if (name == nullptr || strlen(name) == 0)
		return { Error::EMPTY_VARIABLE_NAME, "empty variable name" };

	auto v = std::find_if(m_Variables.begin(), m_Variables.end(),
		[name](const Variable &v) -> bool
	{
		return v.GetName().compare(name) == 0;
	});

	if (v == m_Variables.end())
		return { Error::UNKNOWN_VARIABLE, "variable not found" };

	//add old key variable back to normal variables
	if (m_KeyVariable)
		m_Variables.push_back(m_KeyVariable);

	m_KeyVariable = *v;
	m_Variables.erase(v);
	return {};
}

CError<COrm> COrm::GenerateQuery(COrm::QueryType type, string &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::GenerateQuery(this={}, type={})",
		static_cast<const void *>(this), static_cast<std::underlying_type<decltype(type)>::type>(type));
	
	switch (type)
	{
	case COrm::QueryType::SELECT:
		return GenerateSelectQuery(dest);
	case COrm::QueryType::UPDATE:
		return GenerateUpdateQuery(dest);
	case COrm::QueryType::INSERT:
		return GenerateInsertQuery(dest);
	case COrm::QueryType::DELETE:
		return GenerateDeleteQuery(dest);
	}
	return { COrm::Error::INVALID_QUERY_TYPE, "invalid query type" };
}

COrm::QueryType COrm::GetSaveQueryType()
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::GetSaveQueryType(this={})",
		static_cast<const void *>(this));
	
	if (m_KeyVariable && m_KeyVariable.GetValueAsCell() != 0)
		return QueryType::UPDATE;
	return QueryType::INSERT;
}

CError<COrm> COrm::GenerateSelectQuery(string &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::GenerateSelectQuery(this={})",
		static_cast<const void *>(this));
	
	if (m_Variables.empty())
		return { COrm::Error::NO_VARIABLES, "no registered variables" };

	if (!m_KeyVariable)
		return { COrm::Error::NO_KEY_VARIABLE, "no key variable set" };

	fmt::MemoryWriter writer;
	writer << "SELECT ";
	WriteVariableNamesAsList(writer);
	writer << " FROM `" << m_Table << "` WHERE `" << m_KeyVariable.GetName() 
		<< "`='" << m_KeyVariable.GetValueAsString() << "' LIMIT 1";

	dest.assign(writer.str());
	return {};
}

CError<COrm> COrm::GenerateUpdateQuery(string &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::GenerateUpdateQuery(this={})",
		static_cast<const void *>(this));
	
	if (m_Variables.empty())
		return { COrm::Error::NO_VARIABLES, "no registered variables" };

	if (!m_KeyVariable)
		return { COrm::Error::NO_KEY_VARIABLE, "no key variable set" };

	fmt::MemoryWriter writer;
	writer << "UPDATE `" << m_Table << "` SET `";
	for (size_t i = 0; i != m_Variables.size(); ++i)
	{
		if (i != 0)
			writer << "',`";
		Variable &var = m_Variables.at(i);
		writer << var.GetName() << "`='" << var.GetValueAsString();
	}
	writer << "' WHERE `" 
		<< m_KeyVariable.GetName() << "`='" << m_KeyVariable.GetValueAsString() << "' LIMIT 1";

	dest.assign(writer.str());
	return {};
}

CError<COrm> COrm::GenerateInsertQuery(string &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::GenerateInsertQuery(this={})",
		static_cast<const void *>(this));
	
	if (m_Variables.empty())
		return { COrm::Error::NO_VARIABLES, "no registered variables" };

	if (!m_KeyVariable)
		return { COrm::Error::NO_KEY_VARIABLE, "no key variable set" };

	fmt::MemoryWriter writer;
	writer << "INSERT INTO `" << m_Table << "` (";
	WriteVariableNamesAsList(writer);
	writer << ") VALUES ('";
	for (size_t i = 0; i != m_Variables.size(); ++i)
	{
		if (i != 0)
			writer << "','";
		writer << m_Variables.at(i).GetValueAsString();
	}
	writer << "')";

	dest.assign(writer.str());
	return {};
}

CError<COrm> COrm::GenerateDeleteQuery(string &dest)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::GenerateDeleteQuery(this={})",
		static_cast<const void *>(this));
	
	if (!m_KeyVariable)
		return { COrm::Error::NO_KEY_VARIABLE, "no key variable set" };

	dest = fmt::format("DELETE FROM `{}` WHERE `{}`='{}' LIMIT 1",
		m_Table, m_KeyVariable.GetName(), m_KeyVariable.GetValueAsString());
	return {};
}

void COrm::ApplyResult(const Result_t result, unsigned int rowidx /*= 0*/)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::ApplyResult(this={}, result={}, rowidx={})",
		static_cast<const void *>(this), static_cast<const void *>(result), rowidx);
	
	if (result == nullptr || rowidx >= result->GetRowCount())
	{
		m_Error = PawnError::NO_DATA;
		return;
	}

	const char *data = nullptr;
	for (size_t i = 0; i != m_Variables.size(); ++i)
	{
		if (result->GetRowData(rowidx, i, &data))
		{
			m_Variables.at(i).SetValue(data);
		}
	}
	m_Error = PawnError::OK;
}

bool COrm::ApplyResultByName(const Result_t result, unsigned int rowidx /*= 0*/)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::ApplyResultByName(this={}, result={}, rowidx={})",
		static_cast<const void *>(this), static_cast<const void *>(result), rowidx);
	
	if (result == nullptr || rowidx >= result->GetRowCount())
		return false;

	const char *data = nullptr;
	for (auto &v : m_Variables)
	{
		if (result->GetRowDataByName(rowidx, v.GetName(), &data))
		{
			v.SetValue(data);
		}
	}
	return true;
}

bool COrm::UpdateKeyValue(const Result_t result)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::UpdateKeyValue(this={}, result={})",
		static_cast<const void *>(this), static_cast<const void *>(result));
	
	if (result == nullptr || result->InsertId() == 0)
	{
		CLog::Get()->Log(LogLevel::ERROR, "COrm::UpdateKeyValue - no result or inserted id is zero");
		return false;
	}

	if (!m_KeyVariable)
	{
		CLog::Get()->Log(LogLevel::ERROR, "COrm::UpdateKeyValue - no key variable registered");
		return false;
	}

	m_KeyVariable.SetValue(static_cast<cell>(result->InsertId()));
	return true;
}

void COrm::WriteVariableNamesAsList(fmt::MemoryWriter &writer)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrm::WriteVariableNamesAsList(this={})",
		static_cast<const void *>(this));
	
	writer << '`';
	for (size_t i = 0; i != m_Variables.size(); ++i)
	{
		if (i != 0)
			writer << "`,`";
		writer << m_Variables.at(i).GetName();
	}
	writer << '`';
}


OrmId_t COrmManager::Create(HandleId_t handleid, const char *table,
	CError<COrm> &error)
{
	CLog::Get()->Log(LogLevel::DEBUG, "COrmManager::Create(handleid={}, table='{}')",
		handleid, table ? table : "(nullptr)");
	
	if (CHandleManager::Get()->IsValidHandle(handleid) == false)
	{
		error.set(COrm::Error::INVALID_CONNECTION_HANDLE, "invalid connection handle");
		return 0;
	}
	
	if (table == nullptr || strlen(table) == 0)
	{
		error.set(COrm::Error::EMPTY_TABLE, "empty table name");
		return 0;
	}


	OrmId_t id = 1;
	while (m_Instances.find(id) != m_Instances.end())
		++id;

	m_Instances.emplace(id, std::make_shared<COrm>(handleid, table));

	return id;
}
