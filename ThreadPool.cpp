#include "ThreadPool.h"

void ThreadPool::Init(int threadCount)
{
	_threads.resize(threadCount);
	for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = _threads.begin();
			it != _threads.end(); it++)
	{
		it -> reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &_io)));
	}
}


void ThreadPool::Join()
{
	for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = _threads.begin();
			it != _threads.end(); it++)
	{
		(*it) -> join();
	}
}

boost::asio::io_service& ThreadPool::getService()
{
	return _io;
}

void ThreadPool::Stop()
{
	_io.stop();
	_pWork.reset();
	for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = _threads.begin();
			it != _threads.end(); it++)
	{
		if (*it)
		{
			it -> reset();
		}
	}
}

void SingleThreadPool::Init(int threadCount)
{
	_threads.resize(1);
	for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = _threads.begin();
			it != _threads.end(); it++)
	{
		it -> reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &_io)));
	}
}

void SingleThreadPool::Spawn(SpawnFunc_t callFunc)
{
	Post(boost::bind(&SingleThreadPool::_spawn, this, callFunc));
}

bool SingleThreadPool::_spawn(SpawnFunc_t callFunc)
{
	Coroutine::Ptr m_pCo = CoroutineFactory::GetInstance().GetCoroutine(10);
	assert(m_pCo);
	if (m_pCo)
	{
		bool res = m_pCo->Post(callFunc);
		if (!res)
		{
			std::cout << "task queue is full and this message is dropped" << std::endl;
		}
		return res;
	}
	std::cout << "cannot get coroutine" << std::endl;
	return false;
}
