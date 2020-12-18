#include "global.hpp"
#include "httpserver.h"
#include "DBManager.hpp"

void cmdThread(HttpServer* s)
{
	std::cout << "\n";
	std::string cmd = "";
	while (true)
	{
		std::cin >> cmd;
		if ("quit" == cmd || "exit" == cmd)
		{
			s->setRunning(false);
			break;
		}
		else if ("print" == cmd)
		{
			std::cout << "已打开控制台日志" << std::endl;
			JLog::Instance()->setPrint(true);
		}
		else if ("noprint" == cmd)
		{
			std::cout << "已关闭控制台日志" << std::endl;
			JLog::Instance()->setPrint(false);
		}
		else if ("synlog" == cmd)
		{
			std::cout << "已开启同步日志模式" << std::endl;
			JLog::Instance()->setSyn(true);
		}
		else if ("asynlog" == cmd)
		{
			std::cout << "已开启异步日志模式" << std::endl;
			JLog::Instance()->setSyn(false);
		}
		else if ("nolog" == cmd)
		{
			std::cout << "已关闭日志" << std::endl;
			JLog::Instance()->setLevel(Level_NoLog);
		}
		else if ("clients" == cmd)
		{
			std::cout << "连接数量为：" << s->client_count();
		}
		else std::cout << "无法解析的命令" << std::endl;
	}

}


int main(int argc, char* argv[])
{

	DBManager::Instance()->connect("soulbasic.cxewdbabus4o.ap-northeast-1.rds.amazonaws.com", 3306, "tws", "123456", "tws", 5);
	JLog::Instance()->setLevel(Level_Debug);
	JLog::Instance()->setSyn(false);//异步日志

	char* root = getcwd(nullptr, 256);
	strncat(root, "/html", 16);
	CLIENT::_root = root;
	HttpServer server(EPOLLET | EPOLLRDHUP, EPOLLET | EPOLLRDHUP | EPOLLONESHOT, 500);
	if (SERVER_ERROR == server.initSocket(2324, ""))
		return -1;
	std::thread tcmd(cmdThread, &server);
	tcmd.detach();
	server.onRun();
	std::cout << "程序结束" << std::endl;
	return 0;
}