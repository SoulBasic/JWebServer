#pragma once

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "global.hpp"
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>

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
	CLIENT(SOCKET ssock, SOCKET csock, sockaddr_in csin, int userid, const char* root)
		:_url(nullptr), _httpVersion(nullptr), _ssock(ssock), _sock(csock), _sin(csin), _lastReadBufPos(0), _checkedPos(0),
		_checkState(CHECK_STATE_REQUESTLINE), _startLine(0), _method(METHOD_INVALID), _contentLength(0), _userID(userid),
		 _linger(false), _host(nullptr), _root(root), _requestFileName({}),_fileAddress(nullptr),_lastWriteBufPos(0),
		_ivCount(0),_responseLength(0),_responseSentLength(0)
	{
		memset(_readBuf, '\0', READ_BUF_SIZE);
		_dtHeart = NOWTIME_MILLI;
	}
	~CLIENT();

	inline SOCKET getSock() { return _sock; }
	inline sockaddr_in& getSin() { return _sin; }
	inline char* getmsgBuf() { return _readBuf; }
	inline int getLastBufPos() { return _lastReadBufPos; }
	inline void setLastBufPos(int val) { _lastReadBufPos = val; }
	inline int getUserID() { return _userID; }
	inline void resetHeart(){ _dtHeart = NOWTIME_MILLI; }
	inline bool checkHeart(time_t dt) { return dt - _dtHeart < CLIENT_HEART_DEAD_TIME; }
	inline bool writeBufAllSent() { return (_iovec[0].iov_len + _iovec[1].iov_len) <= 0; }
	inline bool getLinger() { return _linger; }
	REQUEST_TYPE process_request();
	bool process_response(REQUEST_TYPE requestType);
	std::tuple<bool, int> read();
	std::tuple<bool, int> write();

private:
	char _readBuf[READ_BUF_SIZE];
	char _writeBuf[WRITE_BUF_SIZE];
	char _requestFileName[REQUEST_FILENAME_MAXLEN];

	struct stat _fileStat;
	struct iovec _iovec[2];
	struct sockaddr_in _sin;
	time_t _dtHeart;

	const char* _root;
	char* _url;
	char* _httpVersion;
	char* _host;
	char* _fileAddress;

	SOCKET _ssock;
	SOCKET _sock;
	int _lastReadBufPos;
	int _lastWriteBufPos;
	int _checkedPos;
	int _checkState;
	int _userID;
	int _startLine;
	int _method;
	int _contentLength;
	int _ivCount;
	int _responseLength;
	int _responseSentLength;

	bool _linger;
	void unmap();
	REQUEST_TYPE parse_requestLine(char* lineText);
	REQUEST_TYPE parse_headers(char* lineText);
	REQUEST_TYPE parse_content(char* lineText);
	REQUEST_TYPE doGet();
	int parse_line();
	inline char* get_line() { return _readBuf + _startLine; };
	bool addStatusLine(int statusCode, const char* title)
	{
		return addResponse("%s %d %s\r\n", "HTTP/1.1", statusCode, title);
	}

	bool addHeaders(int contentLength)
	{
		return addResponse("Content-Length:%d\r\nConnection:%s\r\n\r\n", contentLength, (_linger == true) ? "keep-alive" : "close");
		//Content-Encoding:utf-8\r\n
	}
	bool addResponse(const char *format, ...);
};
#endif