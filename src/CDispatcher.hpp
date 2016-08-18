#pragma once

#include "CSingleton.hpp"

#include <queue>
#include <mutex>

#include "types.hpp"


class CDispatcher : public CSingleton < CDispatcher >
{
	friend class CSingleton < CDispatcher >;
public: //type definitions

private: //constructor / destructor
	CDispatcher() = default;
	~CDispatcher() = default;

private: //variables
	std::queue<DispatchFunction_t> m_Queue;
	std::mutex m_QueueMtx;

public: //functions
	void Dispatch(DispatchFunction_t &&func);
	void Process();

};
