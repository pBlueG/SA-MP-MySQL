#include "CResult.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <mysql.h>
#else
	#include <mysql/mysql.h>
#endif

const char *CResult::GetFieldName(unsigned int idx)
{
	return (idx < m_Fields) ? m_FieldNames.at(idx).c_str() : nullptr;
}

const char *CResult::GetRowData(unsigned int row, unsigned int fieldidx)
{
	return (row < m_Rows && fieldidx < m_Fields) ? m_Data[row][fieldidx] : nullptr;
}

const char *CResult::GetRowDataByName(unsigned int row, const char *field)
{
	if(row >= m_Rows || m_Fields == 0)
		return nullptr;
	
	if (field == nullptr)
		return nullptr;
	

	for(unsigned int i = 0; i < m_Fields; ++i) 
		if(strcmp(m_FieldNames.at(i).c_str(), field) == 0) 
			return m_Data[row][i];
	
	return nullptr;
}
