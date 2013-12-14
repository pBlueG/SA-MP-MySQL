#pragma once
#pragma warning (disable: 4996)

#include <cstdio>
#include <sstream>
using std::ostringstream;

//for the generating stuff
#include <boost/spirit/include/karma.hpp>
namespace karma = boost::spirit::karma;
using karma::auto_;
using karma::lit;

#include "COrm.h"
#include "CLog.h"
#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLConnection.h"

#include "misc.h"


unordered_map<unsigned int, COrm *> COrm::OrmHandle;


unsigned int COrm::Create(const char *table, CMySQLHandle *connhandle) 
{
	CLog::Get()->LogFunction(LOG_DEBUG, "COrm::Create", "creating new orm object..");

	if(table == NULL)
		return CLog::Get()->LogFunction(LOG_ERROR, "COrm::Create", "empty table name specified");

	if(connhandle == NULL)
		return CLog::Get()->LogFunction(LOG_ERROR, "COrm::Create", "invalid connection handle");

	unsigned int id = 1;
	if(OrmHandle.size() > 0) 
	{
		unordered_map<unsigned int, COrm*>::iterator itHandle = OrmHandle.begin();
		do 
		{
			id = itHandle->first+1;
			++itHandle;
		} while(OrmHandle.find(id) != OrmHandle.end());
	}


	COrm *OrmObject = new COrm;
	OrmObject->m_ConnHandle = connhandle;
	OrmObject->m_TableName.assign(table);
	OrmObject->m_MyID = id;

	OrmHandle.insert( unordered_map<int, COrm*>::value_type(id, OrmObject) );
	CLog::Get()->LogFunction(LOG_DEBUG, "COrm::Create", "orm object created with id = %d", id);
	return id;
}

void COrm::Destroy() 
{
	CLog::Get()->LogFunction(LOG_DEBUG, "COrm::Destroy", "id: %d", m_MyID);
	OrmHandle.erase(m_MyID);
	delete this;
}


void COrm::ApplyActiveResult(unsigned int row) 
{
	CMySQLResult *result = m_ConnHandle->GetActiveResult();
	
	m_ErrorID = ORM_ERROR_NO_DATA;
	if(result == NULL)
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "COrm::ApplyActiveResult", "no active result");

	if(row >= result->GetRowCount())
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "COrm::ApplyActiveResult", "invalid row specified");

	m_ErrorID = ORM_ERROR_OK;
	for(size_t v=0; v < m_Vars.size(); ++v) 
	{
		SVarInfo *var = m_Vars.at(v);

		const char *data = result->GetRowDataByName(row, var->Name.c_str());

		if(data != NULL) 
		{
			switch(var->Datatype) 
			{
				case DATATYPE_INT: 
				{
					int int_var = 0;
					if(ConvertStrToInt(data, int_var))
						(*var->Address) = int_var;
				} 
				break;
				case DATATYPE_FLOAT: 
				{
					float float_var = 0.0f;
					if(ConvertStrToFloat(data, float_var))
						(*var->Address) = amx_ftoc(float_var);

				} 
				break;
				case DATATYPE_STRING:
					amx_SetString(var->Address, data != NULL ? data : "NULL", 0, 0, var->MaxLen);
				break;
			}
		}
	}

	//also check for key in result
	if(m_KeyVar != NULL) 
	{
		const char *key_data = result->GetRowDataByName(row, m_KeyVar->Name.c_str());
		if(key_data != NULL) 
		{
			if(m_KeyVar->Datatype == DATATYPE_INT) 
			{
				int int_var = 0;
				if(ConvertStrToInt(key_data, int_var))
					(*(m_KeyVar->Address)) = int_var;
			}
			else if(m_KeyVar->Datatype == DATATYPE_STRING) 
				amx_SetString(m_KeyVar->Address, key_data, 0, 0, m_KeyVar->MaxLen);
		}
	}

}

void COrm::GenerateSelectQuery(string &dest) 
{
	vector<const char*> field_names;
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v)
		field_names.push_back( (*v)->Name.c_str() );

	VarType key_value;
	if(m_KeyVar->Datatype == DATATYPE_STRING) 
	{
		char *key_value_str = static_cast<char *>(alloca(sizeof(char) * (m_KeyVar->MaxLen + 1)));
		amx_GetString(key_value_str, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		if(key_value_str != NULL) 
		{
			string escaped_str;
			m_ConnHandle->GetMainConnection()->EscapeString(key_value_str, escaped_str);
			key_value = escaped_str;
		}
		else
			key_value = string();
	}
	else //DATATYPE_INT
		key_value = static_cast<int>(*(m_KeyVar->Address));

	std::back_insert_iterator<std::string> sink(dest);
	karma::generate(sink,
		lit("SELECT `") << karma::string % "`,`" << lit("` FROM `") << lit(m_TableName) << lit("` WHERE `") << lit(m_KeyVar->Name) << lit("`='") << auto_ << lit("' LIMIT 1"), 
		field_names, key_value
	);
}

