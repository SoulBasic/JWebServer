#include "global.hpp"
#include "httpserver.h"
#include "dbManager.hpp"

void cmdThread(HttpServer* s)
{
	std::cout << "\n";
	std::string cmd = "";
	while (true)
	{
		std::cin >> cmd;
		if ("quit" == cmd)
		{
			s->setRunning(false);
			break;
		}
		else if ("print" == cmd)
		{
			std::cout << "已打开控制台日志" << std::endl;
			JLog::Instance().setPrint(true);
		}
		else if ("noprint" == cmd)
		{
			std::cout << "已关闭控制台日志" << std::endl;
			JLog::Instance().setPrint(false);
		}
		else std::cout << "无法解析的命令" << std::endl;
	}

}


int main(int argc, char* argv[])
{
	HttpServer server(EPOLLET | EPOLLRDHUP, EPOLLET | EPOLLRDHUP | EPOLLONESHOT, 500);
	if (SERVER_ERROR == server.initSocket(2324, ""))
		return -1;
	std::thread tcmd(cmdThread, &server);
	tcmd.detach();
	server.onRun();
	std::cout << "程序结束" << std::endl;
	return 0;
}