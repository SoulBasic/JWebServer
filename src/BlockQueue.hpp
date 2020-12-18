#pragma once
#ifndef _BLOCKQUEUE_HPP_
#define _BLOCKQUEUE_HPP_

#include "global.hpp"
#include <condition_variable>
#include <deque>

template <class Item>
class BlockQueue
{
public:
	BlockQueue(size_t max_size):_running(true)
	{
		if (max_size <= 0)max_size = 1;
		_max_size = max_size;
		
	}

	~BlockQueue()
	{
		terminal();
	}


	void terminal()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		_deque.clear();
		_running = false;
		_producer.notify_all();
		_consumer.notify_all();
	}

	void clear()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		_deque.clear();
	}
	Item front()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		return _deque.front();
	}
	Item back()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		return _deque.back();
	}
	size_t size()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		return _deque.size();
	}
	inline size_t max_size() { return _max_size; }
	bool empty()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		return _deque.empty();
	}
	bool is_full()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		return _deque.size() >= _max_size;
	}
	void push_back(const Item& item)
	{
		std::unique_lock<std::mutex> locker(_mtx);
		while (_deque.size() >= _max_size)
		{
			_producer.wait(locker);
		}
		_deque.push_back(item);
		_consumer.notify_one();
	}

	void push_front(const Item& item)
	{
		std::unique_lock<std::mutex> locker(_mtx);
		while (_deque.size() >= _max_size)
		{
			_producer.wait(locker);
		}
		_deque.push_front(item);
		_consumer.notify_one();
	}

	bool take(Item& item)
	{
		std::unique_lock<std::mutex> locker(_mtx);
		while (_deque.empty())
		{
			_consumer.wait(locker);
			if (!_running)return false;
		}
		item = _deque.front();
		_deque.pop_front();
		_producer.notify_one();
		return true;
	}
	bool take(Item& item, int timeout_milliseconds)
	{
		std::unique_lock<std::mutex> locker(_mtx);
		while (_deque.empty())
		{
			if (_consumer.wait_for(locker, std::chrono::milliseconds(timeout_milliseconds)) == std::cv_status::timeout)return false;
			if (!_running)return false;
		}
		item = _deque.front();
		_deque.pop_front();
		_producer.notify_one();
		return true;
	}
	void flush() {
		_consumer.notify_one();
	};


private:
	std::deque<Item> _deque;
	size_t _max_size;
	std::mutex _mtx;
	std::condition_variable _consumer;
	std::condition_variable _producer;
	bool _running;
};


#endif // _BLOCKQUEUE_HPP_
