#include "global.hpp"
#include "httpserver.h"
#include "dbManager.hpp"
int main(int argc, char* argv[])
{
	HttpServer server(EPOLLET, 1000);
	server.initSocket(2324,"192.168.199.132");
	server.onRun();
	return 0;
}