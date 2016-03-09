#pragma once

#include "types.hpp"
#include "CSingleton.hpp"

#include <tuple>
#include <unordered_map>
#include <string>

using std::string;

#include <amx/amx.h>
#include "CError.hpp"


class COrm 
{
public:
	struct Variable
	{
		enum class Type
		{
			INVALID,
			INT,
			FLOAT,
			STRING
		};


		Variable(Type type, string name, 
			cell *variable, size_t var_len = 0) : 
			type(type),
			name(name),
			variable(variable),
			variable_maxlen(var_len)
		{ }
		

		Type type = Type::INVALID;
		string name;
		cell *variable = nullptr;
		size_t variable_maxlen = 0;
	};

public:
	enum class Error
	{
		NONE,
		EMPTY_TABLE,
		INVALID_CONNECTION_HANDLE
	};

	static const string ModuleName;

public:
	COrm(HandleId_t handleid, const char *tablename) :
		m_HandleId(handleid),
		m_Table(tablename)
	{ }
	~COrm() = default;

private:
	HandleId_t m_HandleId;
	string m_Table;

	std::vector<Variable> m_Variables;
	decltype(m_Variables)::iterator m_KeyVarIterator;

public:
	bool AddVariable(Variable::Type type, 
		const char *name, cell *var_addr, size_t var_maxlen = 0);
	bool RemoveVariable(const char *name);
	void ClearAllVariables();
	bool SetKeyVariable(const char *name);

	void GenerateSelectQuery(string &dest);
	void GenerateUpdateQuery(string &dest);
	void GenerateInsertQuery(string &dest);
	void GenerateDeleteQuery(string &dest);

	void ApplyResult(ResultSet_t result);
};

class COrmManager : public CSingleton<COrmManager>
{
	friend class CSingleton<COrmManager>;
private:
	COrmManager() = default;
	~COrmManager() = default;

private:
	std::unordered_map<OrmId_t, Orm_t> m_Instances;

public:
	std::tuple<OrmId_t, Orm_t> Create(
		HandleId_t handleid, const char *table,
		CError<COrm> &error);
	inline bool IsValid(OrmId_t id)
	{
		return m_Instances.find(id) != m_Instances.end();
	}
	inline Orm_t Find(OrmId_t id)
	{
		return IsValid(id) ? m_Instances.at(id) : nullptr;
	}
	bool Delete(OrmId_t id)
	{
		return m_Instances.erase(id) == 1;
	}
};
