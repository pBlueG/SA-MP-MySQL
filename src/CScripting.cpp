#pragma once

#include "CScripting.h"
#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"
#include "CCallback.h"
#include "COrm.h"
#include "CLog.h"

#include "misc.h"

#include "malloc.h"
#include <cmath>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
using namespace boost::spirit;


logprintf_t logprintf;


//native serialize_array(dest[], {Float, _}:array[], VarDatatype:datatype, max_arrsize=sizeof(array), element_size=1, max_destsize=sizeof(dest));
cell AMX_NATIVE_CALL Native::serialize_array(AMX *amx, cell *params) {
	cell *SrcArray = NULL;
	amx_GetAddr(amx, params[2], &SrcArray);
	unsigned short Datatype = params[3];
	cell 
		ArraySize = params[4],
		ElemSize = params[5],
		DestSize = params[6];

	if(ArraySize <= 1 || DestSize <= 1 || ElemSize <= 0)
		return 0;

	char DatatypeChar;
	switch(Datatype)
	{
		case DATATYPE_INT:
			DatatypeChar = 'd';
			break;
		case DATATYPE_FLOAT:
			DatatypeChar = 'f';
			break;
		case DATATYPE_STRING:
			DatatypeChar = 's';
			break;
		default:
			return 0;
	}

	vector<string> StrData;
	switch(Datatype) {
		case DATATYPE_INT: {
			for(cell i=0; i < ArraySize; ++i) {
				char IntBuf[12];
				ConvertIntToStr(SrcArray[i], IntBuf);
				StrData.push_back(string(IntBuf));
			}
		} break;
		case DATATYPE_FLOAT: {
			for(cell i=0; i < ArraySize; ++i) {
				char FloatBuf[84];
				ConvertFloatToStr(amx_ctof(SrcArray[i]), FloatBuf);
				StrData.push_back(string(FloatBuf));
			}
		} break;
		case DATATYPE_STRING: { 
			for(cell i=0; i < ArraySize; ++i) {
				char *TmpStr = (char *)alloca(ElemSize * sizeof(char));
				memset(TmpStr, 0, ElemSize * sizeof(char));

				//two-dimensional arrays <333
				cell Offset = (*(SrcArray + i))/sizeof(cell);
				cell *StrPtr = (SrcArray + i + Offset);
				amx_GetString(TmpStr, StrPtr, 0, ElemSize);

				//escape ';'
				for(unsigned int i=0, len=strlen(TmpStr); i < len; ++i)
					if(TmpStr[i] == ';')
						TmpStr[i] = '\x01';
				
				StrData.push_back(string(TmpStr));
			}
		} break;
	}
	

	string SerializedStr;
	std::back_insert_iterator<std::string> sink(SerializedStr);
	karma::generate(sink, 
			karma::lit(DatatypeChar) << karma::string % ';', 
			StrData
		);

	amx_SetCString(amx, params[1], SerializedStr.c_str(), DestSize);
	return 1;
}

//native unserialize_array(src[], dest_array[], max_elementsize = 1, max_destsize=sizeof(dest_array));
cell AMX_NATIVE_CALL Native::unserialize_array(AMX *amx, cell *params) {
	char *Source = NULL;
	amx_StrParam(amx, params[1], Source);
	cell *DestArray = NULL;
	amx_GetAddr(amx, params[2], &DestArray);
	cell 
		DestElementSize = params[3],
		DestArraySize = params[4];

	if(Source == NULL)
		return 0;

	const char DatatypeChar = Source[0];
	const char 
		*FirstIt(Source+1),
		*LastIt(FirstIt+strlen(Source));
	
	qi::rule<const char*, vector<boost::variant<int, float, string> >()> ParseType;
	vector< boost::variant<int, float, string> > ValArray;

	switch(DatatypeChar)
	{
	case 'd':
		ParseType = qi::int_ % ';';	
		break;
	case 'f':
		ParseType = qi::float_ % ';';
		break;
	case 's': {
		qi::rule<const char*, string()> TextParser = +(qi::ascii::char_ - ';');
		ParseType = TextParser % ';';
	} break;
	default:
		return 0;
	}

	if(qi::parse(FirstIt, LastIt, ParseType, ValArray) == false)
		return 0;

	if(ValArray.empty() || static_cast<cell>(ValArray.size()) > DestArraySize)
		return 0;
	

	switch(DatatypeChar)
	{
	case 'd':
		for(int i=0, size=ValArray.size(); i < size; ++i)
			DestArray[i] = static_cast<cell>(boost::get<int>(ValArray.at(i)));

		break;
	case 'f':
		for(int i=0, size=ValArray.size(); i < size; ++i) {
			float FloatVal = boost::get<float>(ValArray.at(i));
			DestArray[i] = amx_ftoc(FloatVal);
		}

		break;
	case 's':
		for(int i=0, size=ValArray.size(); i < size; ++i) {
			cell *StrDest = DestArray + 4 + (i*DestElementSize);

			//unescape ';'
			string &ValStr = boost::get<string>(ValArray.at(i));
			for(string::iterator c = ValStr.begin(), end = ValStr.end(); c != end; ++c)
				if( (*c) == '\x01')
					(*c) = ';';

			amx_SetString(StrDest, ValStr.c_str(), 0, 0, DestElementSize);
		}
		break;
	}

	return 1;
}

