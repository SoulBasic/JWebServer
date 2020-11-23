#pragma once

#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include "global.hpp"

#define LINE_OK 0
#define LINE_BAD 1 
#define LINE_OPEN 2

enum REQUEST_TYPE
{
	NO_REQUEST = 0,
	GET_REQUEST,
	BAD_REQUEST,
	NO_RESOURCE,
	FORBIDDEN_REQUEST,
	FILE_REQUEST,
	INTERNAL_ERROR,
	CLOSED_CONNECTION
};

#define CHECK_STATE_REQUESTLINE 0
#define CHECK_STATE_HEADER 1
#define CHECK_STATE_CONTENT 2

#define METHOD_INVALID -1
#define METHOD_GET 0
#define METHOD_POST 1

class CLIENT
{
public:
	CLIENT(SOCKET ssock, SOCKET csock, sockaddr_in csin, int userid, std::string username)
		:_url(nullptr), _httpVersion(nullptr), _ssock(ssock), _sock(csock), _sin(csin), _lastBufPos(0), _checkedPos(0),
		_checkState(CHECK_STATE_REQUESTLINE), _startLine(0), _method(METHOD_INVALID), _contentLength(0), _userID(userid),
		_userName(username), _linger(false), _host(nullptr)
	{
		memset(_msgBuf, '\0', MSG_BUF_SIZE);
		_dtHeart = NOWTIME_MILLI;
	}
	~CLIENT()
	{
		if (INVALID_SOCKET != _sock)
		{
			close(_sock);
			_sock = INVALID_SOCKET;
		}
		

	}
	inline SOCKET getSock() { return _sock; }
	inline sockaddr_in& getSin() { return _sin; }
	inline char* getmsgBuf() { return _msgBuf; }
	inline int getLastBufPos() { return _lastBufPos; }
	inline void setLastBufPos(int val) { _lastBufPos = val; }
	inline int getUserID() { return _userID; }
	inline std::string getUserName() { return _userName; }
	inline void setUserName(std::string username) { _userName = username; }
	inline void resetHeart(){ _dtHeart = NOWTIME_MILLI; }
	inline bool checkHeart(time_t dt) { return dt - _dtHeart < CLIENT_HEART_DEAD_TIME; }


	std::tuple<bool,int> read_once()
	{
		if (_lastBufPos >= MSG_BUF_SIZE)//缓冲区已满
		{
			return std::make_tuple(false,EAGAIN);
		}
		int bytes_read = 0;
		while (true)
		{
			bytes_read = recv(_sock, _msgBuf + _lastBufPos, MSG_BUF_SIZE - _lastBufPos, 0);
			if (bytes_read == -1)
			{
				if (errno == EAGAIN) break;//EWOULDBLOCK
				return std::make_tuple(false, EAGAIN);
			}
			else if (bytes_read == 0)
			{
				LOG_DEBUG("bytes read =0 客户退出");
				return std::make_tuple(false, -1);
			}
			_lastBufPos += bytes_read;
		}
		return std::make_tuple(true, EAGAIN);
	}

	void process()
	{
		process_request();
	}

