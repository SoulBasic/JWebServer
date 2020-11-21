#pragma once
#ifndef _GLOBAL_HPP_
#define _GLOBAL_HPP_

#include <iostream>
#include <vector>
#include <thread>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "log.hpp"
#include <memory>
#define SOCKET int

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)


#endif // !_GLOBAL_HPP_