//native ORM:orm_create(table[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::orm_create(AMX* amx, cell* params) {
	int ConnID = params[2];
	char *TableName = NULL;
	amx_StrParam(amx, params[1], TableName);

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_create", "table: \"%s\", connectionHandle: %d", TableName, ConnID);

	if(!CMySQLHandle::IsValid(ConnID))
		return ERROR_INVALID_CONNECTION_HANDLE("orm_create", ConnID);
	
	return static_cast<cell>(COrm::Create(TableName, CMySQLHandle::GetHandle(ConnID)));
}

//native orm_destroy(ORM:id);
cell AMX_NATIVE_CALL Native::orm_destroy(AMX* amx, cell* params) {
	unsigned int OrmID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_destroy", "orm_id: %d", OrmID);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_destroy", OrmID);

	COrm::GetOrm(OrmID)->Destroy();
	return 1;
}

//native ORM_Error:orm_errno(ORM:id);
cell AMX_NATIVE_CALL Native::orm_errno(AMX* amx, cell* params) {
	unsigned int OrmID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_errno", "orm_id: %d", OrmID);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_errno", OrmID);

	return static_cast<cell>(COrm::GetOrm(OrmID)->GetErrorID());
}

// native orm_apply_cache(ORM:id, row);
cell AMX_NATIVE_CALL Native::orm_apply_cache(AMX* amx, cell* params) {
	unsigned int OrmID = params[1];
	unsigned int Row = params[2];

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_apply_cache", "orm_id: %d, row: %d", OrmID, Row);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_apply_cache", OrmID);

	COrm::GetOrm(OrmID)->ApplyActiveResult(Row);
	return 1;
}

//native orm_select(ORM:id, callback[], format[], {Float, _}:...);
cell AMX_NATIVE_CALL Native::orm_select(AMX* amx, cell* params) {
	const int ConstParamCount = 3;
	unsigned int OrmID = params[1];
	char 
		*ParamFormat = NULL,
		*CBName = NULL;
	amx_StrParam(amx, params[3], ParamFormat);
	amx_StrParam(amx, params[2], CBName);

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_select", "orm_id: %d, callback: \"%s\", format: \"%s\"", OrmID, CBName, ParamFormat);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_select", OrmID);

	if(ParamFormat != NULL && strlen(ParamFormat) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_select", "callback parameter count does not match format specifier length"), 0;


	COrm *OrmObject = COrm::GetOrm(OrmID);
	CMySQLQuery *Query = CMySQLQuery::Create(NULL, OrmObject->GetConnectionHandle(), CBName, ParamFormat, true, OrmObject, ORM_QUERYTYPE_SELECT);
	if(Query != NULL) {
		if(Query->Callback->Name.length() > 0)
			Query->Callback->FillCallbackParams(amx, params, ConstParamCount);

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenQuery(Query->Query);
			if(ShortenQuery.length() > 512)
				ShortenQuery.resize(512);
			CLog::Get()->LogFunction(LOG_DEBUG, "orm_select", "scheduling query \"%s\"..", ShortenQuery.c_str());
		}

		OrmObject->GetConnectionHandle()->ScheduleQuery(Query);
	}
	return 1;
}

//native orm_update(ORM:id);
cell AMX_NATIVE_CALL Native::orm_update(AMX* amx, cell* params) {
	unsigned int OrmID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_update", "orm_id: %d", OrmID);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_update", OrmID);
	

	COrm *OrmObject = COrm::GetOrm(OrmID);
	CMySQLQuery *Query = CMySQLQuery::Create(NULL, OrmObject->GetConnectionHandle(), NULL, NULL, true, OrmObject, ORM_QUERYTYPE_UPDATE);
	if(Query != NULL) {
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenQuery(Query->Query);
			if(ShortenQuery.length() > 512)
				ShortenQuery.resize(512);
			CLog::Get()->LogFunction(LOG_DEBUG, "orm_update", "scheduling query \"%s\"..", ShortenQuery.c_str());
		}

		OrmObject->GetConnectionHandle()->ScheduleQuery(Query);
	}
	return 1;
}