	REQUEST_TYPE process_request()
	{
		_checkState = CHECK_STATE_REQUESTLINE;
		int lineStatus = LINE_OK;
		int requestType = NO_REQUEST;
		char* lineText;
		while ((_checkState == CHECK_STATE_CONTENT && lineStatus == LINE_OK) || ((lineStatus = parse_line()) == LINE_OK))
		{
			lineText = get_line();
			_startLine = _checkedPos;
			LOG_INFO(lineText);
			switch (_checkState)
			{
			case CHECK_STATE_REQUESTLINE:
			{
				auto res = parse_requestLine(lineText);
				LOG_DEBUG("res = %d", res);
				if (res == BAD_REQUEST)return BAD_REQUEST;
				break;
			}
			case CHECK_STATE_HEADER:
			{
				requestType = parse_headers(lineText);
				if (BAD_REQUEST == requestType)return BAD_REQUEST;
				else if (GET_REQUEST == requestType) return doGet();
				break;
			}
			case CHECK_STATE_CONTENT:
			{
				requestType = parse_content(lineText);
				if (GET_REQUEST == requestType) return doGet();
				lineStatus = LINE_OPEN;
				break;
			}
			default:
				return INTERNAL_ERROR;
			}
		}
		return NO_REQUEST;
	}

	
	REQUEST_TYPE parse_requestLine(char* lineText)
	{
		_url = strpbrk(lineText, " \t");
		*_url++ = '\0'; //将字符串首的空格或者TAB置换为\0
		char* method = lineText;

		if (strcasecmp(method, "GET") == 0) _method = METHOD_GET;
		else if (strcasecmp(method, "POST") == 0) _method = METHOD_POST;
		else return BAD_REQUEST;
		_url += strspn(_url, " \t");
		_httpVersion = strpbrk(_url, " \t");
		if (_httpVersion == nullptr)return BAD_REQUEST;
		*_httpVersion++ = '\0';
		_httpVersion += strspn(_httpVersion, " \t");
		if (strcasecmp(_httpVersion, "HTTP/1.1") != 0)return BAD_REQUEST;//仅支持HTTP/1.1
		if (strncasecmp(_url, "http://", 7) == 0)
		{
			_url += 7;
			_url = strchr(_url, '/');
		}

		if (strncasecmp(_url, "https://", 8) == 0)
		{
			_url += 8;
			_url = strchr(_url, '/');
		}

		if (_url != nullptr || _url[0] != '/') return BAD_REQUEST;
		
		if (strlen(_url) == 1)strcat(_url, "index.html");
		_checkState = CHECK_STATE_HEADER;
		return NO_REQUEST;
	}

	REQUEST_TYPE parse_headers(char* lineText)
	{
		LOG_DEBUG("headers %s", lineText);
		if ('\0' == lineText[0])
		{
			if (_contentLength != 0)
			{
				_checkState = CHECK_STATE_CONTENT;
				return NO_REQUEST;
			}
			return GET_REQUEST;
		}
		else if (0 == strncasecmp(lineText, "Connection:", 11))
		{
			lineText += 11;
			lineText += strspn(lineText, " \t");
			if (0 == strcasecmp(lineText, "keep-alive"))
			{
				_linger = true;
			}
			LOG_DEBUG("Connection:");
		}
		else if (0 == strncasecmp(lineText, "Content-length:", 15))
		{
			lineText += 15;
			lineText += strspn(lineText, " \t");
			_contentLength = atoi(lineText);
			LOG_DEBUG("Content-length:%d", _contentLength);
		}
		else if (0 == strncasecmp(lineText, "Host:", 5))
		{
			lineText += 5;
			lineText += strspn(lineText, " \t");
			_host = lineText;
		}
		else
		{
			LOG_WARNING("客户%s请求中携带未知的header", inet_ntoa(_sin.sin_addr));
		}
		return NO_REQUEST;
	}

	REQUEST_TYPE parse_content(char* lineText)
	{
		LOG_ERROR("暂未支持POST CONTENT解析");
		return GET_REQUEST;
	}

	REQUEST_TYPE doGet()
	{
		LOG_INFO("doget");
		
	}

	int parse_line()
	{
		char temp;
		for (; _checkedPos < _lastBufPos; _checkedPos++)
		{
			temp = _msgBuf[_checkedPos];
			if (temp == '\r')
			{
				if (_checkedPos == _lastBufPos - 1)return LINE_OPEN;
				else if (_msgBuf[_checkedPos + 1] == '\n')
				{
					_msgBuf[_checkedPos++] = '\0';
					_msgBuf[_checkedPos++] = '\0';
					return LINE_OK;
				}
				else return LINE_BAD;
			}
		}
		return LINE_OPEN;
	}
	inline char* get_line() { return _msgBuf + _startLine; };

private:
	char _msgBuf[MSG_BUF_SIZE];
	char* _url;
	char* _httpVersion;
	char* _host;
	SOCKET _ssock;
	SOCKET _sock;
	sockaddr_in _sin;

	int _lastBufPos;
	int _checkedPos;
	int _checkState;
	int _userID;
	int _startLine;
	int _method;
	int _contentLength;
	std::string _userName;
	time_t _dtHeart;
	bool _linger;
	
};




#endif