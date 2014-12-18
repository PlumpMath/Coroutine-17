#ifndef COROUTINE_H
#define COROUTINE_H
#include <exception>
#include <boost/circular_buffer.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/thread/tss.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

class NonResumableException: std::exception
{
	public:
		const char* what()
		{
			return "Cannot resume this coroutine";
		}
};
class CannotStartRunningCoroutine: std::exception
{
	public:
		const char* what()
		{
			return "Cannot start a running coroutine";
		}
};
class YieldNotInCoroutine: std::exception
{
	public:
		const char* what()
		{
			return "Yield outside a coroutine";
		}
};
struct CoroutineContext
{
	std::stringstream logBuff;
};
class Coroutine : public boost::enable_shared_from_this<Coroutine>
{
	public:
		typedef boost::shared_ptr<Coroutine> Ptr;
		static Ptr Create(int maxCount = 1000)
		{
			boost::is_function<BOOST_RV_REF(boost::rv<Co_t>)> noUse1;
			boost::is_function<BOOST_RV_REF(CoFun_t)> noUse2;
			return Ptr(new Coroutine(maxCount));
		}
		virtual ~Coroutine() 
		{
			Stop();
		}
		void Stop();
		typedef boost::function<void()> Task_t;
		virtual bool Post(Task_t task);
		void Resume();
		void Yielding();  // Yield is a macro in windef.h
		bool IsIdle()
		{
			return !m_co;
		}
		CoroutineContext& GetContext()
		{
			return m_context;
		}
		//void SetContext(const CoroutineContext& ctx)
		//{
		//	m_context = ctx;
		//}
	protected:
		typedef boost::coroutines::coroutine<void()> Co_t;
		typedef boost::function<void (Co_t::caller_type &)> CoFun_t;
	protected:
		Coroutine(int maxCount) : m_tasks(maxCount), m_stopped(false), m_ca(NULL) {}
		virtual void run(Co_t::caller_type& ca);
		bool start();		
	private:
		boost::circular_buffer<Task_t> m_tasks;
		bool               m_stopped;
		Co_t               m_co;
		Co_t::caller_type *m_ca;
		CoroutineContext   m_context;
};

class CoroutineFactory
{
	public:
		virtual ~CoroutineFactory()
		{
		}
		static CoroutineFactory &GetInstance()
		{
			static boost::thread_specific_ptr<CoroutineFactory> pFactory;
			if (!pFactory.get())
			{
				pFactory.reset(new CoroutineFactory());
			}
			return *pFactory;
		}
		Coroutine::Ptr GetCoroutine(int maxCount = 100);
		Coroutine::Ptr GetCurrentCoroutine()
		{
			return m_pCo;
		}
		void SetCurrentCoroutine(Coroutine::Ptr pCo = Coroutine::Ptr())
		{
			m_pCo = pCo;
		}
	private:
		Coroutine::Ptr     m_pCo;
};
#endif
