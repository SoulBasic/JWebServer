#pragma once
#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_
#include "Timer.h"
#include "global.hpp"
#include "EpollManager.hpp"
#include "client.h"
#include "ThreadManager.hpp"
#include "DBManager.hpp"


#define SERVER_ERROR -1
#define SERVER_SUCCESS 0


class HttpServer
{
private:
	std::unordered_map<SOCKET, std::shared_ptr<CLIENT>> clients;
	sockaddr_in _ssin;
	char* _root;
	std::unique_ptr<EpollManager> _epollManager;
	std::unique_ptr<ThreadManager> _threadManager;

	uint32_t _listenEvent;
	uint32_t _connEvent;
	SOCKET _ssock;
	int _epollTimeout;
	static const int maxClient = 65536;
	bool _running;
	void acceptClient();
	void onRead(SOCKET fd);
	void onWrite(SOCKET fd);
	void handleRequest(std::shared_ptr<CLIENT> client);
	void handleResponse(std::shared_ptr<CLIENT> client);
	int setNonblock(int fd);
public:
	void closeClient(SOCKET fd);
	HttpServer(uint32_t listenEvent, uint32_t connEvent, int epollTimeoutMilli);
	~HttpServer();
	int initSocket(int port, std::string addr);
	void onRun();
	inline void setRunning(bool running) { _running = running; }
	
	std::unique_ptr<WheelTimer> _wheelTimer;
	static HttpServer* ptr_server;
	static void alarm_handler(int sig);
};


#endif // !_HTTPSERVER_H_

