#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <string>
#include <stack>

using std::string;
using std::stack;

#include "CMySQLResult.h"


class CMySQLHandle;
class CMySQLConnection;
class CCallback;
class COrm;


class CMySQLQuery
{
public:
	static CMySQLQuery Create(
		string query, const CMySQLConnection *connection,
		string cbname, string cbformat, stack<string> cbparams
		);
	//static CMySQLQuery Create(const char *query, CMySQLHandle *connhandle, const char *cbname, const char *cbformat, stack<string> cbparams, bool threaded = true, COrm *ormobject = NULL, unsigned short orm_querytype = 0);
	//static CMySQLQuery Create(const char *query) {}
	//void Destroy();

	//CMySQLQuery *Execute();


	string Query;
	//bool Threaded;

	//CMySQLHandle *ConnHandle;
	const CMySQLConnection *Connection;
	CMySQLResult Result;
	//CMySQLResult *Result;
	//CCallback *Callback;
	struct s_Callback
	{
		stack<string> Params;
		string Name;
		string Format;
	} Callback;

	//COrm *OrmObject;
	//unsigned short OrmQueryType;
	~CMySQLQuery() {}
private:
	CMySQLQuery(const CMySQLConnection *connection) :
		Connection(connection)
	{ }
	
};


#endif // INC_CMYSQLQUERY_H
