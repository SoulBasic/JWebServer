#pragma once
#ifndef _HTTPSERVER_HPP_
#define _HTTPSERVER_HPP_

#include "global.hpp"
#include "EpollManager.hpp"
#define SERVER_ERROR -1
#define SERVER_SUCCESS 0

class HttpServer
{
private:
	SOCKET _ssock;
	sockaddr_in _ssin;
	uint32_t _listenEvent;
	char* root;
	std::unique_ptr<EpollManager> _epollManager;
public:
	HttpServer(uint32_t listenEvent = EPOLLET) :_ssock(INVALID_SOCKET), _ssin({}),_listenEvent(listenEvent), _epollManager(new EpollManager()) 
	{
	
		root = getcwd(nullptr, 256);
		strncat(root, "/html/", 16);

	}
	~HttpServer()
	{
		if (INVALID_SOCKET != _ssock)
		{
			close(_ssock);
			_ssock = INVALID_SOCKET;
		}
	}

	int initSocket(int port, std::string addr = "")
	{
		_ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _ssock)
		{
			LOG_ERROR("生成服务器套接字失败");
			return SERVER_ERROR;
		}
		if (port > 65535 || port < 1024) {
			LOG_ERROR("端口不合法");
			return SERVER_ERROR;
		}
		_ssin.sin_family = AF_INET;
		_ssin.sin_port = htons(port);
		if (addr == "") _ssin.sin_addr.s_addr = htonl(INADDR_ANY);
		else _ssin.sin_addr.s_addr = inet_addr(addr.c_str());

		//struct linger optLinger = { 0 };
		//if (openLinger_) {
		//	/* 优雅关闭: 直到所剩数据发送完毕或超时 */
		//	optLinger.l_onoff = 1;
		//	optLinger.l_linger = 1;
		//}
		//res = setsockopt(_ssock, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
		//if (ret < 0) {
		//	close(listenFd_);
		//	LOG_ERROR("Init linger error!", port_);
		//	return false;
		//}


		int res = bind(_ssock, reinterpret_cast<sockaddr*>(&_ssin), sizeof(_ssin));
		if (SOCKET_ERROR == res)
		{

			close(_ssock);
			_ssock = INVALID_SOCKET;
			LOG_ERROR("绑定端口失败");
			return SERVER_ERROR;
		}

		res = listen(_ssock, 5);
		if (SOCKET_ERROR == res)
		{
			close(_ssock);
			_ssock = INVALID_SOCKET;
			LOG_ERROR("监听端口失败");
			return SERVER_ERROR;
		}

		res = _epollManager->addFd(_ssock, _listenEvent | EPOLLIN);
		if (!res)
		{
			close(_ssock);
			_ssock = INVALID_SOCKET;
			LOG_ERROR("添加Epoll监听事件失败");
			return SERVER_ERROR;
		}
		setNonblock(_ssock);
		LOG_INFO("初始化服务器完成");

		return SERVER_SUCCESS;
	}

	int initEpoll()
	{

	}

	int setNonblock(int fd) {
		if (fd < 0)return SERVER_ERROR;
		return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	}
};


#endif // !_HTTPSERVER_H_

