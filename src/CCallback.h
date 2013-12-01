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

using std::list;
using std::stack;
using std::string;
using boost::tuple;
using boost::shared_future;
using boost::mutex;

#include "main.h"
#include "CMySQLQuery.h"


class CMySQLHandle;


class CCallback 
{
private:
	static list< tuple<shared_future<CMySQLQuery>, CMySQLHandle*> > m_CallbackQueue;
	static mutex m_QueueMtx;

	static list<AMX *> m_AmxList;

public:

	static void FillCallbackParams(stack< boost::variant<cell, string> > &dest, const char *format, AMX* amx, cell* params, const int ConstParamCount);

	
	static void ProcessCallbacks();
	
	static inline void AddQueryToQueue(shared_future<CMySQLQuery> fut, CMySQLHandle *handle)
	{
		boost::mutex::scoped_lock LockGuard(m_QueueMtx);
		m_CallbackQueue.push_back(make_tuple(boost::move(fut), handle));
	}

	static void AddAmx(AMX *amx);
	static void EraseAmx(AMX *amx);

	static void ClearAll();

};


#endif // INC_CCALLBACK_H