//native orm_insert(ORM:id, callback[]="", format[]="", {Float, _}:...);
cell AMX_NATIVE_CALL Native::orm_insert(AMX* amx, cell* params) {
	const int ConstParamCount = 3;
	unsigned int OrmID = params[1];
	char 
		*ParamFormat = NULL,
		*CBName = NULL;
	amx_StrParam(amx, params[3], ParamFormat);
	amx_StrParam(amx, params[2], CBName);

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_insert", "orm_id: %d, callback: \"%s\", format: \"%s\"", OrmID, CBName, ParamFormat);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_insert", OrmID);

	if(ParamFormat != NULL && strlen(ParamFormat) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_insert", "callback parameter count does not match format specifier length"), 0;


	COrm *OrmObject = COrm::GetOrm(OrmID);
	CMySQLQuery *Query = CMySQLQuery::Create(NULL, OrmObject->GetConnectionHandle(), CBName, ParamFormat, true, OrmObject, ORM_QUERYTYPE_INSERT);
	if(Query != NULL) {
		if(Query->Callback->Name.length() > 0)
			Query->Callback->FillCallbackParams(amx, params, ConstParamCount);

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenQuery(Query->Query);
			if(ShortenQuery.length() > 512)
				ShortenQuery.resize(512);
			CLog::Get()->LogFunction(LOG_DEBUG, "orm_insert", "scheduling query \"%s\"..", ShortenQuery.c_str());
		}

		OrmObject->GetConnectionHandle()->ScheduleQuery(Query);
	}
	return 1;
}

//native orm_delete(ORM:id, bool:clearvars=true);
cell AMX_NATIVE_CALL Native::orm_delete(AMX* amx, cell* params) {
	unsigned int OrmID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_delete", "orm_id: %d, clearvars: %d", OrmID, params[2]);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_delete", OrmID);


	COrm *OrmObject = COrm::GetOrm(OrmID);

	CMySQLQuery *Query = CMySQLQuery::Create(NULL, OrmObject->GetConnectionHandle(), NULL, NULL, true, OrmObject, ORM_QUERYTYPE_DELETE);
	if(Query != NULL) {

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenQuery(Query->Query);
			if(ShortenQuery.length() > 512)
				ShortenQuery.resize(512);
			CLog::Get()->LogFunction(LOG_DEBUG, "orm_delete", "scheduling query \"%s\"..", ShortenQuery.c_str());
		}

		OrmObject->GetConnectionHandle()->ScheduleQuery(Query);

		if(!!(params[2]) == true)
			OrmObject->ClearVariableValues();
	}
	return 1;
}

//native orm_save(ORM:id, callback[]="", format[]="", {Float, _}:...);
cell AMX_NATIVE_CALL Native::orm_save(AMX* amx, cell* params) {
	const int ConstParamCount = 3;
	unsigned int OrmID = params[1];
	char 
		*ParamFormat = NULL,
		*CBName = NULL;
	amx_StrParam(amx, params[3], ParamFormat);
	amx_StrParam(amx, params[2], CBName);

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_save", "orm_id: %d, callback: \"%s\", format: \"%s\"", OrmID, CBName, ParamFormat);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_save", OrmID);

	if(ParamFormat != NULL && strlen(ParamFormat) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_save", "callback parameter count does not match format specifier length"), 0;


	COrm *OrmObject = COrm::GetOrm(OrmID);
	CMySQLQuery *Query = CMySQLQuery::Create(NULL, OrmObject->GetConnectionHandle(), CBName, ParamFormat, true, OrmObject, ORM_QUERYTYPE_SAVE);
	if(Query != NULL) {
		if(Query->Callback->Name.length() > 0)
			Query->Callback->FillCallbackParams(amx, params, ConstParamCount);

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenQuery(Query->Query);
			if(ShortenQuery.length() > 512)
				ShortenQuery.resize(512);
			CLog::Get()->LogFunction(LOG_DEBUG, "orm_save", "scheduling query \"%s\"..", ShortenQuery.c_str());
		}

		OrmObject->GetConnectionHandle()->ScheduleQuery(Query);
	}
	return 1;
}

//native orm_addvar(ORM:id, &{Float, _}:var, VarDatatype:datatype, var_maxlen, varname[]);
cell AMX_NATIVE_CALL Native::orm_addvar(AMX* amx, cell* params) {
	char *VarName = NULL;
	cell *VarAddress = NULL;

	unsigned int OrmID = params[1];
	amx_GetAddr(amx, params[2], &VarAddress);
	unsigned short VarDatatype = (unsigned short)params[3];
	int VarMaxLen = params[4];
	amx_StrParam(amx, params[5], VarName);

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_addvar", "orm_id: %d, var: %p, datatype: %d, varname: \"%s\", var_maxlen: %d", OrmID, VarAddress, VarDatatype, VarName, VarMaxLen);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_addvar", OrmID);

	if(VarDatatype != DATATYPE_INT && VarDatatype != DATATYPE_FLOAT && VarDatatype != DATATYPE_STRING)
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_addvar", "unknown datatype specified"), 0;

	if(VarMaxLen <= 0)
		return CLog::Get()->LogFunction(LOG_ERROR, "orm_addvar", "invalid variable length specified"), 0;

	COrm *OrmObject = COrm::GetOrm(OrmID);
	OrmObject->AddVariable(VarName, VarAddress, VarDatatype, VarMaxLen);
	return 1;
}

