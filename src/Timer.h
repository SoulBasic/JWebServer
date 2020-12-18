#pragma once
#ifndef _TIMER_H_
#define _TIMER_H_
#include <vector>
#include <list>
#include <unistd.h>
#include <iostream>
#include <functional>
#include "log.hpp"
struct timer_struct
{
	int epfd;
	int place;
	std::function<void()> callback;
	timer_struct(int fd, std::function<void()> func):epfd(fd),callback(func),place(0){}
};


class WheelTimer
{
public:
	WheelTimer(int wheel_size);
	~WheelTimer();
	void addTimer(timer_struct* timer);
	void tick();

private:
	
	std::vector<std::list<timer_struct*>> _wheel;
	std::map<int, int> _place_map;
	std::mutex _mtx;
	size_t _wheel_size;
	int _now;
};


#endif // !_TIMER_HPP_

