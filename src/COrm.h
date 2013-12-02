#pragma once
#ifndef INC_CORM_H
#define INC_CORM_H


#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

using std::string;
using std::vector;
using boost::unordered_map;

#include "main.h"


typedef boost::variant<int, double, string> VarType;


#define ERROR_INVALID_ORM_ID(function, id) \
	CLog::Get()->LogFunction(LOG_ERROR, #function, "invalid orm id (ID = %d)", id)


class CMySQLHandle;
class CMySQLResult;


class COrm 
{
public:
	static unsigned int Create(const char *table, CMySQLHandle *connhandle);
	void Destroy();

	static inline bool IsValid(int id) 
	{
		return OrmHandle.find(id) != OrmHandle.end();
	}
	static inline COrm *GetOrm(int id) 
	{
		return OrmHandle.at(id);
	}

	void ApplyActiveResult(unsigned int row);

	void GenerateSelectQuery(string &dest);
	void ApplySelectResult(CMySQLResult *result);
	void GenerateUpdateQuery(string &dest);
	void GenerateInsertQuery(string &dest);
	void ApplyInsertResult(CMySQLResult *result);
	void GenerateDeleteQuery(string &dest);
	unsigned short GenerateSaveQuery(string &dest);

	void ClearVariableValues();

	void AddVariable(const char *varname, cell *address, unsigned short datatype, size_t len=0);
	void SetVariableAsKey(const char *varname);

	inline CMySQLHandle *GetConnectionHandle() const 
	{
		return m_ConnHandle;
	}

	inline int GetErrorID() const 
	{
		return m_ErrorID;
	}

private:
	struct SVarInfo 
	{
		SVarInfo(const char *name, cell *addr, unsigned short datatype, size_t len) :
			Address(addr),
			MaxLen(len),
			Name(name),
			Datatype(datatype)
		{ }

		cell *Address;
		size_t MaxLen;
		string Name;
		unsigned short Datatype;
	};
	
	static unordered_map<unsigned int, COrm *> OrmHandle;


	COrm() :
		m_KeyVar(NULL),

		m_ConnHandle(NULL),

		m_MyID(0),
		m_ErrorID(0)
	{}
	~COrm();
	
	vector<SVarInfo *> m_Vars;
	SVarInfo *m_KeyVar;

	string m_TableName;
	CMySQLHandle *m_ConnHandle;
	unsigned int m_MyID;

	int m_ErrorID;
};

enum ORM_ERROR 
{
	ORM_ERROR_OK,
	ORM_ERROR_NO_DATA
};

enum ORM_QUERYTYPE 
{
	ORM_QUERYTYPE_SELECT=1,
	ORM_QUERYTYPE_UPDATE,
	ORM_QUERYTYPE_INSERT,
	ORM_QUERYTYPE_DELETE,

	ORM_QUERYTYPE_SAVE
};


#endif // INC_CORM_H
