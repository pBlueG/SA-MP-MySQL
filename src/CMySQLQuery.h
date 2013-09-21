#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <string>
using std::string;


class CMySQLHandle;
class CMySQLConnection;
class CMySQLResult;
class CCallback;
class COrm;


class CMySQLQuery {
public:
	static CMySQLQuery *Create(const char *query, CMySQLHandle *connhandle, const char *cbname, const char *cbformat, bool threaded = true, COrm *ormobject = NULL, unsigned short orm_querytype = 0);
	void Destroy();

	void Execute();


	string Query;
	bool Threaded;

	CMySQLHandle *ConnHandle;
	CMySQLConnection *Connection;
	CMySQLResult *Result;
	CCallback *Callback;

	COrm *OrmObject;
	unsigned short OrmQueryType;

private:
	CMySQLQuery();
	~CMySQLQuery();
};


#endif // INC_CMYSQLQUERY_H
