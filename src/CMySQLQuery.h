#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <string>
#include <stack>
#include <boost/variant.hpp>

using std::string;
using std::stack;

#include "main.h"
#include "CMySQLConnection.h"


class CMySQLHandle;
class CMySQLResult;
class COrm;


class CMySQLQuery
{
private:
	bool StoreResult(MYSQL *mysql_connection, MYSQL_RES *mysql_result);

public:
	bool Execute(MYSQL *mysql_connection);


	string Query;

	CMySQLHandle *Handle;
	CMySQLResult *Result;

	bool Unthreaded;

	struct s_Callback
	{
		stack< boost::variant<cell, string> > Params;
		string Name;
	} Callback;

	struct s_Orm
	{
		s_Orm() :
			Object(NULL),
			Type(0)
		{}
		
		COrm *Object;
		unsigned short Type;
	} Orm;


	CMySQLQuery() :
		Handle(NULL),
		Result(NULL),

		Unthreaded(false)
	{}
	~CMySQLQuery() {}
	
};


#endif // INC_CMYSQLQUERY_H
