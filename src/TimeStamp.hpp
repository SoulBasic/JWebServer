#pragma once
#ifndef _TIMESTAMP_HPP_
#define _TIMESTAMP_HPP_
#include <chrono>
#include <functional>

#define NOWTIME_SEC std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()
#define NOWTIME_MILLI std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()
#define NOWTIME_MICRO std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()
#define NOWTIME_NANO std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()

class Timestamp
{
public:
	Timestamp()
	{
		this->update();
	}
	~Timestamp()
	{

	}
	void update(){_begin = std::chrono::high_resolution_clock::now();}

	double getElapsedTimeInSec()
	{
		return this->getElapsedTimeInMircoSec() * 0.000001;
	}

	double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMircoSec() * 0.001;
	}

	long long  getElapsedTimeInMircoSec()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
	}

protected:
	std::chrono::time_point < std::chrono::high_resolution_clock> _begin;

};




#endif

