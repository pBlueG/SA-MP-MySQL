#include "CPawnPreparedStmt.hpp"
#include "CHandle.hpp"


CPawnPreparedStmt::~CPawnPreparedStmt()
{
	for (auto &v : m_VarMap)
		free(v.first);
}

void CPawnPreparedStmt::BindParam(unsigned int idx, ParamType type, cell *var_addr)
{
	enum_field_types internal_type;
	switch (type)
	{
	case ParamType::INT:
		internal_type = MYSQL_TYPE_LONG;
		break;
	case ParamType::FLOAT:
		internal_type = MYSQL_TYPE_FLOAT;
		break;
	case ParamType::STRING:
		internal_type = MYSQL_TYPE_VAR_STRING;
		break;
	}

	cell *var = static_cast<cell *>(malloc(sizeof(cell)));
	m_VarMap.emplace(var, var_addr);
	m_InternalStmt->BindParam(idx, internal_type, var);
}

bool CPawnPreparedStmt::Execute()
{
	Handle_t handle = CHandleManager::Get()->GetHandle(m_HandleId);
	if (handle == nullptr)
		return false;

	m_InternalStmt->OnExecutionFinished([this](ResultSet_t result)
	{
		for (auto &v : m_VarMap)
		{
			(*v.second) = (*v.first);
		}
	});
	return handle->Execute(CHandle::ExecutionType::THREADED, 
		std::static_pointer_cast<ISqlStatement>(m_InternalStmt));
}


PawnPrepStmt_t CPawnPrepStmtManager::Create(HandleId_t handleid, string &&query)
{
	PawnPrepStmtId_t id = 1;
	while (m_Stmts.find(id) != m_Stmts.end())
		++id;

	return PawnPrepStmt_t(new CPawnPreparedStmt(id, handleid, std::move(query)));
}
