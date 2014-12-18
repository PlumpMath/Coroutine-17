#include "Coroutine.h"
#include <boost/bind.hpp>
#include <boost/assert.hpp>

Coroutine::Ptr CoroutineFactory::GetCoroutine(int maxCount)
{
	return Coroutine::Create(maxCount);
}

bool Coroutine::Post(Task_t task)
{
	if (m_tasks.full())
	{
		return false;
	}
	m_tasks.push_back(task);
	if (!m_co)
	{
		return start();
	}
	return true;
}
void Coroutine::Resume()
{
	BOOST_ASSERT(m_co);
	if (!m_co)
	{
		throw NonResumableException();
	}
	BOOST_ASSERT(m_ca && *m_ca);
	Coroutine::Ptr current = CoroutineFactory::GetInstance().GetCurrentCoroutine();
	CoroutineFactory::GetInstance().SetCurrentCoroutine(shared_from_this());
	m_co();
	CoroutineFactory::GetInstance().SetCurrentCoroutine(current);
}
void Coroutine::Yielding()
{
	BOOST_ASSERT(m_ca && *m_ca);
	if (!m_ca || !*m_ca)
	{
			throw YieldNotInCoroutine();
	}
	CoroutineFactory::GetInstance().SetCurrentCoroutine(Coroutine::Ptr());
	(*m_ca)();
}
void Coroutine::run(Co_t::caller_type& ca)
{
	m_ca = &ca;
	while(!m_tasks.empty())
	{
		Task_t task = m_tasks.front();
		task();
		m_tasks.pop_front();
	}
}
void Coroutine::Stop()
{
	m_ca = NULL;
}
bool Coroutine::start()
{
	BOOST_ASSERT(!m_co);
			if (m_co)
			{
					throw CannotStartRunningCoroutine();
			}
	try
	{
		Coroutine::Ptr current = CoroutineFactory::GetInstance().GetCurrentCoroutine();
		CoroutineFactory::GetInstance().SetCurrentCoroutine(shared_from_this());
		// shared_from_this() is not used here because
		// 1) it's not needed to keep the coroutine resumable after pCo is deleted. It is not resumable through pCo->Resume() anyway.
		// 2) it's easier to avoid memory leak. Otherwise, we need to call m_co.swap(Co_t()) whenenver m_co is finished.
		CoFun_t runFunc = boost::bind(&Coroutine::run, this, _1);
		m_co = Co_t(runFunc);
		CoroutineFactory::GetInstance().SetCurrentCoroutine(current);
		return true;
	}
	catch(std::exception& e)
	{
		// usually only bad alloc is thrown. It means there's no enough virtual memory space.
		return false;
	}
}
