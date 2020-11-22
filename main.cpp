#include "global.hpp"
#include "httpserver.h"
#include "dbManager.hpp"
int main(int argc, char* argv[])
{
	int count = DBManager::Instance().connect("soulbasic.cxewdbabus4o.ap-northeast-1.rds.amazonaws.com", 3306, "tws", "123456", "tws", 10);
	std::cout << count << std::endl;
	return 0;
}