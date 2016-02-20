#pragma once

#include "mysql.hpp"
#include <memory>
#include <vector>
#include <boost/any.hpp>
#include "types.hpp"
#include "ISqlStatement.hpp"


class CPreparedStmt : public ISqlStatement
{
public:
	CPreparedStmt(string &&query) :
		m_Query(std::move(query))
	{ }
	~CPreparedStmt() = default;

public:
	struct SParam
	{
		SParam(unsigned int idx, enum enum_field_types type, void *addr, 
			bool *isnull_var = nullptr) :
			column_index(idx),
			type(type),
			addr(addr),
			isnull_addr(isnull_var)
		{ }

		SParam(unsigned int idx, enum enum_field_types type, void *addr, 
			unsigned long max_size, unsigned long *length_var, bool *isnull_var = nullptr) :
			column_index(idx),
			type(type),
			addr(addr),
			isnull_addr(isnull_var),
			max_size(max_size),
			length_addr(length_var)
		{ }

		unsigned int column_index;
		enum enum_field_types type;
		void *addr = nullptr;
		unsigned long max_size = 0;
		unsigned long *length_addr = nullptr;
		bool *isnull_addr = nullptr;
	};
	
private:
	//unsigned int m_BindArrayLength;
	//MYSQL_BIND *m_BindArray = nullptr;
	string m_Query;
	std::vector<SParam> m_Params;
	MYSQL_STMT *m_Stmt = nullptr;

public:
	inline void BindParam(unsigned int index, enum enum_field_types param_type, void *param_addr, 
		bool *is_null_addr = nullptr)
	{
		m_Params.push_back(SParam(index, param_type, param_addr, is_null_addr));
	}

	inline void BindParam(unsigned int index, enum enum_field_types param_type, char *param_addr, 
		unsigned long param_size, unsigned long *value_length_addr, bool *is_null_addr = nullptr)
	{
		m_Params.push_back(SParam(index, param_type, param_addr, 
			param_size, value_length_addr, is_null_addr));
	}

	bool Execute(MYSQL *connection);

public: //factory function
	static PrepStmt_t Create(string &&query)
	{
		return std::make_shared<CPreparedStmt>(std::move(query));
	}
};
