#pragma once

#ifndef _LOGINCGI_HPP_
#define _LOGINCGI_HPP_

#include "CGI.hpp"

class LoginCGI : public CGIBase
{
public:
	LoginCGI(std::string content):CGIBase(content),_userName(""),_passWord(""){}
	~LoginCGI(){}
	std::tuple<bool,std::string> exec()
	{
		std::string userName = _paras["username"];
		std::string passWord = _paras["password"];
		if (userName.find('\'') != userName.npos || passWord.find('\'') != userName.npos)
		{
			return { false, "login request have been rejected" };
		}
		_userName = userName;
		_passWord = passWord;
		
		if (userName == "" || passWord == "")
		{
			return { false,"invaild input" };
		}
		else
		{
			std::string sql = "SELECT password FROM user WHERE username = '" + userName + "'";
			auto_conn ac;
			if (ac.get() == nullptr)
			{
				LOG_ERROR("取到MYSQL空指针");
				return { false,"server error" };
			}
			int res = mysql_query(ac.get(), sql.c_str());
			if (res != 0)
			{
				return { false,"server error" };
			}
			MYSQL_RES* result = mysql_store_result(ac.get());
			MYSQL_ROW row = mysql_fetch_row(result);
			auto cmp = strcmp(row[0], passWord.c_str());
			mysql_free_result(result);
			if (cmp == 0)
			{
				
				return { true,"welcome back" };
			}
			else
			{
				return { false,"login failed" };
			}

		}

	}

	std::string userName() { return _userName; }
private:
	std::string _userName;
	std::string _passWord;
};



#endif