#pragma once
#ifndef INC_CCALLBACK_H
#define INC_CCALLBACK_H


#include <stack>
#include <string>
#include <boost/variant.hpp>
#include <set>
#include <boost/lockfree/queue.hpp>

using std::stack;
using std::string;
using std::set;

#include "main.h"


class CMySQLHandle;
class CMySQLQuery;


class CCallback 
{
private:
	static CCallback *m_Instance;


	boost::lockfree::queue<
		CMySQLQuery *,
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<32648>
	> m_CallbackQueue;

	set<AMX *> m_AmxList;

	CCallback() { }
	~CCallback() { }

public:
	static inline CCallback *Get()
	{
		return m_Instance;
	}
	inline void Destroy()
	{
		m_Instance = NULL;
		delete this;
	}


	void FillCallbackParams(stack< boost::variant<cell, string> > &dest, const char *format, AMX* amx, cell* params, const int ConstParamCount);


	inline void QueueQuery(CMySQLQuery *query)
	{
		m_CallbackQueue.push(query);
	}

	inline void AddAmx(AMX *amx)
	{
		m_AmxList.insert(amx);
	}
	inline void EraseAmx(AMX *amx)
	{
		m_AmxList.erase(amx);
	}

	void ClearByHandle(CMySQLHandle *handle);

	void ProcessCallbacks();
};


#endif // INC_CCALLBACK_H
