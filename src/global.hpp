#pragma once
#ifndef _GLOBAL_HPP_
#define _GLOBAL_HPP_
#include "log.hpp"
#include "TimeStamp.hpp"


#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <mutex>
#include <signal.h>
#include <tuple>
#define SOCKET int

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define SEND_BUF_SIZE 4096
#define RECV_BUF_SIZE 2048

#define READ_BUF_SIZE 4096
#define WRITE_BUF_SIZE 4096
#define CLIENT_HEART_DEAD_TIME 10000
#define SERVER_SEND_TIME 200 

#define REQUEST_FILENAME_MAXLEN 200

#endif // !_GLOBAL_HPP_



