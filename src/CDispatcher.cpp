#include "CDispatcher.hpp"


void CDispatcher::Dispatch(DispatchFunction_t &&func)
{
	std::lock_guard<std::mutex> lock_guard(m_QueueMtx);
	return m_Queue.push(std::move(func));
}

void CDispatcher::Process()
{
	std::lock_guard<std::mutex> lock_guard(m_QueueMtx);
	while (m_Queue.empty() == false)
	{
		m_Queue.front()();
		m_Queue.pop();
	}
}
