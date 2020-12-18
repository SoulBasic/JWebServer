#include "Timer.h"

WheelTimer::WheelTimer(int wheel_size = 60):_now(0)
{
	if (wheel_size <= 0)wheel_size = 1;
	_wheel_size = wheel_size;
	_wheel = std::vector<std::list<timer_struct*>>(wheel_size, std::list<timer_struct*>());
}

WheelTimer::~WheelTimer()
{
	for (auto list : _wheel)
	{
		for (auto t : list)
		{
			if (t != nullptr)
			{
				delete t;
			}
		}
	}
}
void WheelTimer::addTimer(timer_struct* timer)
{
	auto itr = _place_map.find(timer->epfd);
	int place = _now - 1;
	if (place == -1)place = 59;
	if (itr == _place_map.end())
	{
		timer->place = place;
		_wheel[place].push_back(timer);
		_place_map[timer->epfd] = place;
		LOG_DEBUG("创建新定时器 fd=%d place=%d", timer->epfd, timer->place);
	}
	else if((*itr).second == place){}
	else
	{
		auto& list = _wheel[(*itr).second];
		if (list.size() > 0)
		{
			for (auto it = list.begin(); it != list.end();)
			{
				if ((*it)->epfd == timer->epfd)
				{
					auto temp = *it;
					it = list.erase(it);
					delete temp;
					//LOG_DEBUG("用户%d更新定时器", timer->epfd);
					break;
				}
				else it++;
			}
		}
		_place_map.erase(itr);
		timer->place = place;
		_wheel[place].push_back(timer);
		_place_map[timer->epfd] = place;
	}

}



void WheelTimer::tick()
{
	_now++;
	if (_now >= _wheel_size || _now < 0)_now = 0;
	if(_wheel[_now].size() > 0)
	{
		LOG_DEBUG("tick=%d 中有%d个过期fd，进行清理",_now,_wheel[_now].size());
		for (auto t : _wheel[_now])
		{
			if (t != nullptr)
			{
				t->callback();
				delete t;
			}
		}

		_wheel[_now].clear();
	}

}