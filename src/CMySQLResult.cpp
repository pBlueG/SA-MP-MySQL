#pragma once

#include "CLog.h"
#include "CMySQLResult.h"
#include "CMySQLHandle.h"


const char *CMySQLResult::GetFieldName(unsigned int idx)
{
	if (idx < m_Fields) 
	{
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetFieldName", "index: '%d', name: \"%s\"", idx, m_FieldNames.at(idx).c_str());
		return m_FieldNames.at(idx).c_str();
	}
	else
	{
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetFieldName", "invalid field index ('%d')", idx);
		return NULL;
	}
}

const char *CMySQLResult::GetRowData(unsigned int row, unsigned int fieldidx)
{
	if(row < m_Rows && fieldidx < m_Fields) 
	{
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) 
		{
			string ShortenDest(m_Data[row][fieldidx] != NULL ? m_Data[row][fieldidx] : "NULL");
			if (MySQLOptions.Log_TruncateData && ShortenDest.length() > 1024)
				ShortenDest.resize(1024);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowData", "row: '%d', field: '%d', data: \"%s\"", row, fieldidx, ShortenDest.c_str());
			
		}

		return m_Data[row][fieldidx];
	}
	else
	{
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowData", "invalid row ('%d') or field index ('%d')", row, fieldidx);
		return NULL;
	}
}

const char *CMySQLResult::GetRowDataByName(unsigned int row, const char *field) 
{
	if(row >= m_Rows || m_Fields == 0)
	{
		CLog::Get()->LogFunction(LOG_ERROR, "CMySQLResult::GetRowDataByName()", "invalid row index ('%d')", row);
		return NULL;
	}
	
	if(field == NULL)
	{
		CLog::Get()->LogFunction(LOG_ERROR, "CMySQLResult::GetRowDataByName()", "empty field name specified");
		return NULL;
	}

	for(unsigned int i = 0; i < m_Fields; ++i) 
	{
		if(::strcmp(m_FieldNames.at(i).c_str(), field) == 0) 
		{
			if(CLog::Get()->IsLogLevel(LOG_DEBUG)) 
			{
				string ShortenDest(m_Data[row][i] != NULL ? m_Data[row][i] : "NULL");
				if (MySQLOptions.Log_TruncateData && ShortenDest.length() > 1024)
					ShortenDest.resize(1024);
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowDataByName", "row: '%d', field: \"%s\", data: \"%s\"", row, field, ShortenDest.c_str());
			}

			return m_Data[row][i];
		}
	}
	CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowDataByName", "field not found (\"%s\")", field);
	return NULL;
}

CMySQLResult::CMySQLResult() :
	m_Fields(0),
	m_Rows(0),

	m_Data(NULL),

	m_InsertID(0),
	m_AffectedRows(0),
	m_WarningCount(0)
{
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::CMySQLResult()", "constructor called");
}

CMySQLResult::~CMySQLResult() 
{
	if(m_Data != NULL)
		free(m_Data);

	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::~CMySQLResult()", "deconstructor called");
}
