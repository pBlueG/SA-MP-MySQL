#pragma once

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

#include "misc.h"


unordered_map<int, COrm *> COrm::OrmHandle;


int COrm::Create(char *table, CMySQLHandle *connhandle) {
	CLog::Get()->LogFunction(LOG_DEBUG, "COrm::Create", "creating new orm object..");

	if(table == NULL)
		return CLog::Get()->LogFunction(LOG_ERROR, "COrm::Create", "empty table name specified"), 0;

	if(connhandle == NULL)
		return CLog::Get()->LogFunction(LOG_ERROR, "COrm::Create", "invalid connection handle"), 0;

	int ID = 1;
	if(OrmHandle.size() > 0) {
		unordered_map<int, COrm*>::iterator itHandle = OrmHandle.begin();
		do {
			ID = itHandle->first+1;
			++itHandle;
		} while(OrmHandle.find(ID) != OrmHandle.end());
	}


	COrm *OrmObject = new COrm;
	OrmObject->m_ConnHandle = connhandle;
	OrmObject->m_TableName.assign(table);
	OrmObject->m_MyID = ID;

	OrmHandle.insert( unordered_map<int, COrm*>::value_type(ID, OrmObject) );
	CLog::Get()->LogFunction(LOG_DEBUG, "COrm::Create", "orm object created with id = %d", ID);
	return ID;
}

void COrm::Destroy() {
	CLog::Get()->LogFunction(LOG_DEBUG, "COrm::Destroy", "id: %d", m_MyID);
	OrmHandle.erase(m_MyID);
	delete this;
}


void COrm::ApplyActiveResult(unsigned int row) {
	CMySQLResult *Result = m_ConnHandle->GetActiveResult();
	
	m_ErrorID = ORM_ERROR_NO_DATA;
	if(Result == NULL)
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "COrm::ApplyActiveResult", "no active result");

	if(row >= Result->GetRowCount())
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "COrm::ApplyActiveResult", "invalid row specified");

	m_ErrorID = ORM_ERROR_OK;
	for(size_t v=0; v < m_Vars.size(); ++v) {
		SVarInfo *Var = m_Vars.at(v);

		char *Data = NULL;
		Result->GetRowDataByName(row, Var->Name.c_str(), &Data);

		if(Data != NULL) {
			switch(Var->Datatype) {
			case DATATYPE_INT: {
				int IntVar = 0;
				if(ConvertStrToInt(Data, IntVar))
					(*Var->Address) = IntVar;

				} break;
			case DATATYPE_FLOAT: {
				float FloatVar = 0.0f;
				if(ConvertStrToFloat(Data, FloatVar))
					(*Var->Address) = amx_ftoc(FloatVar);

				} break;
			case DATATYPE_STRING:
				amx_SetString(Var->Address, Data != NULL ? Data : "NULL", 0, 0, Var->MaxLen);
				break;
			}
		}
	}

	//also check for key in result
	if(m_KeyVar != NULL) {
		char *KeyData = NULL;
		Result->GetRowDataByName(row, m_KeyVar->Name.c_str(), &KeyData);
		if(KeyData != NULL) {
			if(m_KeyVar->Datatype == DATATYPE_INT) {
				int IntVar = 0;
				if(ConvertStrToInt(KeyData, IntVar))
					(*(m_KeyVar->Address)) = IntVar;
			}
			else if(m_KeyVar->Datatype == DATATYPE_STRING) {
				amx_SetString(m_KeyVar->Address, KeyData, 0, 0, m_KeyVar->MaxLen);
			}
		}
	}

}

void COrm::GenerateSelectQuery(string &dest) {
	vector<const char*> FieldNames;
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v)
		FieldNames.push_back( (*v)->Name.c_str() );

	VarType KeyValue;
	if(m_KeyVar->Datatype == DATATYPE_STRING) {
		char *KeyValStr = (char *)alloca(sizeof(char) * (m_KeyVar->MaxLen + 1));
		amx_GetString(KeyValStr, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		if(KeyValStr != NULL) {
			string EscapedStr;
			m_ConnHandle->GetMainConnection()->EscapeString(KeyValStr, EscapedStr);
			KeyValue = EscapedStr;
		}
		else
			KeyValue = string();
	}
	else //DATATYPE_INT
		KeyValue = static_cast<int>(*(m_KeyVar->Address));

	std::back_insert_iterator<std::string> sink(dest);
	karma::generate(sink,
		lit("SELECT `") << karma::string % "`,`" << lit("` FROM `") << lit(m_TableName) << lit("` WHERE `") << lit(m_KeyVar->Name) << lit("`='") << auto_ << lit("' LIMIT 1"), 
		FieldNames, KeyValue
	);
}