//native orm_setkey(ORM:id, varname[]);
cell AMX_NATIVE_CALL Native::orm_setkey(AMX* amx, cell* params) {
	unsigned int OrmID = params[1];
	char *VarName = NULL;
	amx_StrParam(amx, params[2], VarName);

	CLog::Get()->LogFunction(LOG_DEBUG, "orm_setkey", "orm_id: %d, varname: \"%s\"", OrmID, VarName);

	if(!COrm::IsValid(OrmID))
		return ERROR_INVALID_ORM_ID("orm_setkey", OrmID);

	if(VarName != NULL)
		COrm::GetOrm(OrmID)->SetVariableAsKey(VarName);
	else
		CLog::Get()->LogFunction(LOG_ERROR, "orm_setkey", "empty variable name specified");
	return 1;
}


//native cache_affected_rows(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_affected_rows(AMX* amx, cell* params) {
	unsigned int cID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "cache_affected_rows", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_affected_rows", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_affected_rows", "no active cache");
		return 0;
	}
	
	return static_cast<cell>(Result->AffectedRows());
}

//native cache_warning_count(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_warning_count(AMX* amx, cell* params) {
	unsigned int cID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "cache_warning_count", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_warning_count", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_warning_count", "no active cache");
		return 0;
	}
	
	return static_cast<cell>(Result->WarningCount());
}

//native cache_insert_id(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_insert_id(AMX* amx, cell* params) {
	unsigned int cID = params[1];

	CLog::Get()->LogFunction(LOG_DEBUG, "cache_insert_id", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_insert_id", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_insert_id", "no active cache");
		return 0;
	}
	
	return static_cast<cell>(Result->InsertID());
}


// native Cache:cache_save(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_save(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_save", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_save", cID);
		return 0;
	}
	
	int CacheID = CMySQLHandle::GetHandle(cID)->SaveActiveResult();
	if(CacheID == 0)
		CLog::Get()->LogFunction(LOG_WARNING, "cache_save", "no active cache");

	return static_cast<cell>(CacheID);
}

// native cache_delete(Cache:id, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_delete(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_delete", "cache_id: %d, connection: %d", params[1], cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_delete", cID);
		return 0;
	}

	return static_cast<cell>(CMySQLHandle::GetHandle(cID)->DeleteSavedResult(params[1]));
}

// native cache_set_active(Cache:id, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_set_active(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_set_active", "cache_id: %d, connection: %d", params[1], cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_set_active", cID);
		return 0;
	}

	return static_cast<cell>(CMySQLHandle::GetHandle(cID)->SetActiveResult((int)params[1]) == true ? 1 : 0);
}

// native cache_get_row_count(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_row_count(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_count", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row_count", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_count", "no active cache");
		return 0;
	}

	return static_cast<cell>(Result->GetRowCount());
}

// native cache_get_field_count(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_field_count(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_count", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_count", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_count", "no active cache");
		return 0;
	}

	return static_cast<cell>(Result->GetFieldCount());
}

// native cache_get_data(&num_rows, &num_fields, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_data(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_data", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_data", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_data", "no active cache");
		return 0;
	}

	cell *AddressPtr;
	amx_GetAddr(amx, params[1], &AddressPtr);
	(*AddressPtr) = static_cast<cell>(Result->GetRowCount());
	amx_GetAddr(amx, params[2], &AddressPtr);
	(*AddressPtr) = static_cast<cell>(Result->GetFieldCount());
	return 1;
}

// native cache_get_field_name(field_index, dest[], connectionHandle = 1)
cell AMX_NATIVE_CALL Native::cache_get_field_name(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	int FieldIndex = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_name", "field_index: %d, connection: %d", FieldIndex, cID);

	if(FieldIndex < 0) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_name", "invalid field index");
		return 0;
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_name", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_name", "no active cache");
		return 0; 
	}
	
	char *FieldName = NULL;
	Result->GetFieldName(FieldIndex, &FieldName);

	amx_SetCString(amx, params[2], FieldName == NULL ? "NULL" : FieldName, params[4]);
	return 1;
}

