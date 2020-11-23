#pragma once
#ifndef _LOG_HPP_
#define _LOG_HPP_
#include <cstdio>
#include <ctime>
#include <chrono>

#define LOG_INFO JLog::Instance().info
#define LOG_ERROR JLog::Instance().error
#define LOG_WARNING JLog::Instance().warning
#define LOG_DEBUG JLog::Instance().debug

class JLog
{
public:
	JLog& operator=(const JLog& other) = delete;
	static JLog& Instance()
	{
		static JLog _log;
		return _log;
	}

	void info(const char* str) { info("%s", str); }
	template<typename ...Args>
	void info(const char* format, Args... args)
	{
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Info]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		fprintf(_logFile, format, args...);
		printf("%s", "\n");
		printf(format, args...);
		fflush(_logFile);
	}

	void error(const char* str) { error("%s", str); }
	template<typename ...Args>
	void error(const char* format, Args... args)
	{
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Error]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		fprintf(_logFile, format, args...);
		printf("%s", "\n");
		printf(format, args...);
		fflush(_logFile);
	}

	void warning(const char* str) { warning("%s", str); }
	template<typename ...Args>
	void warning(const char* format, Args... args)
	{
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Warning]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		fprintf(_logFile, format, args...);
		printf("%s", "\n");
		printf(format, args...);
		fflush(_logFile);
	}

	void debug(const char* str) { debug("%s", str); }
	template<typename ...Args>
	void debug(const char* format, Args... args)
	{
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		fprintf(_logFile, "\n[%d-%d-%d %d:%d:%d][Debug]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		fprintf(_logFile, format, args...);
		printf("%s", "\n");
		printf(format, args...);
		fflush(_logFile);
	}

private:
	FILE* _logFile;
	JLog()
	{
		auto scn = std::chrono::system_clock().now();
		time_t nowTime = std::chrono::system_clock::to_time_t(scn);
		std::tm* now = std::gmtime(&nowTime);
		char fileName[36] = {};
		snprintf(fileName, 36, "log %d-%d-%d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
		_logFile = fopen(fileName, "at");
	}
	~JLog ()
	{
		fclose(_logFile);
	}
};



#endif // !_LOG_H_