void COrm::ApplySelectResult(CMySQLResult *result) {
	if(result == NULL || result->GetFieldCount() != m_Vars.size() || result->GetRowCount() != 1)
		m_ErrorID = ORM_ERROR_NO_DATA;
	else {
		m_ErrorID = ORM_ERROR_OK;
		for(size_t i=0; i < m_Vars.size(); ++i) {
			SVarInfo *Var = m_Vars.at(i);

			char *Data = NULL;
			result->GetRowData(0, i, &Data);

			switch(Var->Datatype) {
				case DATATYPE_INT: {
					int IntVar = 0;
					if(ConvertStrToInt(Data, IntVar))
						(*Var->Address) = IntVar;
					
				} break;
				case DATATYPE_FLOAT: {
					float FloatVar = 0.0f;
					if(ConvertStrToFloat(Data, FloatVar))
						(*Var->Address) = amx_ftoc(FloatVar);

				} break;
				case DATATYPE_STRING: 
					amx_SetString(Var->Address, Data, 0, 0, Var->MaxLen);
				break;
			}
		}
	}
}

void COrm::GenerateUpdateQuery(string &dest) {
	ostringstream StrFormat;
	char StrBuf[4096];

	sprintf(StrBuf, "UPDATE `%s` SET ", m_TableName.c_str());
	StrFormat << StrBuf;

	bool FirstIt=true;
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v) {
		const SVarInfo *Var = (*v);

		switch( Var->Datatype) {
		case DATATYPE_INT:
			if(FirstIt == true)
			sprintf(StrBuf, "%s`%s`='%d'", FirstIt == true ? "" : ",", Var->Name.c_str(), static_cast<int>( *(Var->Address) ));
			break;
		case DATATYPE_FLOAT:
			sprintf(StrBuf, "%s`%s`='%f'", FirstIt == true ? "" : ",", Var->Name.c_str(), static_cast<float>( amx_ctof(*(Var->Address)) ));
			break;
		case DATATYPE_STRING:
			char *StrVal = (char *)alloca(sizeof(char) * Var->MaxLen+1);
			amx_GetString(StrVal, Var->Address, 0, Var->MaxLen);
			string EscapedStr;
			m_ConnHandle->GetMainConnection()->EscapeString(StrVal, EscapedStr);
			sprintf(StrBuf, "%s`%s`='%s'", FirstIt == true ? "" : ",", Var->Name.c_str(), EscapedStr.c_str());
			break;
		}

		StrFormat << StrBuf;
		FirstIt = false;
	}

	if(m_KeyVar->Datatype == DATATYPE_STRING) {
		char *StrVal = (char *)alloca(sizeof(char) * m_KeyVar->MaxLen+1);
		amx_GetString(StrVal, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		string EscapedStr;
		m_ConnHandle->GetMainConnection()->EscapeString(StrVal, EscapedStr);
		sprintf(StrBuf, " WHERE `%s`='%s' LIMIT 1", m_KeyVar->Name.c_str(), EscapedStr.c_str());
	}
	else //DATATYPE_INT
		sprintf(StrBuf, " WHERE `%s`='%d' LIMIT 1", m_KeyVar->Name.c_str(), static_cast<int>( *(m_KeyVar->Address) ));

	StrFormat << StrBuf;
	dest = StrFormat.str();
}