// native cache_get_row(row, field_idx, destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::cache_get_row(AMX* amx, cell* params) {
	unsigned int cID = params[4];
	int 
		Row = params[1],
		FieldIndex = params[2],
		MaxLen = params[5];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row", "row: %d, field_idx: %d, connection: %d, max_len: %d", Row, FieldIndex, cID, MaxLen);

	if(Row < 0) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row", "invalid row number");
		return 0;
	}

	if(FieldIndex < 0) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row", "invalid field index");
		return 0;
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row", "no active cache");
		return 0;
	}

	char *RowData = NULL;
	Result->GetRowData(Row, FieldIndex, &RowData);

	amx_SetCString(amx, params[3], RowData == NULL ? "NULL" : RowData, MaxLen);
	return 1;
}

// native cache_get_row_int(row, field_idx, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_row_int(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	int
		Row = params[1],
		FieldIndex = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_int", "row: %d, field_idx: %d, connection: %d", Row, FieldIndex, cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row_int", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_int", "no active cache");
		return 0;
	}

	char *RowData = NULL;
	int ReturnVal = 0;
	Result->GetRowData(Row, FieldIndex, &RowData);
	if(RowData != NULL) {
		if(ConvertStrToInt(RowData, ReturnVal) == false) {
			CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_int", "invalid datatype");
			ReturnVal = 0;
		}
	}

	return static_cast<cell>(ReturnVal);
}

// native Float:cache_get_row_float(row, field_idx, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_row_float(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	int
		Row = params[1],
		FieldIndex = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_float", "row: %d, field_idx: %d, connection: %d", Row, FieldIndex, cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row_float", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_float", "no active cache");
		return 0;
	}

	float ReturnVal = 0.0f;
	char *RowData = NULL;
	Result->GetRowData(Row, FieldIndex, &RowData);
	if(RowData != NULL) {
		if(ConvertStrToFloat(RowData, ReturnVal) == false) {
			CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_float", "invalid datatype");
			ReturnVal = 0.0f;
		}
	}
	
	return amx_ftoc(ReturnVal);
}

// native cache_get_field_content(row, const field_name[], destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::cache_get_field_content(AMX* amx, cell* params) {
	unsigned int cID = params[4];
	int 
		Row = params[1],
		MaxLen = params[5];
	char *FieldName = NULL;
	amx_StrParam(amx, params[2], FieldName);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content", "row: %d, field_name: \"%s\", connection: %d, max_len: %d", Row, FieldName, cID, MaxLen);

	if(Row < 0) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content", "invalid row number");
		return 0;
	}

	if(FieldName == NULL) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content", "empty field name specified");
		return 0;
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_content", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content", "no active cache");
		return 0;
	}
	
	char *FieldData = NULL;
	Result->GetRowDataByName(Row, FieldName, &FieldData);

	amx_SetCString(amx, params[3], FieldData == NULL ? "NULL" : FieldData, MaxLen);
	return 1;
}

// native cache_get_field_content_int(row, const field_name[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_field_content_int(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	int Row = params[1];
	char *FieldName = NULL;
	amx_StrParam(amx, params[2], FieldName);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content_int", "row: %d, field_name: \"%s\", connection: %d", Row, FieldName, cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_content_int", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content_int", "no active cache");
		return 0;
	}

	int ReturnVal = 0;
	char *FieldData = NULL;
	Result->GetRowDataByName(Row, FieldName, &FieldData);

	if(FieldData != NULL) {
		if(ConvertStrToInt(FieldData, ReturnVal) == false) {
			CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_int", "invalid datatype");
			ReturnVal = 0;
		}
	}
	return static_cast<cell>(ReturnVal);
}

// native Float:cache_get_field_content_float(row, const field_name[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_field_content_float(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	int Row = params[1];
	char *FieldName = NULL;
	amx_StrParam(amx, params[2], FieldName);
	CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content_float", "row: %d, field_name: \"%s\", connection: %d", Row, FieldName, cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_content_float", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetActiveResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content_float", "no active cache");
		return 0;
	}

	float ReturnVal = 0.0f;
	char *FieldData = NULL;
	Result->GetRowDataByName(params[1], FieldName, &FieldData);

	if(FieldData != NULL) {
		if(ConvertStrToFloat(FieldData, ReturnVal) == false) {
			CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_float", "invalid datatype");
			ReturnVal = 0.0f;
		}
	}
	return amx_ftoc(ReturnVal);
}

