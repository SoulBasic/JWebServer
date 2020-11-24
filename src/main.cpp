#include "global.hpp"
#include "httpserver.h"
#include "dbManager.hpp"
int main(int argc, char* argv[])
{
	HttpServer server(EPOLLET | EPOLLRDHUP, EPOLLET | EPOLLRDHUP | EPOLLONESHOT, -1);
	if (SERVER_ERROR == server.initSocket(2324, ""))
		return -1;
	server.onRun();
	return 0;
}