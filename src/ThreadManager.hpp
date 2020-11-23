#pragma once
#ifndef _THREAD_MANAGER_HPP_
#define _THREAD_MANAGER_HPP_

#include <thread>
#include "global.hpp"
#include <queue>
#include <condition_variable>
class ThreadManager
{
private:
	std::mutex _mtx;
	std::queue<std::function<void()>> _tasks;
	std::condition_variable _cond;
	bool running;
public:
	ThreadManager(size_t threadCount = 4)
	{
		if (threadCount <= 0)LOG_ERROR("线程池中可用线程数量小于等于0");
		running = true;
		for (size_t i = 0; i < threadCount; i++)
		{
			std::thread(onRun,this).detach();
		}
		
	}
	~ThreadManager()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		running = false;
		_cond.notify_all();
	}

	static void onRun(ThreadManager* ts)
	{
		std::unique_lock<std::mutex> locker(ts->_mtx);
		while (ts->running)
		{
			if (!ts->_tasks.empty())
			{
				auto task = std::move(ts->_tasks.front());
				ts->_tasks.pop();
				locker.unlock();
				task();
				locker.lock();
			}
			else ts->_cond.wait(locker);
		}
	}


	template<typename Func>
	void addTask(Func&& task)
	{
		{
			std::lock_guard<std::mutex> locker(_mtx);
			_tasks.emplace(std::forward<Func>(task));
		}
		_cond.notify_one();
	}

};

#endif // !_THREAD_MANAGER_HPP_