//native mysql_connect(const host[], const user[], const database[], const password[], port = 3306, bool:autoreconnect = true);
cell AMX_NATIVE_CALL Native::mysql_connect(AMX* amx, cell* params) {
	char
		*host = NULL, 
		*user = NULL, 
		*db = NULL, 
		*pass = NULL;

	amx_StrParam(amx, params[1], host);
	amx_StrParam(amx, params[2], user);
	amx_StrParam(amx, params[3], db);
	amx_StrParam(amx, params[4], pass);

	unsigned int port = params[5];
	bool AutoReconn = !!(params[6]);

	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_connect", "host: \"%s\", user: \"%s\", database: \"%s\", password: \"****\", port: %d, autoreconnect: %s", host, user, db, port, AutoReconn == true ? "true" : "false");

	if(host == NULL || user == NULL || db == NULL) {
		CLog::Get()->LogFunction(LOG_ERROR, "mysql_connect", "empty connection data specified");
		return 0;
	}
	
	CMySQLHandle *Handle = CMySQLHandle::Create(host, user, pass != NULL ? pass : "", db, port, AutoReconn);
	Handle->GetMainConnection()->Connect();
	Handle->GetQueryConnection()->Connect();
	return static_cast<cell>(Handle->GetID());
}

//native mysql_close(connectionHandle = 1, bool:wait = true);
cell AMX_NATIVE_CALL Native::mysql_close(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	bool wait = !!params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_close", "connection: %d, wait: %s", cID, wait == true ? "true" : "false");

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_close", cID);
		return 0;
	}
	
	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);
	
	if(wait == true)
		Handle->WaitForQueryExec();

	Handle->GetMainConnection()->Disconnect();
	Handle->GetQueryConnection()->Disconnect();
	Handle->Destroy();
	return 1;
}

//native mysql_reconnect(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_reconnect(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_reconnect", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_reconnect", cID);
		return 0;
	}
	
	cell ReturnVal = 0;
	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);
	Handle->GetMainConnection()->Disconnect();
	Handle->GetMainConnection()->Connect();

	//wait until all threaded queries are executed, then reconnect query connection
	Handle->WaitForQueryExec();
	Handle->GetQueryConnection()->Disconnect();
	Handle->GetQueryConnection()->Connect();

	return 1;
}

//native mysql_unprocessed_queries(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_unprocessed_queries(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_unprocessed_queries", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_unprocessed_queries", cID);
		return 0;
	}

	return static_cast<cell>(CMySQLHandle::GetHandle(cID)->GetUnprocessedQueryCount());
}

//native mysql_tquery(conhandle, query[], callback[], format[], {Float,_}:...);
cell AMX_NATIVE_CALL Native::mysql_tquery(AMX* amx, cell* params) {
	static const int ConstParamCount = 4;
	unsigned int cID = params[1];

	char 
		*tmpQuery = NULL,
		*tmpCBName = NULL,
		*tmpParamFormat = NULL;
	amx_StrParam(amx, params[2], tmpQuery);
	amx_StrParam(amx, params[3], tmpCBName);
	amx_StrParam(amx, params[4], tmpParamFormat);

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		string ShortenQuery(tmpQuery == NULL ? "" : tmpQuery);
		ShortenQuery.resize(64);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", "connection: %d, query: \"%s\", callback: \"%s\", format: \"%s\"", cID, ShortenQuery.c_str(), tmpCBName, tmpParamFormat);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_tquery", cID);
		return 0;
	}

	if(tmpParamFormat != NULL && strlen(tmpParamFormat) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "mysql_tquery", "callback parameter count does not match format specifier length"), 0;

	CMySQLHandle *ConnHandle = CMySQLHandle::GetHandle(cID);
	CMySQLQuery *Query = CMySQLQuery::Create(tmpQuery, ConnHandle, tmpCBName, tmpParamFormat);
	if(Query != NULL) {
		if(Query->Callback->Name.length() > 0)
			Query->Callback->FillCallbackParams(amx, params, ConstParamCount);
	
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenQuery(Query->Query);
			if(ShortenQuery.length() > 512)
				ShortenQuery.resize(512);
			CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", "scheduling query \"%s\"..", ShortenQuery.c_str());
		}

		ConnHandle->ScheduleQuery(Query);
	}
	return 1;
}


//native Cache:mysql_query(conhandle, query[], bool:use_cache = true);
cell AMX_NATIVE_CALL Native::mysql_query(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	char *tmpQuery = NULL;
	amx_StrParam(amx, params[2], tmpQuery);
	bool UseCache = !!params[3];

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		string ShortenQuery(tmpQuery == NULL ? "" : tmpQuery);
		ShortenQuery.resize(64);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_query", "connection: %d, query: \"%s\", use_cache: %s", cID, ShortenQuery.c_str(), UseCache == true ? "true" : "false");
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_query", cID);
		return 0;
	}

	int StoredResultID = 0;
	CMySQLHandle *ConnHandle = CMySQLHandle::GetHandle(cID);
	CMySQLQuery *Query = CMySQLQuery::Create(tmpQuery, ConnHandle, NULL, NULL, false);
	if(Query != NULL) {
		Query->Execute();

		if(UseCache == true) {
			//first we set this result as active
			ConnHandle->SetActiveResult(Query->Result);
			//now we can save the result if we want
			StoredResultID = ConnHandle->SaveActiveResult();
			Query->Result = NULL;
		}

		Query->Destroy();
	}
	return static_cast<cell>(StoredResultID);
}


