#pragma once
#ifndef INC_CCALLBACK_H
#define INC_CCALLBACK_H


#include <list>
#include <stack>
#include <string>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/variant.hpp>
#include <boost/unordered_set.hpp>

using std::list;
using std::stack;
using std::string;
using boost::tuple;
using boost::shared_future;
using boost::mutex;
using boost::unordered_set;

#include "main.h"
#include "CMySQLQuery.h"


class CMySQLHandle;


class CCallback 
{
private:
	static CCallback *m_Instance;


	list< tuple<shared_future<CMySQLQuery>, CMySQLHandle*> > m_CallbackQueue;
	mutex m_QueueMtx;

	unordered_set<AMX *> m_AmxList;

	CCallback() { }
	~CCallback() { }

public:
	static inline CCallback *Get()
	{
		return m_Instance;
	}
	static inline void Destroy()
	{
		delete m_Instance;
		m_Instance = NULL;
	}


	void FillCallbackParams(stack< boost::variant<cell, string> > &dest, const char *format, AMX* amx, cell* params, const int ConstParamCount);


	inline void AddQueryToQueue(shared_future<CMySQLQuery> fut, CMySQLHandle *handle)
	{
		boost::mutex::scoped_lock LockGuard(m_QueueMtx);
		m_CallbackQueue.push_back(make_tuple(boost::move(fut), handle));
	}

	inline void AddAmx(AMX *amx)
	{
		m_AmxList.insert(amx);
	}
	inline void EraseAmx(AMX *amx)
	{
		m_AmxList.erase(amx);
	}


	void ProcessCallbacks();
};


#endif // INC_CCALLBACK_H
