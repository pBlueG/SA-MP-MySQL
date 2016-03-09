#include "COrm.hpp"
#include "CHandle.hpp"
#include <cstring>


const string COrm::ModuleName{ "orm" };


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


	m_Variables.push_back(Variable(type, name, var_addr, var_maxlen));
	return true;
}

bool COrm::RemoveVariable(const char *name)
{
	if (name == nullptr || strlen(name) == 0)
		return false;

	auto v = std::find_if(m_Variables.begin(), m_Variables.end(), 
		[name](const Variable &v) -> bool
		{
			return v.name.compare(name) == 0;
		});
	
	if (v == m_Variables.end())
		return false;


	m_Variables.erase(v);
	return true;
}

void COrm::ClearAllVariables()
{
	for (auto &v : m_Variables)
	{
		if (v.variable != nullptr)
			(*v.variable) = 0;
	}
}

bool COrm::SetKeyVariable(const char *name)
{
	if (name == nullptr || strlen(name) == 0)
		return false;

	auto v = std::find_if(m_Variables.begin(), m_Variables.end(),
		[name](const Variable &v) -> bool
	{
		return v.name.compare(name) == 0;
	});

	if (v == m_Variables.end())
		return false;


	m_KeyVarIterator = v;
	return true;
}


void COrm::GenerateSelectQuery(string &dest)
{
}

void COrm::GenerateUpdateQuery(string &dest)
{
}

void COrm::GenerateInsertQuery(string &dest)
{
}

void COrm::GenerateDeleteQuery(string &dest)
{
}

void COrm::ApplyResult(ResultSet_t result)
{
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
