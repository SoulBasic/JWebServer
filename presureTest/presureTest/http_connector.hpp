#pragma once

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <thread>


class HttpConnector
{
public:
	HttpConnector(std::string ip, std::string url, std::string& buffer, int port = 80):csock(INVALID_SOCKET),ssin({}),_port(port),_ip(ip),_buffer(buffer)
	{
	}
	~HttpConnector()
	{
		if (INVALID_SOCKET != csock)
		{
			closesocket(csock);
		}
	}

	bool getResponse()
	{
		if (INVALID_SOCKET == csock)
		{
			_errno = "SOCKET无效";
			return false;
		}
		char buf[4096];
		int res = recv(csock, buf, sizeof(buf), 0);
		while (res > 0)
		{
			for (int i = 0; i < res; i++)
			{
				std::cout << buf[i];
				_buffer += buf[i];
			}
			res = recv(csock, buf, sizeof(buf), 0);
		}
		return true;
	}

	bool init()
	{
		WORD word = MAKEWORD(2, 2);
		WSADATA wsadata;
		WSAStartup(word, &wsadata);
		csock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == csock)
		{
			_errno = "获取套接字失败";
			return false;
		}
		ssin.sin_family = AF_INET;
		ssin.sin_port = htons(_port);
		ssin.sin_addr.S_un.S_addr = inet_addr(_ip.c_str());
		if (SOCKET_ERROR == connect(csock, reinterpret_cast<sockaddr*>(&ssin), sizeof(ssin)))
		{
			_errno = "连接服务器失败";
			return false;
		}
		_responseThread = std::thread(std::mem_fun(&HttpConnector::getResponse),this);
		_responseThread.detach();
		return true;
	}

	bool request(std::string content)
	{
		std::string con = "";
		for (int i = 0; i < content.length(); i++)
		{
			if (content[i] == '\n')con += '\r';
			con += content[i];
		}
		content = con;
		if (INVALID_SOCKET == csock)
		{
			_errno = "与服务器尚未建立连接";
			return false;
		}
		int res = send(csock, content.c_str(), content.length(), 0);
		qDebug(content.c_str());
		if (res != content.length())
		{
			_errno = "未完全发送或失败，发送了" + std::to_string(res);
			return false;
		}
		return true;
	}



	inline std::string getErrno() { return _errno; }
private:
	SOCKET csock;
	sockaddr_in ssin;
	std::thread _responseThread;
	std::string _ip;
	std::string _errno;
	std::string& _buffer;
	int _port;
};