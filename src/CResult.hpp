#pragma once

#include "CSingleton.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <tuple>
#include <boost/chrono.hpp>

using std::vector;
using std::string;
using std::unordered_map;
using default_clock = boost::chrono::steady_clock;

#include "types.hpp"
#include "mysql.hpp"


class CResult 
{
	friend class CResultSet;
public:
	struct FieldInfo
	{
		string Name;
		enum_field_types Type;
	};

private: //constructor / destructor
	CResult() = default;
	~CResult();

private: //variables
	unsigned int m_NumFields = 0;
	my_ulonglong m_NumRows = 0;

	char ***m_Data = nullptr;
	vector<FieldInfo> m_Fields;

public: //functions
	inline my_ulonglong GetRowCount() const 
	{
		return m_NumRows;
	}
	inline unsigned int GetFieldCount() const 
	{
		return m_NumFields;
	}

	bool GetFieldName(unsigned int idx, string &dest) const;
	bool GetFieldType(unsigned int idx, enum_field_types &dest) const;
	bool GetRowData(unsigned int row, unsigned int fieldidx, const char **dest) const;
	inline bool GetRowData(unsigned int row, unsigned int fieldidx, string &dest) const
	{
		const char *cdest = nullptr;
		bool result = GetRowData(row, fieldidx, &cdest);
		if(result && cdest != nullptr)
			dest = cdest;
		return result;
	}
	bool GetRowDataByName(unsigned int row, const string &field, const char **dest) const;
	bool GetRowDataByName(unsigned int row, const string &field, string &dest) const
	{
		const char *cdest = nullptr;
		bool result = GetRowDataByName(row, field, &cdest);
		if (result && cdest != nullptr)
			dest = cdest;
		return result;
	}

};

class CResultSet
{
private:
	CResultSet() = default;

public:
	~CResultSet();

private:
	vector<Result_t> m_Results;
	Result_t m_ActiveResult = nullptr;

	my_ulonglong
		m_InsertId = 0,
		m_AffectedRows = 0;

	unsigned int m_WarningCount = 0;

	unsigned int
		m_ExecTimeMilli = 0,
		m_ExecTimeMicro = 0;

	string m_ExecQuery;

public:
	inline const Result_t GetActiveResult()
	{
		if (m_ActiveResult == nullptr)
			m_ActiveResult = m_Results.front();
		return m_ActiveResult;
	}
	bool SetActiveResult(size_t result_idx)
	{
		if (result_idx < GetResultCount())
		{
			m_ActiveResult = m_Results.at(result_idx);
			return true;
		}
		return false;
	}
	inline size_t GetResultCount()
	{
		return m_Results.size();
	}
	inline const Result_t GetResultByIndex(size_t idx)
	{
		return idx < GetResultCount() ? m_Results.at(idx) : nullptr;
	}

	inline my_ulonglong InsertId() const
	{
		return m_InsertId;
	}
	inline my_ulonglong AffectedRows() const
	{
		return m_AffectedRows;
	}
	inline unsigned int WarningCount() const
	{
		return m_WarningCount;
	}

	inline auto GetExecutionTime() const
	{
		return std::make_tuple(m_ExecTimeMilli, m_ExecTimeMicro);
	}
	inline const string &GetExecutedQuery() const
	{
		return m_ExecQuery;
	}

public: //factory function
	static ResultSet_t Create(MYSQL *connection, 
		default_clock::duration &exec_time, string query_str);

};

class CResultSetManager : public CSingleton< CResultSetManager >
{
	friend class CSingleton< CResultSetManager >;
private:
	CResultSetManager() = default;
	~CResultSetManager() = default;

private:
	ResultSet_t m_ActiveResultSet;
	unordered_map<ResultSetId_t, ResultSet_t> m_StoredResults;

public:
	inline void SetActiveResultSet(ResultSet_t resultset)
	{
		m_ActiveResultSet = resultset;
	}
	inline ResultSet_t GetActiveResultSet()
	{
		return m_ActiveResultSet;
	}

	inline bool IsValidResultSet(ResultSetId_t id)
	{
		return m_StoredResults.find(id) != m_StoredResults.end();
	}
	ResultSetId_t StoreActiveResultSet();
	bool DeleteResultSet(ResultSetId_t id);
	inline ResultSet_t GetResultSet(ResultSetId_t id)
	{
		return IsValidResultSet(id) ? m_StoredResults.at(id) : nullptr;
	}

};
