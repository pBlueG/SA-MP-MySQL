#pragma once

#include "CSingleton.h"

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

using std::vector;
using std::string;
using std::shared_ptr;
using std::unordered_map;

typedef struct st_mysql MYSQL;
typedef unsigned long long my_ulonglong;


class CResult 
{
	friend class CResultSet;
private: //constructor / destructor
	CResult() = default;
	~CResult();

private: //variables
	unsigned int m_Fields = 0;
	my_ulonglong m_Rows = 0;

	char ***m_Data = nullptr;
	vector<string> m_FieldNames;

public: //functions
	inline my_ulonglong GetRowCount() const 
	{
		return m_Rows;
	}
	inline unsigned int GetFieldCount() const 
	{
		return m_Fields;
	}

	bool GetFieldName(unsigned int idx, string &dest) const;
	bool GetRowData(unsigned int row, unsigned int fieldidx, string &dest) const;
	bool GetRowDataByName(unsigned int row, const string &field, string &dest) const;

};

class CResultSet
{
public:
	using Id_t = unsigned int;
	using Type_t = shared_ptr<CResultSet>;

private:
	CResultSet() = default;

public:
	~CResultSet();

private:
	vector<CResult *> m_Results;
	CResult *m_ActiveResult = nullptr;

	my_ulonglong
		m_InsertId = 0,
		m_AffectedRows = 0;

	unsigned int m_WarningCount = 0;

public:
	inline const CResult *GetActiveResult()
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

public: //factory function
	static CResultSet::Type_t Create(MYSQL *connection);

};

class CResultSetManager : public CSingleton< CResultSetManager >
{
	friend class CSingleton< CResultSetManager >;
private:
	CResultSetManager() = default;
	~CResultSetManager() = default;

private:
	CResultSet::Type_t m_ActiveResultSet;
	unordered_map<CResultSet::Id_t, CResultSet::Type_t> m_StoredResults;

public:
	inline void SetActiveResultSet(CResultSet::Type_t resultset)
	{
		m_ActiveResultSet = resultset;
	}
	inline CResultSet::Type_t GetActiveResultSet()
	{
		return m_ActiveResultSet;
	}

	inline bool IsValidResultSet(CResultSet::Id_t id)
	{
		return m_StoredResults.find(id) != m_StoredResults.end();
	}
	CResultSet::Id_t StoreActiveResultSet();
	bool DeleteResultSet(CResultSet::Id_t id);
	inline CResultSet::Type_t GetResultSet(CResultSet::Id_t id)
	{
		return IsValidResultSet(id) ? m_StoredResults.at(id) : nullptr;
	}

};
