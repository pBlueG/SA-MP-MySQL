#pragma once

#include "CSingleton.h"

#include <queue>
#include <boost/thread/mutex.hpp>

#include "Types.h"


class CDispatcher : public CSingleton < CDispatcher >
{
	friend class CSingleton < CDispatcher > ;
public: //type definitions

private: //constructor / destructor
	CDispatcher() = default;
	~CDispatcher() = default;

private: //variables
	std::queue<DispatchFunction_t> m_Queue;
	boost::mutex m_QueueMtx;

public: //functions
	void Dispatch(DispatchFunction_t &&func);
	void Process();

};
