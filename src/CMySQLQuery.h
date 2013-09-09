#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <string>
using std::string;


class CMySQLHandle;
class CMySQLResult;
class CCallback;
class COrm;


class CMySQLQuery {
public:
	static CMySQLQuery *Create(const char *query, CMySQLHandle *connhandle, const char *cbname, const char *cbformat, COrm *ormobject = NULL, unsigned short orm_querytype = 0);
	void Destroy();

	void Execute(bool threaded=true);


	string Query;

	CMySQLHandle *ConnHandle;
	CMySQLResult *Result;
	CCallback *Callback;

	COrm *OrmObject;
	unsigned short OrmQueryType;

private:
	CMySQLQuery();
	~CMySQLQuery();
};


#endif // INC_CMYSQLQUERY_H