void COrm::ApplySelectResult(CMySQLResult *result) 
{
	if(result == NULL || result->GetFieldCount() != m_Vars.size() || result->GetRowCount() != 1)
		m_ErrorID = ORM_ERROR_NO_DATA;
	else 
	{
		m_ErrorID = ORM_ERROR_OK;
		for(size_t i=0; i < m_Vars.size(); ++i) 
		{
			const SVarInfo *var = m_Vars.at(i);

			const char *data = result->GetRowData(0, i);

			switch(var->Datatype) 
			{
				case DATATYPE_INT: {
					int int_var = 0;
					if(ConvertStrToInt(data, int_var))
						(*var->Address) = int_var;
					} break;
				case DATATYPE_FLOAT: {
					float float_var = 0.0f;
					if(ConvertStrToFloat(data, float_var))
						(*var->Address) = amx_ftoc(float_var);
					} break;
				case DATATYPE_STRING: 
					amx_SetString(var->Address, data, 0, 0, var->MaxLen);
					break;
			}
		}
	}
}

void COrm::GenerateUpdateQuery(string &dest) 
{
	ostringstream str_format;
	char str_buf[4096];

	sprintf(str_buf, "UPDATE `%s` SET ", m_TableName.c_str());
	str_format << str_buf;

	bool FirstIt=true;
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v) 
	{
		const SVarInfo *var = (*v);

		switch(var->Datatype) 
		{
			case DATATYPE_INT:
				sprintf(str_buf, "%s`%s`='%d'", FirstIt == true ? "" : ",", var->Name.c_str(), static_cast<int>( *(var->Address) ));
				break;
			case DATATYPE_FLOAT:
				sprintf(str_buf, "%s`%s`='%f'", FirstIt == true ? "" : ",", var->Name.c_str(), static_cast<float>( amx_ctof(*(var->Address)) ));
				break;
			case DATATYPE_STRING:
				char *str_val = static_cast<char *>(alloca(sizeof(char) * var->MaxLen+1));
				amx_GetString(str_val, var->Address, 0, var->MaxLen);
				string escaped_str;
				m_ConnHandle->GetMainConnection()->EscapeString(str_val, escaped_str);
				sprintf(str_buf, "%s`%s`='%s'", FirstIt == true ? "" : ",", var->Name.c_str(), escaped_str.c_str());
				break;
		}

		str_format << str_buf;
		FirstIt = false;
	}

	if(m_KeyVar->Datatype == DATATYPE_STRING) 
	{
		char *key_value_str = static_cast<char *>(alloca(sizeof(char) * m_KeyVar->MaxLen+1));
		amx_GetString(key_value_str, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		string escaped_str;
		m_ConnHandle->GetMainConnection()->EscapeString(key_value_str, escaped_str);
		sprintf(str_buf, " WHERE `%s`='%s' LIMIT 1", m_KeyVar->Name.c_str(), escaped_str.c_str());
	}
	else //DATATYPE_INT
		sprintf(str_buf, " WHERE `%s`='%d' LIMIT 1", m_KeyVar->Name.c_str(), static_cast<int>( *(m_KeyVar->Address) ));

	str_format << str_buf;
	dest = str_format.str();
}


void COrm::GenerateInsertQuery(string &dest) 
{
	vector<const char*> field_names;
	vector<VarType> vars;
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v) 
	{
		field_names.push_back((*v)->Name.c_str());

		switch( (*v)->Datatype) 
		{
			case DATATYPE_INT:
				vars.push_back(static_cast<int>( *((*v)->Address) ));
				break;
			case DATATYPE_FLOAT:
				vars.push_back(static_cast<float>( amx_ctof(*((*v)->Address)) ));
				break;
			case DATATYPE_STRING:
				char *val_str = static_cast<char *>(alloca(sizeof(char) * (*v)->MaxLen+1));
				amx_GetString(val_str, (*v)->Address, 0, (*v)->MaxLen);
				string escaped_str;
				m_ConnHandle->GetMainConnection()->EscapeString(val_str, escaped_str);
				vars.push_back(escaped_str);
				break;
		}
	}

	std::back_insert_iterator<std::string> sink(dest);
	karma::generate(sink,
		lit("INSERT INTO `") << lit(m_TableName) << lit("` (`") <<  karma::string % "`,`" << lit("`) VALUES ('") << auto_ % "','" << lit("')"),
		field_names, vars
	);
}

