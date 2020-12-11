#include "client.h"

CLIENT::~CLIENT()
{
	if (INVALID_SOCKET != _sock)
	{
		close(_sock);
		_sock = INVALID_SOCKET;
	}
}

void CLIENT::reset()
{
	_responseLength = 0;
	_responseSentLength = 0;
	_checkState = CHECK_STATE_REQUESTLINE;
	_linger = false;
	_method = METHOD_GET;
	_url = nullptr;
	_httpVersion = nullptr;
	_content = nullptr;
	_contentLength = 0;
	_host = nullptr;
	_startLine = 0;
	_checkedPos = 0;
	_lastReadBufPos = 0;
	_lastWriteBufPos = 0;
	_cgi = false;
	memset(_readBuf, '\0', READ_BUF_SIZE);
	memset(_writeBuf, '\0', WRITE_BUF_SIZE);
	memset(_requestFileName, '\0', REQUEST_FILENAME_MAXLEN);
}

REQUEST_TYPE CLIENT::process_request()
{
	_checkState = CHECK_STATE_REQUESTLINE;
	int lineStatus = LINE_OK;
	REQUEST_TYPE requestType = NO_REQUEST;
	char* lineText;
	while ((_checkState == CHECK_STATE_CONTENT && lineStatus == LINE_OK) || ((lineStatus = parse_line()) == LINE_OK))
	{
		lineText = get_line();
		_startLine = _checkedPos;
		switch (_checkState)
		{
		case CHECK_STATE_REQUESTLINE:
		{
			REQUEST_TYPE res = parse_requestLine(lineText);
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

bool CLIENT::process_response(REQUEST_TYPE requestType)
{
	switch (requestType)
	{
	case FILE_REQUEST:
	{
		const char* ok200 = "OK";
		addStatusLine(200, ok200);
		if (_fileStat.st_size != 0)
		{
			addHeaders(_fileStat.st_size);
			//第一个iovec指针指向响应报文缓冲区，长度指向m_write_idx
			_iovec[0].iov_base = _writeBuf;
			_iovec[0].iov_len = _lastWriteBufPos;
			//第二个iovec指针指向mmap返回的文件指针，长度指向文件大小
			_iovec[1].iov_base = _fileAddress;
			_iovec[1].iov_len = _fileStat.st_size;
			_ivCount = 2;
			//发送的全部数据为响应报文头部信息和文件大小
			_responseLength = _lastWriteBufPos + _fileStat.st_size;
			//LOG_DEBUG("响应体应发长度%d", _responseLength);	
			return true;
		}
		break;
	}
	case BAD_REQUEST:
	{
		const char* error400 = "Bad Request";
		const char* error400_content = "您的请求有错误";
		addStatusLine(400, error400);
		addResponse("%s", "Content-Type: text/html;charset:utf-8;");
		addHeaders(strlen(error400_content));
		if (!addResponse("%s", error400_content)) return false;
		break;
	}
	case NO_RESOURCE:
	{
		const char* error404 = "Not Found";
		const char* error404_content = "啊哦~没有你要找的内容哦";
		addStatusLine(404, error404);
		addResponse("%s", "Content-Type: text/html;charset:utf-8;");
		addHeaders(strlen(error404_content));
		if (!addResponse("%s", error404_content)) return false;
		break;
	}
	case INTERNAL_ERROR:
	{
		const char* error500 = "Internal Error";
		const char* error500_content = "服务器内部错误";
		addStatusLine(500, error500);
		addResponse("%s", "Content-Type: text/html;charset:utf-8;");
		addHeaders(strlen(error500_content));
		if (!addResponse("%s", error500_content)) return false;
		break;
	}
	case FORBIDDEN_REQUEST:
	{
		const char* error403 = "Forbidden";
		const char* error403_content = "您没有权限访问这个页面";
		addStatusLine(403, error403);
		addResponse("%s", "Content-Type: text/html;charset:utf-8;");
		addHeaders(strlen(error403_content));
		if (!addResponse("%s", error403_content)) return false;
		break;
	}
	default:
		return false;
	}
	_iovec[0].iov_base = _writeBuf;
	_iovec[0].iov_len = _lastWriteBufPos;
	_ivCount = 1;
	_responseLength = _lastWriteBufPos;
	return true;
}

std::tuple<bool, int> CLIENT::read()
{
	if (_lastReadBufPos >= READ_BUF_SIZE)//缓冲区已满
	{
		return std::make_tuple(false, EAGAIN);
	}
	int bytes_read = 0;
	while (true)
	{
		bytes_read = recv(_sock, _readBuf + _lastReadBufPos, READ_BUF_SIZE - _lastReadBufPos, 0);
		if (bytes_read == -1)
		{
			if (errno == EAGAIN) break;//EWOULDBLOCK
			return std::make_tuple(false, EAGAIN);
		}
		else if (bytes_read == 0)
		{
			LOG_DEBUG("客户%s退出", inet_ntoa(_sin.sin_addr));
			return std::make_tuple(false, -1);
		}
		_lastReadBufPos += bytes_read;
	}
	return std::make_tuple(true, EAGAIN);
}

std::tuple<bool, int> CLIENT::write()
{
	int sentLength = 0;
	//若要发送的数据长度为0
	//表示响应报文为空，一般不会出现这种情况
	if (_responseLength == 0) return std::make_tuple(false, EAGAIN);
	while (true)
	{
		//将响应报文的状态行、消息头、空行和响应正文发送给浏览器端
		sentLength = writev(_sock, _iovec, _ivCount);

		//正常发送，sentLength为发送的字节数
		if (sentLength >= 0)
		{
			// 更新已发送字节
			_responseSentLength += sentLength;
			//LOG_DEBUG("成功发送%d数据，累计发送%d,还剩%d", sentLength, _responseSentLength, _responseLength - sentLength);
			if (_responseSentLength >= _iovec[0].iov_len)
			{
				//第一个iovec头部信息的数据已发送完，发送第二个iovec数据
				_iovec[0].iov_len = 0;
				_iovec[1].iov_base = _fileAddress + (_responseSentLength - _lastWriteBufPos);
				_iovec[1].iov_len = _responseLength;
			}
			//继续发送第一个iovec头部信息的数据
			else
			{
				_iovec[0].iov_base = _writeBuf + _responseSentLength;
				_iovec[0].iov_len -= _responseSentLength;
			}
		}
		else
		{
			//判断缓冲区是否满了
			if (errno == EAGAIN)
			{
				if (_responseSentLength >= _iovec[0].iov_len)
				{
					//第一个iovec头部信息的数据已发送完，发送第二个iovec数据
					_iovec[0].iov_len = 0;
					_iovec[1].iov_base = _fileAddress + (_responseSentLength - _lastWriteBufPos);
					_iovec[1].iov_len = _responseLength;
				}
				//继续发送第一个iovec头部信息的数据
				else
				{
					_iovec[0].iov_base = _writeBuf + _responseSentLength;
					_iovec[0].iov_len -= _responseSentLength;
				}
				//需要重新注册写事件
				//LOG_DEBUG("一次未发送完的情况,累计发送%d,还剩%d", _responseSentLength, _responseLength);
				return std::make_tuple(true, EPOLLOUT);
			}
			//如果发送失败，但不是缓冲区问题，取消映射
			unmap();
			LOG_ERROR("发送失败，但不是缓冲区问题的情况");
			return std::make_tuple(false, EPOLLERR);
		}
		//更新已发送字节数
		_responseLength -= sentLength;
		//判断条件，数据已全部发送完
		if (_responseLength <= 0)
		{
			unmap();
			//需重置EPOLLONESHOT事件
			//LOG_DEBUG("发送完全,累计发送%d,还剩%d", _responseSentLength, _responseLength);
			if (_linger)
			{
				reset();
				return std::make_tuple(true, EPOLLIN);
			}
			else return std::make_tuple(false, EPOLLERR);
		}

	}
}

void CLIENT::unmap()
{
	if (_fileAddress != nullptr)
	{
		munmap(_fileAddress, _fileStat.st_size);
		_fileAddress = nullptr;
	}
}

REQUEST_TYPE CLIENT::parse_requestLine(char* lineText)
{
	_url = strpbrk(lineText, " \t");
	*_url++ = '\0'; //将字符串首的空格或者TAB置换为\0
	char* method = lineText;

	if (strcasecmp(method, "GET") == 0) _method = METHOD_GET;
	else if (strcasecmp(method, "POST") == 0)
	{
		_method = METHOD_POST;
		_cgi = true;
	}
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

	if (_url == nullptr || _url[0] != '/') return BAD_REQUEST;
	
	if (strlen(_url) == 1)strcat(_url, "index.html");
	_checkState = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

REQUEST_TYPE CLIENT::parse_headers(char* lineText)
{
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
	}
	else if (0 == strncasecmp(lineText, "Content-length:", 15))
	{
		lineText += 15;
		lineText += strspn(lineText, " \t");
		_contentLength = atoi(lineText);
		//LOG_DEBUG("Content-length:%d", _contentLength);
	}
	else if (0 == strncasecmp(lineText, "Host:", 5))
	{
		lineText += 5;
		lineText += strspn(lineText, " \t");
		_host = lineText;
	}
	else
	{
		//LOG_WARNING("客户%s请求中携带未知的header", inet_ntoa(_sin.sin_addr));
	}
	return NO_REQUEST;
}

REQUEST_TYPE CLIENT::parse_content(char* lineText)
{
	if (_lastReadBufPos >= _contentLength + _checkedPos)
	{
		lineText[_contentLength] = '\0';
		_content = lineText;
		return GET_REQUEST;
	}
	return NO_REQUEST;

}

REQUEST_TYPE CLIENT::doGet()
{
	strcpy(_requestFileName, _root);
	int len = strlen(_root);
	const char* p = strrchr(_url, '/');
	p++;
	if (_cgi && strncasecmp(p, "login", 5) == 0)
	{
		LOG_INFO("用户POST方法请求login接口");
		std::string content(_content);
		LoginCGI cgi(content);
		cgi.parse_form();
		if (cgi.para_count() != 2)
		{
			LOG_INFO("参数不为2，错误");
			return BAD_REQUEST;
		}
		bool stat = false;
		std::string msg = "";
		std::tie(stat,msg) = cgi.exec();
		if (stat)
		{
			LOG_INFO("用户 %s 登录成功",cgi.userName().c_str());
			strcpy(_url, "/welcome.html");
		}
		else
		{
			LOG_INFO("用户 %s 登录失败", cgi.userName().c_str());
			strcpy(_url, "/error.html");
		}
	}
	else if (_cgi && strncasecmp(p, "register", 8) == 0)
	{
		LOG_INFO("用户POST方法请求register接口");
		std::string content(_content);
		RegisterCGI cgi(content);
		cgi.parse_form();
		if (cgi.para_count() != 2)
		{
			LOG_INFO("参数不为2，错误");
			return BAD_REQUEST;
		}
		bool stat = false;
		std::string msg = "";
		std::tie(stat, msg) = cgi.exec();
		if (stat)
		{
			LOG_INFO("用户 %s 注册成功", cgi.userName().c_str());
			strcpy(_url, "/welcome.html");
		}
		else
		{
			LOG_INFO("用户 %s 注册失败", cgi.userName().c_str());
			strcpy(_url, "/error.html");
		}
	}

	strncpy(_requestFileName + len, _url, REQUEST_FILENAME_MAXLEN - len - 1);

	LOG_DEBUG("用户%s请求URL = %s",inet_ntoa(_sin.sin_addr), _requestFileName);
	if (stat(_requestFileName, &_fileStat) < 0)return NO_RESOURCE;
	if (!(_fileStat.st_mode & S_IROTH))return FORBIDDEN_REQUEST;
	if (S_ISDIR(_fileStat.st_mode))return BAD_REQUEST;

	int file = open(_requestFileName, O_RDONLY);
	_fileAddress = reinterpret_cast<char*>(mmap(0, _fileStat.st_size, PROT_READ, MAP_PRIVATE, file, 0));
	close(file);
	return FILE_REQUEST;
}


int CLIENT::parse_line()
{
	char temp;
	for (; _checkedPos < _lastReadBufPos; _checkedPos++)
	{
		temp = _readBuf[_checkedPos];
		if (temp == '\r')
		{
			if (_checkedPos == _lastReadBufPos - 1)return LINE_OPEN;
			else if (_readBuf[_checkedPos + 1] == '\n')
			{
				_readBuf[_checkedPos++] = '\0';
				_readBuf[_checkedPos++] = '\0';
				return LINE_OK;
			}
			else return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

bool CLIENT::addResponse(const char *format, ...)
{
	if (_lastWriteBufPos >= WRITE_BUF_SIZE)
		return false;
	va_list arg_list;
	va_start(arg_list, format);
	int len = vsnprintf(_writeBuf + _lastWriteBufPos, WRITE_BUF_SIZE - 1 - _lastWriteBufPos, format, arg_list);
	if (len >= (WRITE_BUF_SIZE - 1 - _lastWriteBufPos))
	{
		va_end(arg_list);
		return false;
	}
	_lastWriteBufPos += len;
	va_end(arg_list);
	return true;
}
