#pragma once
#ifndef _LOG_HPP_
#define _LOG_HPP_
#include <cstdio>
#include <ctime>
#include <chrono>
#include "BlockQueue.hpp"
#include <thread>

#define LOG_INFO JLog::Instance()->info
#define LOG_ERROR JLog::Instance()->error
#define LOG_WARNING JLog::Instance()->warning
#define LOG_DEBUG JLog::Instance()->debug


enum LogLevel
{
	Level_Debug = 0,
	Level_Release,
	Level_ReleaseNoWarning,
	Level_NoLog
};

class JLog
{
public:
	JLog& operator=(const JLog& other) = delete;
	static JLog* Instance()
	{
		static JLog _log;
		return &_log;
	}

	inline void setPrint(bool print) { _print = print; }
	inline void setLevel(LogLevel level) { _level = level; }
	inline void setSyn(bool syn) { _syn = syn; }

	void info(const char* str) { info("%s", str); }
	template<typename ...Args>
	void info(const char* format, Args... args)
	{
		if (_level >= Level_NoLog)return;
		std::unique_lock<std::mutex> locker(_mtx);
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		if (!_syn)
		{
			int c1 = snprintf(_buf, 40960, "\n[%d-%d-%d %d:%d:%d][Info]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			int c2 = snprintf(_buf + c1, 40000, format, args...);
			_buf[c1 + c2] = '\0';
			_bqueue.push_back(std::string(_buf));
		}
		else
		{
			fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Info]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			fprintf(_logFile, format, args...);
			if (_print)
			{
				printf("%s", "\n");
				printf(format, args...);
			}
		}

		flush();
	}

	void error(const char* str) { error("%s", str); }
	template<typename ...Args>
	void error(const char* format, Args... args)
	{
		if (_level >= Level_NoLog)return;
		std::unique_lock<std::mutex> locker(_mtx);
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		if (!_syn)
		{
			int c1 = snprintf(_buf, 40960, "\n[%d-%d-%d %d:%d:%d][Error]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			int c2 = snprintf(_buf + c1, 40000, format, args...);
			_buf[c1 + c2] = '\0';
			_bqueue.push_back(std::string(_buf));
		}
		else
		{
			fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Error]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			fprintf(_logFile, format, args...);
			if (_print)
			{
				printf("%s", "\n");
				printf(format, args...);
			}
		}

		flush();
	}

	void warning(const char* str) { warning("%s", str); }
	template<typename ...Args>
	void warning(const char* format, Args... args)
	{
		if (_level >= Level_ReleaseNoWarning)return;
		std::unique_lock<std::mutex> locker(_mtx);
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		
		if (!_syn)
		{
			int c1 = snprintf(_buf, 40960, "\n[%d-%d-%d %d:%d:%d][Warning]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			int c2 = snprintf(_buf + c1, 40000, format, args...);
			_buf[c1 + c2] = '\0';
			_bqueue.push_back(std::string(_buf));
		}
		else
		{
			fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Warning]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			fprintf(_logFile, format, args...);
			if (_print)
			{
				printf("%s", "\n");
				printf(format, args...);
			}
		}

		flush();
	}

	void debug(const char* str) { debug("%s", str); }
	template<typename ...Args>
	void debug(const char* format, Args... args)
	{
		if (_level > Level_Debug)return;
		std::unique_lock<std::mutex> locker(_mtx);
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);

		if (!_syn)
		{
			int c1 = snprintf(_buf, 40960, "\n[%d-%d-%d %d:%d:%d][Debug]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			int c2 = snprintf(_buf + c1, 40000, format, args...);
			_buf[c1 + c2] = '\0';
			_bqueue.push_back(std::string(_buf));
		}
		else
		{
			fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Debug]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			fprintf(_logFile, format, args...);
			if (_print)
			{
				printf("%s", "\n");
				printf(format, args...);
			}
		}

		flush();
	}

private:
	FILE* _logFile;
	bool _print;
	LogLevel _level;
	bool _syn;
	char _buf[40960];
	BlockQueue<std::string> _bqueue;
	std::thread _asynThread;
	std::mutex _mtx;



	JLog() :_print(true), _syn(true), _bqueue(1024),_asynThread(std::thread(JLog::asynThread))
	{
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		char fileName[36] = {};
		snprintf(fileName, 36, "log%d-%d-%d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
		_logFile = fopen(fileName, "at");
		_asynThread.detach();
	}
	~JLog()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		flush();
		fclose(_logFile);
	}

	void flush() {
		if (!_syn) {
			_bqueue.flush();
		}
		fflush(_logFile);
	}


	static void asynThread(){JLog::Instance()->asynWrite();}

	void asynWrite() {
		std::string str = "";
		while (_bqueue.take(str)) {
			std::lock_guard<std::mutex> locker(_mtx);
			fprintf(_logFile, "%s", str.c_str());
			if (_print)
			{
				printf("%s", str.c_str());
			}

		}
		printf("%s", "[Error]异步日志未知原因已终止!");
	}
};



#endif // !_LOG_H_
