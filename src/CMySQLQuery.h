#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <string>
#include <stack>
#include <boost/variant.hpp>

using std::string;
using std::stack;

#include "main.h"


class CMySQLResult;
class CMySQLConnection;
class COrm;


class CMySQLQuery
{
public:
	static CMySQLQuery Create(
		string query, CMySQLConnection *connection, unsigned int connection_id,
		string cbname, stack< boost::variant<cell, string> > cbparams
		COrm *orm_object = NULL, unsigned short orm_querytype = 0
	);


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
