#pragma once
#ifndef INC_CCALLBACK_H
#define INC_CCALLBACK_H


#include <list>
#include <stack>
#include <string>
//#include <boost/lockfree/queue.hpp>
#include <queue>
#include <tuple>//#include <boost/tuple/tuple.hpp>
#include <boost/thread/future.hpp>

using std::list;
using std::stack;
using std::string;
using std::queue;

#include "main.h"


class CMySQLQuery;
class CMySQLHandle;


class CCallback 
{
public:

	static void FillCallbackParams(stack<string> &dest, string &format, AMX* amx, cell* params, const int ConstParamCount);

	CCallback() :
		IsInline(false)
	{}
	~CCallback() {}


	stack<string> Parameters;
	string Name;
	string ParamFormat;
	bool IsInline;

	
	static void ProcessCallbacks();
	
	/*static inline void AddQueryToQueue(CMySQLQuery *cb) 
	{
		m_CallbackQueue.push(cb);
	}
	static inline boost::unique_future<void> GetNextQuery() 
	{
		CMySQLQuery *NextQuery = NULL;
		m_CallbackQueue.pop(NextQuery);
		return NextQuery;
		//return m_CallbackQueue.pop();
	}*/

	static void AddAmx(AMX *amx);
	static void EraseAmx(AMX *amx);

	static void ClearAll();

//private:
	/*static boost::lockfree::queue<
			CMySQLQuery*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<8192>
		> m_CallbackQueue;*/
	//static queue<boost::unique_future<void>> m_CallbackQueue;
	static list<std::tuple<boost::unique_future<CMySQLQuery>, CMySQLHandle*>> m_CallbackQueue;


	static list<AMX *> m_AmxList;
};


#endif // INC_CCALLBACK_H
