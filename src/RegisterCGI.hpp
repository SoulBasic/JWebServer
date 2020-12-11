#pragma once

#ifndef _REGISTERCGI_HPP_
#define _REGISTERCGI_HPP_

#include "CGI.hpp"
#include "DBManager.hpp"
class RegisterCGI : public CGIBase
{
public:
	RegisterCGI(std::string content) :CGIBase(content), _userName(""), _passWord("") {}
	~RegisterCGI() {}
	std::tuple<bool, std::string> exec()
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
			std::string sql = "INSERT INTO user VALUES('" + userName + "','" + passWord + "')";
			auto_conn ac;
			if (ac.get() == nullptr)
			{
				LOG_ERROR("»°µΩMYSQLø’÷∏’Î");
				return { false,"server error" };
			}
			int res = mysql_query(ac.get(), sql.c_str());
			if (res != 0)
			{
				return { false,"server error" };
			}
			return { true,"register ok" };
		}
		
	}

	std::string userName() { return _userName; }
private:
	std::string _userName;
	std::string _passWord;
};

#endif