void COrm::ApplyInsertResult(CMySQLResult *result) 
{
	if(result == NULL || result->InsertID() == 0)
		m_ErrorID = ORM_ERROR_NO_DATA;
	else 
	{
		m_ErrorID = ORM_ERROR_OK;
		if(m_KeyVar != NULL) 
		{
			//update KeyVar, force int-datatype
			m_KeyVar->Datatype = DATATYPE_INT;
			m_KeyVar->MaxLen = 0;
			(*(m_KeyVar->Address)) = (cell)result->InsertID();
		}
	}
}

void COrm::GenerateDeleteQuery(string &dest) 
{
	char str_buf[512];
	
	if(m_KeyVar->Datatype == DATATYPE_INT)
		sprintf(str_buf, "DELETE FROM %s WHERE `%s`='%d' LIMIT 1", m_TableName.c_str(), m_KeyVar->Name.c_str(), static_cast<int>( *(m_KeyVar->Address) ));
	else 
	{
		char *key_value_str = static_cast<char *>(alloca(sizeof(char) * m_KeyVar->MaxLen+1));
		amx_GetString(key_value_str, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		string escaped_str;
		m_ConnHandle->GetMainConnection()->EscapeString(key_value_str, escaped_str);
		sprintf(str_buf, "DELETE FROM `%s` WHERE `%s`='%s' LIMIT 1", m_TableName.c_str(), m_KeyVar->Name.c_str(), escaped_str.c_str());
	}
	dest = str_buf;
}

unsigned short COrm::GenerateSaveQuery(string &dest) 
{
	bool has_valid_key_value = false;
	if(m_KeyVar->Datatype == DATATYPE_STRING) 
	{
		char *key_value_str = static_cast<char *>(alloca(sizeof(char) * m_KeyVar->MaxLen+1));
		amx_GetString(key_value_str, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		has_valid_key_value = (strlen(key_value_str) > 0);
	}
	else //DATATYPE_INT
		has_valid_key_value = (static_cast<int>( *(m_KeyVar->Address) ) > 0);

	
	if(has_valid_key_value == true) //there is a valid key value -> update
	{
		GenerateUpdateQuery(dest);
		return ORM_QUERYTYPE_UPDATE;
	}
	else //no valid key value -> insert
	{
		GenerateInsertQuery(dest);
		return ORM_QUERYTYPE_INSERT;
	}
}



void COrm::AddVariable(const char *varname, cell *address, unsigned short datatype, size_t len) 
{
	if(varname == NULL || address == NULL)
		return ;

	//abort variable saving if there is already one with same name
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v)
		if((*v)->Name.compare(varname) == 0)
			return ;
	
	m_Vars.push_back( new SVarInfo(varname, address, datatype, len) );
}

void COrm::SetVariableAsKey(const char *varname) 
{
	//remove key if there is one
	if(m_KeyVar != NULL) 
	{
		m_Vars.push_back(m_KeyVar);
		m_KeyVar = NULL;
	}

	//set new key
	for(size_t i=0; i < m_Vars.size(); ++i) 
	{
		SVarInfo *key_var = m_Vars.at(i);
		if(key_var->Name.compare(varname) == 0) 
		{
			m_Vars.erase(m_Vars.begin()+i);
			if(m_KeyVar != NULL)
				delete m_KeyVar;
			m_KeyVar = key_var;
			break;
		}
	}
}

COrm::~COrm() 
{
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v)
		delete (*v);
	
	if(m_KeyVar != NULL)
		delete m_KeyVar;
}

void COrm::ClearVariableValues() 
{
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v) 
	{
		switch( (*v)->Datatype) 
		{
			case DATATYPE_INT:
				(*((*v)->Address)) = 0;
				break;
			case DATATYPE_FLOAT: {
				float EmtpyFloat = 0.0f;
				(*((*v)->Address)) = amx_ftoc(EmtpyFloat);
				} break;
			case DATATYPE_STRING:
				amx_SetString((*v)->Address, "", 0, 0, (*v)->MaxLen);
				break;
		}
	}
	//also clear key variable
	if(m_KeyVar->Datatype == DATATYPE_STRING)
		amx_SetString(m_KeyVar->Address, "", 0, 0, m_KeyVar->MaxLen);
	else //DATATYPE_INT
		(*(m_KeyVar->Address)) = 0;
}
