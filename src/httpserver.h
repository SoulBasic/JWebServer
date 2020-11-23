#pragma once
#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "global.hpp"
#include "EpollManager.hpp"
#include "Client.hpp"
#include "ThreadManager.hpp"
#define SERVER_ERROR -1
#define SERVER_SUCCESS 0

class HttpServer
{
private:
	SOCKET _ssock;
	sockaddr_in _ssin;
	uint32_t _listenEvent;
	uint32_t _connEvent;
	int _epollTimeout;
	char* root;
	bool _running;
	std::unique_ptr<EpollManager> _epollManager;
	std::unique_ptr<ThreadManager> _threadManager;
	static const int maxClient = 65536;
	std::unordered_map<SOCKET, std::shared_ptr<CLIENT>> clients;

	void acceptClient();
	void closeClient(SOCKET fd);
	void onRead(SOCKET fd);
	void onWrite(SOCKET fd);
	void handleRequest(std::shared_ptr<CLIENT> client);
	void handleResponse(std::shared_ptr<CLIENT> client);
	int setNonblock(int fd);
public:
	HttpServer(uint32_t listenEvent, uint32_t connEvent, int epollTimeoutMilli);
	~HttpServer();
	int initSocket(int port, std::string addr);
	void onRun();

};


#endif // !_HTTPSERVER_H_

