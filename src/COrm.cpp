#include "COrm.hpp"
#include "CHandle.hpp"
#include "CResult.hpp"
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

//TODO: some more logging
string COrm::Variable::GetValueAsString()
{
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
		buffer[m_VarMaxLen] = '\0';
		res.assign(buffer.get());
		break;
	}
	return res;
}

void COrm::Variable::SetValue(const char *val)
{
	switch (m_Type)
	{
	case COrm::Variable::Type::INT:
		ConvertStrToData(val, (*m_VariableAddr));
		break;
	case COrm::Variable::Type::FLOAT: {
		float dest = 0.0f;
		if (ConvertStrToData(val, dest))
			(*m_VariableAddr) = amx_ftoc(dest);
		} break;
	case COrm::Variable::Type::STRING:
		amx_SetString(m_VariableAddr, 
			val != nullptr ? val : "NULL", 0, 0, m_VarMaxLen);
		break;
	}
}

bool COrm::AddVariable(Variable::Type type, 
	const char *name, cell *var_addr, size_t var_maxlen)
{
	if (type == Variable::Type::INVALID)
		return false;

	if (name == nullptr || strlen(name) == 0)
		return false;

	if (var_addr == nullptr)
		return false;

	if (type == Variable::Type::STRING && var_maxlen == 0)
		return false;

	if (m_KeyVariable.GetName().compare(name) == 0)
		return false;

	auto v = std::find_if(m_Variables.begin(), m_Variables.end(),
		[name](const Variable &v) -> bool
	{
		return v.GetName().compare(name) == 0;
	}); 
	
	if (v != m_Variables.end())
		return false;

	
	m_Variables.push_back(Variable(type, name, var_addr, var_maxlen));
	return true;
}

bool COrm::RemoveVariable(const char *name)
{
	if (name == nullptr || strlen(name) == 0)
		return false;

	if (m_KeyVariable.GetName().compare(name) == 0)
	{
		m_KeyVariable = Variable();
	}
	else
	{
		auto v = std::find_if(m_Variables.begin(), m_Variables.end(), 
			[name](const Variable &v) -> bool
			{
				return v.GetName().compare(name) == 0;
			});
		
		if (v == m_Variables.end())
			return false;
		
		m_Variables.erase(v);
	}
	return true;
}

void COrm::ClearAllVariables()
{
	for (auto &v : m_Variables)
		v.Clear();

	m_KeyVariable.Clear();
}

bool COrm::SetKeyVariable(const char *name)
{
	if (name == nullptr || strlen(name) == 0)
		return false;

	auto v = std::find_if(m_Variables.begin(), m_Variables.end(),
		[name](const Variable &v) -> bool
	{
		return v.GetName().compare(name) == 0;
	});

	if (v == m_Variables.end())
		return false;


	m_KeyVariable = *v;
	m_Variables.erase(v);
	return true;
}

CError<COrm> COrm::GenerateQuery(COrm::QueryType type, string &dest)
{
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
	if (m_KeyVariable && m_KeyVariable.GetValueAsCell() != 0)
		return QueryType::UPDATE;
	return QueryType::INSERT;
}

CError<COrm> COrm::GenerateSelectQuery(string &dest)
{
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
	if (!m_KeyVariable)
		return { COrm::Error::NO_KEY_VARIABLE, "no key variable set" };

	dest = fmt::format("DELETE FROM `{}` WHERE `{}`='{}' LIMIT 1",
		m_Table, m_KeyVariable.GetName(), m_KeyVariable.GetValueAsString());
	return {};
}

void COrm::ApplyResult(const Result_t result, unsigned int rowidx /*= 0*/)
{
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

bool COrm::UpdateKeyValue(const ResultSet_t result)
{
	if (result == nullptr || result->InsertId() == 0)
		return false;

	if (!m_KeyVariable)
		return false;

	m_KeyVariable.SetValue(static_cast<cell>(result->InsertId()));
	return true;
}

void COrm::WriteVariableNamesAsList(fmt::MemoryWriter &writer)
{
	writer << '`';
	for (size_t i = 0; i != m_Variables.size(); ++i)
	{
		if (i != 0)
			writer << "`,`";
		writer << m_Variables.at(i).GetName();
	}
	writer << '`';
}


std::tuple<OrmId_t, Orm_t> COrmManager::Create(
	HandleId_t handleid, const char *table,
	CError<COrm> &error)
{
	static const std::tuple<OrmId_t, Orm_t> empty_ret(0, nullptr);

	if (CHandleManager::Get()->IsValidHandle(handleid) == false)
	{
		error.set(COrm::Error::INVALID_CONNECTION_HANDLE, "invalid connection handle");
		return empty_ret;
	}
		
	if (table == nullptr || strlen(table) == 0)
	{
		error.set(COrm::Error::EMPTY_TABLE, "empty table name");
		return empty_ret;
	}


	OrmId_t id = 1;
	while (m_Instances.find(id) != m_Instances.end())
		++id;

	return std::make_tuple(id, std::make_shared<COrm>(handleid, table));
}