// native mysql_format(connectionHandle, output[], len, format[], {Float,_}:...);
cell AMX_NATIVE_CALL Native::mysql_format(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	size_t DestLen = (size_t)params[3];
	char *Format = NULL;
	amx_StrParam(amx, params[4], Format);

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		string ShortenFormat(Format == NULL ? "" : Format);
		if(ShortenFormat.length() > 128) {
			ShortenFormat.erase(128, ShortenFormat.length());
			ShortenFormat.append("...");
		}
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_format", "connection: %d, len: %d, format: \"%s\"", cID, DestLen, ShortenFormat.c_str());
	}

	if(Format == NULL)
		return 0;

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_format", cID);
		return 0;
	}
	
	CMySQLHandle *ConnHandle = CMySQLHandle::GetHandle(cID);

	char *Output = (char *)calloc(DestLen * 2, sizeof(char)); //*2 just for safety, what if user specified wrong DestLen?
	char *OrgOutput = Output;

	const unsigned int FirstParam = 5;
	unsigned int NumArgs = (params[0] / sizeof(cell));
	unsigned int NumDynArgs = NumArgs - (FirstParam - 1);
	unsigned int ParamCounter = 0;

	for( ; *Format != '\0'; ++Format) {
		
		if(strlen(OrgOutput) >= DestLen) {
			CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "destination size is too small");
			break;
		}
		
		if(*Format == '%') {
			++Format;

			if(*Format == '%') {
				*Output = '%';
				++Output;
				continue;
			}

			if(ParamCounter >= NumDynArgs) {
				CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "no value for specifier \"%%%c\" available", *Format);
				continue;
			}

			bool SpaceWidth = true;
			int Width = -1;
			int Precision = -1;
			
			if(*Format == '0') {
				SpaceWidth = false;
				++Format;
			}
			if(*Format > '0' && *Format <= '9') {
				Width = 0;
				while(*Format >= '0' && *Format <= '9') {
					Width *= 10;
					Width += *Format - '0';
					++Format;
				}
			}

			if(*Format == '.') {
				++Format;
				Precision = *Format - '0';
				++Format;
			}

			cell *AddressPtr = NULL;
			amx_GetAddr(amx, params[FirstParam + ParamCounter], &AddressPtr);
			
			switch (*Format) {
				case 'i': 
				case 'I':
				case 'd': 
				case 'D':
				{
					char NumBuf[13];
					ConvertIntToStr<10>(*AddressPtr, NumBuf);
					size_t NumBufLen = strlen(NumBuf);
					for(int len = (int)NumBufLen; Width > len; ++len) {
						if(SpaceWidth == true)
							*Output = ' ';
						else
							*Output = '0';
						++Output;
					}
					
					for(size_t c=0; c < NumBufLen; ++c) {
						*Output = NumBuf[c];
						++Output;
					}
					break;
				}
				case 'z': 
				case 'Z':
				case 's': 
				case 'S':
				{
					char *StrBuf = NULL;
					amx_StrParam(amx, params[FirstParam + ParamCounter], StrBuf);
					if(StrBuf != NULL) {
						for(size_t c=0, len = strlen(StrBuf); c < len; ++c) {
							*Output = StrBuf[c];
							++Output;
						}
					}
					
					break;
				}
				case 'f':
				case 'F':
				{
					float FloatVal = amx_ctof(*AddressPtr);
					char 
						FloatBuf[84+1], 
						SpecBuf[13];

					ConvertIntToStr<10>((int)floor(FloatVal), FloatBuf);
					for(int len = (int)strlen(FloatBuf); Width > len; ++len) {
						if(SpaceWidth == true)
							*Output = ' ';
						else
							*Output = '0';
						++Output;
					}

					if(Precision <= 6 && Precision >= 0)
						sprintf(SpecBuf, "%%.%df", Precision);
					else
						sprintf(SpecBuf, "%%f");
					
					sprintf(FloatBuf, SpecBuf, FloatVal);

					for(size_t c=0, len = strlen(FloatBuf); c < len; ++c) {
						*Output = FloatBuf[c];
						++Output;
					}
					break;
				}
				case 'e': 
				case 'E':
				{
					char *StrBuf = NULL;
					amx_StrParam(amx, params[FirstParam + ParamCounter], StrBuf);
					if(StrBuf != NULL) {
						string EscapedStr;
						ConnHandle->GetMainConnection()->EscapeString(StrBuf, EscapedStr);

						for(size_t c=0, len = EscapedStr.length(); c < len; ++c) {
							*Output = EscapedStr.at(c);
							++Output;
						}
					}
					break;
				}
				case 'X':
				{
					char HexBuf[17];
					memset(HexBuf, 0, 17);
					ConvertIntToStr<16>(*AddressPtr, HexBuf);

					for(size_t c=0, len = strlen(HexBuf); c < len; ++c) {
						if(HexBuf[c] >= 'a' && HexBuf[c] <= 'f')
							HexBuf[c] = toupper(HexBuf[c]);

						*Output = HexBuf[c];
						++Output;
					}

					break;
				}
				case 'x':
				{
					char HexBuf[17];
					memset(HexBuf, 0, 17);
					ConvertIntToStr<16>(*AddressPtr, HexBuf);

					for(size_t c=0, len = strlen(HexBuf); c < len; ++c) {
						*Output = HexBuf[c];
						++Output;
					}
					break;
				}
				case 'b':
				case 'B':
				{
					char BinBuf[33];
					memset(BinBuf, 0, 33);
					ConvertIntToStr<2>(*AddressPtr, BinBuf);

					for(size_t c=0, len = strlen(BinBuf); c < len; ++c) {
						*Output = BinBuf[c];
						++Output;
					}
					break;
				}
				default:
					CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "invalid format specifier \"%%%c\"", *Format);

			}
			ParamCounter++;
		}
		else {
			*Output = *Format;
			++Output;
		}
	}
	
	*Output = '\0';
	amx_SetCString(amx, params[2], OrgOutput, DestLen);
	free(OrgOutput);
	return static_cast<cell>(Output-OrgOutput);
}

