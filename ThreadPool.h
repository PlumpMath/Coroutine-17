#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "Coroutine.h"

class ThreadPool
{
	public:
		typedef boost::shared_ptr<ThreadPool> Ptr;
		typedef boost::function<void ()> SpawnFunc_t;

	public:
		ThreadPool() : _pWork(new boost::asio::io_service::work(_io)) {}
		virtual void Init(int threadCount);
		template<typename Work>
			void Post(const Work& work)
		{
			_io.post(work);
		}
		virtual void Spawn(SpawnFunc_t callFunc) {}
		void Join();
		void Stop();
		boost::asio::io_service& getService();
		virtual ~ThreadPool() {}
	protected:
		boost::asio::io_service _io;
		boost::shared_ptr<boost::asio::io_service::work> _pWork;
		std::vector<boost::shared_ptr<boost::thread> > _threads;
};

class SingleThreadPool : public ThreadPool
{
	public:
		typedef boost::shared_ptr<SingleThreadPool> Ptr;
		typedef std::vector<Ptr> PtrList;

	public:
		virtual void Init(int threadCount = 1);
		virtual void Spawn(SpawnFunc_t callFunc);

	protected:
		bool _spawn(SpawnFunc_t callFunc);
};


#endif
