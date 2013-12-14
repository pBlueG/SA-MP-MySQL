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


class CMySQLResult;
class COrm;


class CMySQLQuery
{
private:
	void Execute(bool unthreaded = false);
	bool StoreResult(MYSQL *mysql_connection, MYSQL_RES *mysql_result);

public:
	static CMySQLQuery CreateThreaded(
		string query, CMySQLConnection *connection,
		string cb_name, stack< boost::variant<cell, string> > cb_params);

	static CMySQLQuery CreateUnthreaded(string query, CMySQLConnection *connection);

	static CMySQLQuery CreateOrm(
		string query, CMySQLConnection *connection,
		string cbname, stack< boost::variant<cell, string> > cbparams,
		COrm *orm_object, unsigned short orm_querytype);


	string Query;

	CMySQLConnection *Connection;
	CMySQLResult *Result;

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
		Connection(NULL),
		Result(NULL)
	{}
	~CMySQLQuery() {}
	
};


#endif // INC_CMYSQLQUERY_H
