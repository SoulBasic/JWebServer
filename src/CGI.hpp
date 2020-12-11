#pragma once
#ifndef _CGI_HPP_
#define _CGI_HPP_

#include "global.hpp"



class CGIBase
{
public:
	CGIBase(std::string content):_content(content),_errno(""){}
	virtual ~CGIBase(){}
	virtual void parse_form()
	{
		_paras.clear();
		auto paras = split(_content, "&");
		if (paras.size() == 0)return;
		for (auto& l : paras)
		{
			int i = 0;
			for (i; i < l.length(); i++)
			{
				if (l[i] == '=')break;
			}
			std::string key = l.substr(0, i);
			std::string val = "";
			if (i + 1 < l.length())
			{
				val = l.substr(i + 1);
			}
			_paras[key] = val;
		}
	}
	virtual size_t para_count() { return _paras.size(); }
protected:
	std::string _content;
	std::map<std::string, std::string> _paras;
	std::vector<std::string> split(std::string str, std::string pattern)
	{
		std::string::size_type pos;
		std::vector<std::string> result;
		str += pattern;//扩展字符串以方便操作
		int size = str.size();
		for (int i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::string s = str.substr(i, pos - i);
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}
	std::string _errno;
};


#endif // !_CGI_HPP_

