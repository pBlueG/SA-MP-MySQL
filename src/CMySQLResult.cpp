#pragma once

#include "CLog.h"
#include "CMySQLResult.h"


void CMySQLResult::GetFieldName(unsigned int idx, char **dest) 
{
	if (idx < m_Fields) 
	{
		(*dest) = const_cast<char*>(m_FieldNames.at(idx).c_str());

		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetFieldName", "index: '%d', name: \"%s\"", idx, *dest);
	}
	else 
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetFieldName", "invalid field index ('%d')", idx);
}

void CMySQLResult::GetRowData(unsigned int row, unsigned int fieldidx, char **dest) 
{
	if(row < m_Rows && fieldidx < m_Fields) 
	{
		(*dest) = m_Data[row][fieldidx];

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) 
		{
			string ShortenDest((*dest) != NULL ? (*dest) : "NULL");
			if(ShortenDest.length() > 1024)
				ShortenDest.resize(1024);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowData", "row: '%d', field: '%d', data: \"%s\"", row, fieldidx, ShortenDest.c_str());
		}
	}
	else 
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowData", "invalid row ('%d') or field index ('%d')", row, fieldidx);
}

void CMySQLResult::GetRowDataByName(unsigned int row, const char *field, char **dest) 
{
	if(row >= m_Rows || m_Fields == 0)
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "CMySQLResult::GetRowDataByName()", "invalid row index ('%d')", row);
	
	if(field == NULL)
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "CMySQLResult::GetRowDataByName()", "empty field name specified");

	if (dest == NULL)
		return (void)CLog::Get()->LogFunction(LOG_ERROR, "CMySQLResult::GetRowDataByName()", "invalid destination specified");

	for(unsigned int i = 0; i < m_Fields; ++i) 
	{
		if(::strcmp(m_FieldNames.at(i).c_str(), field) == 0) 
		{
			(*dest) = m_Data[row][i];

			if(CLog::Get()->IsLogLevel(LOG_DEBUG)) 
			{
				string ShortenDest((*dest) != NULL ? (*dest) : "NULL");
				if(ShortenDest.length() > 1024)
					ShortenDest.resize(1024);
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowDataByName", "row: '%d', field: \"%s\", data: \"%s\"", row, field, ShortenDest.c_str());
			}

			return ;
		}
	}
	CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowDataByName", "field not found (\"%s\")", field);
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
	if (m_Data != NULL)
	free(m_Data);

	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::~CMySQLResult()", "deconstructor called");
}
