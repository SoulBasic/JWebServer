#include "global.hpp"
#include "httpserver.h"
#include "dbManager.hpp"
int main(int argc, char* argv[])
{
	HttpServer server(EPOLLET, EPOLLET, 1000);
	server.initSocket(2324,"");
	server.onRun();
	return 0;
}