#pragma once

#include "types.hpp"
#include "CSingleton.hpp"

#include <tuple>
#include <unordered_map>
#include <string>
#include <vector>

using std::string;

#include <amx/amx.h>
#include "CError.hpp"


#ifdef NO_DATA
#undef NO_DATA //thanks M$
#endif
#ifdef DELETE
#undef DELETE //goddammit
#endif


class COrm 
{
public:
	class Variable
	{
	public:
		enum class Type
		{
			INVALID,
			INT,
			FLOAT,
			STRING
		};

	public:
		Variable(Type type, string name, 
			cell *variable, size_t var_len = 0) : 
			m_Type(type),
			m_Name(name),
			m_VariableAddr(variable),
			m_VarMaxLen(var_len)
		{ }
		Variable() = default;
		~Variable() = default;

	private:
		Type m_Type = Type::INVALID;
		string m_Name;
		cell *m_VariableAddr = nullptr;
		size_t m_VarMaxLen = 0;

	public:
		inline string const & GetName() const
		{
			return m_Name;
		}

		string GetValueAsString();
		inline cell GetValueAsCell() const
		{
			return *m_VariableAddr;
		}
		void SetValue(const char *val);
		inline void SetValue(cell val)
		{
			(*m_VariableAddr) = val;
		}
		inline void Clear()
		{
			if (m_VariableAddr != nullptr)
				(*m_VariableAddr) = 0;
		}

		inline operator bool() const
		{
			return m_Type != Type::INVALID;
		}
	};

public:
	enum class Error
	{
		NONE,
		EMPTY_TABLE,
		INVALID_CONNECTION_HANDLE,
		NO_VARIABLES,
		NO_KEY_VARIABLE,
		INVALID_QUERY_TYPE,
	};

	enum class PawnError //errors for Pawn
	{
		OK,
		NO_DATA,
	};

	enum class QueryType
	{
		INVALID,
		SELECT,
		UPDATE,
		INSERT,
		DELETE,
		SAVE, //not used to generate query
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
	Variable m_KeyVariable;

	PawnError m_Error = PawnError::OK;

public:
	inline HandleId_t GetHandleId() const
	{
		return m_HandleId;
	}

	bool AddVariable(Variable::Type type, 
		const char *name, cell *var_addr, size_t var_maxlen = 0);
	bool RemoveVariable(const char *name);
	void ClearAllVariables();
	bool SetKeyVariable(const char *name);

	CError<COrm> GenerateQuery(QueryType type, string &dest);
	QueryType GetSaveQueryType();

	void ApplyResult(const Result_t result, 
		unsigned int rowidx = 0U);
	bool ApplyResultByName(const Result_t result,
		unsigned int rowidx = 0U);
	bool UpdateKeyValue(const ResultSet_t result);

	inline PawnError GetError() const
	{
		return m_Error;
	}
	inline void ResetError()
	{
		m_Error = PawnError::OK;
	}
private:
	CError<COrm> GenerateSelectQuery(string &dest);
	CError<COrm> GenerateUpdateQuery(string &dest);
	CError<COrm> GenerateInsertQuery(string &dest);
	CError<COrm> GenerateDeleteQuery(string &dest);

	void WriteVariableNamesAsList(fmt::MemoryWriter &writer);
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
