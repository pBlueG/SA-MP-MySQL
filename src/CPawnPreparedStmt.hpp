#pragma once

#include <string>
#include <unordered_map>

#include "CPreparedStmt.hpp"
#include "CSingleton.hpp"
#include "types.hpp"
#include <amx/amx.h>


class CPawnPreparedStmt
{
	friend class CPawnPrepStmtManager;
public:
	enum class ParamType
	{
		INT,
		FLOAT,
		STRING,
	};
private:
	CPawnPreparedStmt(PawnPrepStmtId_t id, HandleId_t hid, std::string &&query) :
		m_InternalStmt(CPreparedStmt::Create(std::move(query))),
		m_HandleId(hid),
		m_Id(id)
	{ }

public:
	~CPawnPreparedStmt();

private:
	HandleId_t m_HandleId;
	PawnPrepStmtId_t m_Id;
	PrepStmt_t m_InternalStmt;
	std::unordered_map<cell *, cell *> m_VarMap;

public:
	inline PawnPrepStmtId_t GetId() const
	{
		return m_Id;
	}

	void BindParam(unsigned int idx, ParamType type, cell *var_addr);
	bool Execute();
};


class CPawnPrepStmtManager : public CSingleton<CPawnPrepStmtManager>
{
	friend class CSingleton<CPawnPrepStmtManager>;
private:
	CPawnPrepStmtManager() = default;
	~CPawnPrepStmtManager() = default;

private:
	std::unordered_map<PawnPrepStmtId_t, PawnPrepStmt_t> m_Stmts;

public:
	PawnPrepStmt_t Create(HandleId_t handleid, string &&query);
	inline bool IsValidStatement(PawnPrepStmtId_t id)
	{
		return m_Stmts.find(id) != m_Stmts.end();
	}
	inline PawnPrepStmt_t GetStatement(PawnPrepStmtId_t id)
	{
		return IsValidStatement(id) ? m_Stmts.at(id) : nullptr;
	}
};
