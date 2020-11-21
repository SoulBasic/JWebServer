#pragma once
#ifndef _EPOLL_MANAGER_HPP_
#define _EPOLL_MANAGER_HPP_
#include "global.hpp"
class EpollManager
{
public:
	explicit EpollManager(int maxEvents = 1024):_epfd(epoll_create(256)),_events(maxEvents)
	{
		if (_events.size() == 0)LOG_ERROR("max event����Ϊ0");
		if (_epfd < 0)LOG_ERROR("epfd���Ϸ�");
	}
	~EpollManager()
	{
		close(_epfd);
	}

	bool addFd(int fd, uint32_t events)
	{
		if (fd < 0)return false;
		epoll_event ev = {  };
		ev.data.fd = fd;
		ev.events = events;
		if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) == 0)return true;
		else
		{
			LOG_ERROR("epoll_ctl_add����");
			return false;
		}
	}

	bool modFd(int fd, uint32_t events) {
		if (fd < 0) return false;
		epoll_event ev = { 0 };
		ev.data.fd = fd;
		ev.events = events;
		if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev) == 0)return true;
		else
		{
			LOG_ERROR("epoll_ctl_mod����");
			return false;
		}
	}

	bool deleteFd(int fd)
	{
		if (fd < 0)return false;
		epoll_event ev = {};
		if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev) == 0)return true;
		else
		{
			LOG_ERROR("epoll_ctl_del����");
			return false;
		}
	}

	int getEventFd(int index)
	{
		if (index >= _events.size() || index < 0)return INVALID_SOCKET;
		return _events[index].data.fd;
	}


	uint32_t getEvents(int index)
	{
		if (index >= _events.size() || index < 0)return INVALID_SOCKET;
		return _events[index].events;
	}




private:
	int _epfd;
	std::vector<epoll_event> _events;
};

#endif // !_EPOLL_MANAGER_HPP_