void COrm::GenerateInsertQuery(string &dest) {
	vector<const char*> FieldNames;
	vector<VarType> Vars;
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v) {
		FieldNames.push_back((*v)->Name.c_str());

		switch( (*v)->Datatype) {
		case DATATYPE_INT:
			Vars.push_back(static_cast<int>( *((*v)->Address) ));
			break;
		case DATATYPE_FLOAT:
			Vars.push_back(static_cast<float>( amx_ctof(*((*v)->Address)) ));
			break;
		case DATATYPE_STRING:
			char *StrVal = (char *)alloca(sizeof(char) * (*v)->MaxLen+1);
			amx_GetString(StrVal, (*v)->Address, 0, (*v)->MaxLen);
			string EscapedStr;
			m_ConnHandle->GetMainConnection()->EscapeString(StrVal, EscapedStr);
			Vars.push_back(EscapedStr);
			break;
		}
	}

	std::back_insert_iterator<std::string> sink(dest);
	karma::generate(sink,
		lit("INSERT INTO `") << lit(m_TableName) << lit("` (`") <<  karma::string % "`,`" << lit("`) VALUES ('") << auto_ % "','" << lit("')"),
		FieldNames, Vars
	);
}

void COrm::ApplyInsertResult(CMySQLResult *result) {
	if(result == NULL || result->InsertID() == 0)
		m_ErrorID = ORM_ERROR_NO_DATA;
	else {
		m_ErrorID = ORM_ERROR_OK;
		if(m_KeyVar != NULL) {
			//update KeyVar, force int-datatype
			m_KeyVar->Datatype = DATATYPE_INT;
			m_KeyVar->MaxLen = 0;
			(*(m_KeyVar->Address)) = (cell)result->InsertID();
		}
	}
}

void COrm::GenerateDeleteQuery(string &dest) {
	char StrBuf[512];
	
	if(m_KeyVar->Datatype == DATATYPE_INT)
		sprintf(StrBuf, "DELETE FROM %s WHERE `%s`='%d' LIMIT 1", m_TableName.c_str(), m_KeyVar->Name.c_str(), static_cast<int>( *(m_KeyVar->Address) ));
	else {
		char *StrVal = (char *)alloca(sizeof(char) * m_KeyVar->MaxLen+1);
		amx_GetString(StrVal, m_KeyVar->Address, 0, m_KeyVar->MaxLen);
		string EscapedStr;
		m_ConnHandle->GetMainConnection()->EscapeString(StrVal, EscapedStr);
		sprintf(StrBuf, "DELETE FROM `%s` WHERE `%s`='%s' LIMIT 1", m_TableName.c_str(), m_KeyVar->Name.c_str(), EscapedStr.c_str());
	}
	dest = StrBuf;
}



void COrm::AddVariable(char *varname, cell *address, unsigned short datatype, size_t len) {
	if(varname == NULL || address == NULL)
		return ;

	//abort variable saving if there is already one with same name
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v)
		if((*v)->Name.compare(varname) == 0)
			return ;
	
	m_Vars.push_back( new SVarInfo(varname, address, datatype, len) );
}

void COrm::SetVariableAsKey(char *varname) {
	//remove key if there is one
	if(m_KeyVar != NULL) {
		m_Vars.push_back(m_KeyVar);
		m_KeyVar = NULL;
	}

	//set new key
	for(size_t i=0; i < m_Vars.size(); ++i) {
		SVarInfo *KeyVar = m_Vars.at(i);
		if(KeyVar->Name.compare(varname) == 0) {
			m_Vars.erase(m_Vars.begin()+i);
			if(m_KeyVar != NULL)
				delete m_KeyVar;
			m_KeyVar = KeyVar;
			break;
		}
	}
}

COrm::~COrm() {
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v)
		delete (*v);
	
	if(m_KeyVar != NULL)
		delete m_KeyVar;
}

void COrm::ClearVariableValues() {
	for(vector<SVarInfo *>::iterator v = m_Vars.begin(), end = m_Vars.end(); v != end; ++v) {
		switch( (*v)->Datatype) {
		case DATATYPE_INT:
			(*((*v)->Address)) = 0;
			break;
		case DATATYPE_FLOAT: {
			float EmtpyFloat = 0.0f;
			(*((*v)->Address)) = amx_ftoc(EmtpyFloat);
		}   break;
		case DATATYPE_STRING:
			amx_SetString((*v)->Address, "", 0, 0, (*v)->MaxLen);
			break;
		}
	}
	//also clear key variable
	if(m_KeyVar->Datatype == DATATYPE_STRING) {
		amx_SetString(m_KeyVar->Address, "", 0, 0, m_KeyVar->MaxLen);
	}
	else //DATATYPE_INT
		(*(m_KeyVar->Address)) = 0;
}
