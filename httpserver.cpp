#include "httpserver.h"

HttpServer::HttpServer(uint32_t listenEvent, int epollTimeoutMilli) :_ssock(INVALID_SOCKET), _ssin({}), _listenEvent(listenEvent), _epollTimeout(epollTimeoutMilli), _epollManager(new EpollManager())
{
	_running = true;
	root = getcwd(nullptr, 256);
	strncat(root, "/html/", 16);

}

HttpServer::~HttpServer()
{
	_running = false;
	if (INVALID_SOCKET != _ssock)
	{
		close(_ssock);
		_ssock = INVALID_SOCKET;
	}
}

int HttpServer::initSocket(int port, std::string addr = "")
{
	_ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _ssock)
	{
		LOG_ERROR("���ɷ������׽���ʧ��");
		_running = false;
		return SERVER_ERROR;
	}
	if (port > 65535 || port < 1024) {
		LOG_ERROR("�˿ڲ��Ϸ�");
		_running = false;
		return SERVER_ERROR;
	}
	_ssin.sin_family = AF_INET;
	_ssin.sin_port = htons(port);
	if (addr == "") _ssin.sin_addr.s_addr = htonl(INADDR_ANY);
	else _ssin.sin_addr.s_addr = inet_addr(addr.c_str());

	//struct linger optLinger = { 0 };
	//if (openLinger_) {
	//	/* ���Źر�: ֱ����ʣ���ݷ�����ϻ�ʱ */
	//	optLinger.l_onoff = 1;
	//	optLinger.l_linger = 1;
	//}
	//res = setsockopt(_ssock, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
	//if (ret < 0) {
	//	close(listenFd_);
	//	LOG_ERROR("Init linger error!", port_);
	//	return false;
	//}


	int res = bind(_ssock, reinterpret_cast<sockaddr*>(&_ssin), sizeof(_ssin));
	if (SOCKET_ERROR == res)
	{

		close(_ssock);
		_ssock = INVALID_SOCKET;
		LOG_ERROR("�󶨶˿�ʧ��");
		_running = false;
		return SERVER_ERROR;
	}

	res = listen(_ssock, 5);
	if (SOCKET_ERROR == res)
	{
		close(_ssock);
		_ssock = INVALID_SOCKET;
		LOG_ERROR("�����˿�ʧ��");
		_running = false;
		return SERVER_ERROR;
	}

	res = _epollManager->addFd(_ssock, _listenEvent | EPOLLIN);
	if (!res)
	{
		close(_ssock);
		_ssock = INVALID_SOCKET;
		LOG_ERROR("���Epoll�����¼�ʧ��");
		_running = false;
		return SERVER_ERROR;
	}
	setNonblock(_ssock);
	LOG_INFO("��ʼ�����������");

	return SERVER_SUCCESS;
}

int HttpServer::setNonblock(int fd) {
	if (fd < 0)return SERVER_ERROR;
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void HttpServer::onRun()
{
	LOG_INFO("��������ʼ����");
	while (_running)
	{
		int eventCount = _epollManager->wait(_epollTimeout);
		for (int i = 0; i < eventCount; i++)
		{
			int fd = _epollManager->getEventFd(i);
			auto event = _epollManager->getEvents(i);
			if (fd == _ssock) acceptClient();
			else if (event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) closeClient(fd);
			else if (event & EPOLLIN) onRead(fd);
			else if (event & EPOLLOUT) onWrite(fd);
			else LOG_ERROR("������δ֪����epoll eventδ����");
		}
	}
}
void HttpServer::acceptClient()
{
	SOCKET csock = INVALID_SOCKET;
	sockaddr_in csin = {};
	socklen_t len = sizeof(csin);
	while (_listenEvent & EPOLLET)
	{
		csock = accept(_ssock, reinterpret_cast<sockaddr*>(&csin), &len);
		if (INVALID_SOCKET == csock)
		{
			LOG_ERROR("���յ���Чsocket");
			return;
		}
		else if (clients.size() >= maxClient)
		{
			LOG_WARNING("�ͻ����Ѵ����ޣ���������æ");
			return;
		}
		else
		{
			LOG_INFO("�¿ͻ�������:%d IP:%s", csock, inet_ntoa(csin.sin_addr));
			clients[csock] = std::shared_ptr<CLIENT>(std::make_shared<CLIENT>(csock, csin, csock, "user" + std::to_string(csock)));
		}
	}
}

void HttpServer::closeClient(SOCKET fd)
{
	auto it = clients.find(fd);
	if (fd >= 0 && it != clients.end())
	{
		LOG_INFO("�ͻ�%d�˳�",fd);
		_epollManager->deleteFd(fd);
		clients.erase(fd);
	}
}

void HttpServer::onRead(SOCKET fd)
{

}

void HttpServer::onWrite(SOCKET fd)
{

}