//native mysql_set_charset(charset[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_set_charset(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	char *CharSet = NULL;
	amx_StrParam(amx, params[1], CharSet);
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_set_charset", "charset: \"%s\", connection: %d", CharSet, cID);

	if(CharSet == NULL)
		return 0;

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_set_charset", cID);
		return 0;
	}

	mysql_set_character_set(CMySQLHandle::GetHandle(cID)->GetMainConnection()->GetMySQLPointer(), CharSet);

	return 1;
}

//native mysql_get_charset(destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_get_charset(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_get_charset", "connection: %d, max_len: %d", cID, params[3]);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_get_charset", cID);
		return 0;
	}

	const char *CharSet = mysql_character_set_name(CMySQLHandle::GetHandle(cID)->GetMainConnection()->GetMySQLPointer());

	amx_SetCString(amx, params[1], CharSet == NULL ? "NULL" : CharSet, params[3]);
	return 1;
}

//native mysql_escape_string(const source[], destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_escape_string(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	char *Source = NULL;
	amx_StrParam(amx, params[1], Source);
	size_t DestLen = params[4];

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		string ShortenSource(Source == NULL ? "" : Source);
		if(ShortenSource.length() > 128) {
			ShortenSource.erase(128, ShortenSource.length());
			ShortenSource.append("...");
		}
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_escape_string", "source: \"%s\", connection: %d, max_len: %d", ShortenSource.c_str(), cID, params[4]);
	}

	if(Source == NULL)
		return 0;

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_escape_string", cID);
		return 0;
	}
	
	if(strlen(Source) > DestLen) {
		CLog::Get()->LogFunction(LOG_ERROR, "mysql_escape_string", "destination size is too small (must be at least as big as source)");
		return 0;
	}

	string EscapedStr;
	CMySQLHandle::GetHandle(cID)->GetMainConnection()->EscapeString(Source, EscapedStr);

	amx_SetCString(amx, params[2], EscapedStr.c_str(), params[4]);
	return static_cast<cell>(EscapedStr.length());
}

//native mysql_stat(destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_stat(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_stat", "connection: %d, max_len: %d", cID, params[3]);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_stat", cID);
		return 0;
	}
	
	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);
	const char *Stats = mysql_stat(Handle->GetMainConnection()->GetMySQLPointer());

	amx_SetCString(amx, params[1], Stats == NULL ? "NULL" : Stats, params[3]);
	return 1;
}

//native mysql_errno(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_errno(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_errno", "connection: %d", cID);

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_errno", cID);
		return 0;
	}

	return static_cast<cell>(mysql_errno(CMySQLHandle::GetHandle(cID)->GetMainConnection()->GetMySQLPointer()));
}

//native mysql_log(loglevel, logtype);
cell AMX_NATIVE_CALL Native::mysql_log(AMX* amx, cell* params) {
	if(params[1] < 0)
		return 0;

	CLog::Get()->SetLogLevel(params[1]);
	CLog::Get()->SetLogType(params[2]);
	return 1;
}
