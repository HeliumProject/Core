#include "Precompile.h"

#include <memory>
#include <math.h>

#include "Platform/Thread.h"

#include "gtest/gtest.h"


using namespace Helium;

const uint32_t LIGHT_WORKLOAD = 100;
const uint32_t MEDIUM_WORKLOAD = 1000000;
const uint32_t HEAVY_WORKLOAD = ~0;

int work(const uint32_t count)
{
	int foo = 0;
	for (uint32_t i = 0; i < count; i++)
	{
		foo += int(sqrt(i * 2));
		foo /= 2; int(sqrt(i * 2));
	}
	return foo;
}

int g_tmp = 0;

class Worker : public Runnable
{
	const uint32_t m_count;
	const uint32_t m_sleepMilliseconds = 0;
public:
	Worker(const uint32_t count) : m_count(count) {}

	Worker(const uint32_t count, const uint32_t sleepMilliseconds)
		: m_count(count),
		m_sleepMilliseconds(sleepMilliseconds)
	{}

	virtual ~Worker() = default;

	virtual void Run()
	{
		if (m_sleepMilliseconds)
		{
			Thread::Sleep(m_sleepMilliseconds);
		}

		g_tmp = work(m_count);
	}
};


TEST(PlatformThreadTest, InvalidThread)
{
	RunnableThread thread(nullptr);
	ASSERT_FALSE(thread.IsValid());
}

TEST(PlatformThreadTest, IsMainThread)
{
	ASSERT_TRUE(Thread::IsMain());
	ASSERT_TRUE(Thread::GetCurrentId() == Thread::GetMainId());
}

TEST(PlatformThreadTest, TryToRunNullWorkLoadThread)
{
	RunnableThread thread(nullptr);
	EXPECT_NO_THROW(thread.Start("TryToRunNullWorkLoadThread"));
	ASSERT_TRUE(thread.Join());
}

TEST(PlatformThreadTest, RunRunableThread)
{
	Worker runnableWorker(MEDIUM_WORKLOAD);
	RunnableThread thread(&runnableWorker);
	thread.Start("RunRunableThread");
	ASSERT_TRUE(thread.Join());
}

TEST(PlatformThreadTest, RunableThreadJoinTimeOut)
{
	Worker runnableWorker(MEDIUM_WORKLOAD);
	RunnableThread thread(&runnableWorker);
	thread.Start("RunableThreadJoinTimeOut");
	ASSERT_FALSE(thread.Join(1));
}

TEST(PlatformThreadTest, SleepRunableThread)
{
	const uint32_t sleepMilliseconds = 2;
	Worker runnableWorker(LIGHT_WORKLOAD, sleepMilliseconds);
	RunnableThread thread(&runnableWorker);
	thread.Start("SleepRunableThread");
	ASSERT_FALSE(thread.Join(sleepMilliseconds));
}

TEST(PlatformThreadTest, TryToJoinRunableThread)
{
	Worker runnableWorker(1);
	RunnableThread thread(&runnableWorker);
	thread.Start("TryToJoinRunableThread");
	Thread::Sleep(1);
	ASSERT_TRUE(thread.TryJoin());
}


#if HELIUM_ASSERT_ENABLED 

TEST(PlatformThreadTest, FailToSetRunableThread)
{
	Worker runnableWorker(MEDIUM_WORKLOAD);
	RunnableThread thread(&runnableWorker);
	thread.Start("foobar");
	ASSERT_DEATH(thread.SetRunnable(nullptr), "");
}

TEST(PlatformThreadTest, FailToCreateNullCallbackThread)
{
	CallbackThread cbThread;

	ASSERT_DEATH(cbThread.Create(nullptr, nullptr, ""), "");
}

TEST(PlatformThreadTest, FailToRunNullCallbackThread)
{
	CallbackThread cbThread;

	ASSERT_DEATH(cbThread.Run(), "");
}

#endif


class CallbackThreadWorker
{
public:
	CallbackThreadWorker() = default;
	virtual ~CallbackThreadWorker() = default;

	void DoWork()
	{
		work(LIGHT_WORKLOAD);
	}
};


TEST(PlatformThreadTest, RunCallbackThread)
{
	CallbackThread cbThread;
	CallbackThreadWorker cbWorker;

	CallbackThread::Entry entry = &CallbackThread::EntryHelper<CallbackThreadWorker, &CallbackThreadWorker::DoWork>;
	ASSERT_TRUE(cbThread.Create(entry, &cbWorker, "RunCallbackThread"));
	ASSERT_TRUE(cbThread.Join());
}


class TLSCallbackThreadWorker
{
	ThreadLocalPointer m_threadLocalPointer;
public:
	TLSCallbackThreadWorker() = default;
	virtual ~TLSCallbackThreadWorker() = default;

	void DoWork()
	{
		std::unique_ptr<int[]> uptr{ new int[LIGHT_WORKLOAD] };

		m_threadLocalPointer.SetPointer(uptr.get());

		int* arr = (int*)m_threadLocalPointer.GetPointer();

		for (uint32_t i = 0; i < LIGHT_WORKLOAD; i++)
		{
			arr[i] = int(sqrt(i * 2));
		}
	}
};


TEST(PlatformThreadTest, GetNullThreadLocalPointer)
{
	ThreadLocalPointer threadLocalPointer;

	ASSERT_FALSE(threadLocalPointer.GetPointer());
}

TEST(PlatformThreadTest, RunThreadLocalPointerWorker)
{
	CallbackThread cbThread;
	CallbackThread cbThread2;
	TLSCallbackThreadWorker cbWorker;

	CallbackThread::Entry entry = &CallbackThread::EntryHelper<TLSCallbackThreadWorker, &TLSCallbackThreadWorker::DoWork>;
	cbThread.Create(entry, &cbWorker, "RunCallbackThread");
	cbThread2.Create(entry, &cbWorker, "RunCallbackThread2");
	ASSERT_TRUE(cbThread.Join());
	ASSERT_TRUE(cbThread2.Join());
}