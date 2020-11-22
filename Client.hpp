#pragma once

#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include "global.hpp"



class CLIENT
{
public:
	CLIENT(SOCKET csock, sockaddr_in csin, int userid, std::string username)
		:sock(csock), sin(csin), lastBufPos(0), userID(userid), userName(username)
	{
		dtHeart = NOWTIME_MILLI;
	}
	~CLIENT()
	{
		if (INVALID_SOCKET != sock)
		{
			close(sock);
			sock = INVALID_SOCKET;
		}

	}
	inline SOCKET getSock() { return sock; }
	inline sockaddr_in& getSin() { return sin; }
	inline char* getmsgBuf() { return msgBuf; }
	inline int getLastBufPos() { return lastBufPos; }
	inline void setLastBufPos(int val) { lastBufPos = val; }
	inline int getUserID() { return userID; }
	inline std::string getUserName() { return userName; }
	inline void setUserName(std::string username) { userName = username; }
	inline void resetHeart(){ dtHeart = NOWTIME_MILLI; }
	inline bool checkHeart(time_t dt) { return dt - dtHeart < CLIENT_HEART_DEAD_TIME; }
private:
	SOCKET sock;
	sockaddr_in sin;
	char msgBuf[MSG_BUF_SIZE] = {};
	int lastBufPos;
	int userID;
	std::string userName;
	time_t dtHeart;

};




#endif