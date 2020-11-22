#pragma once
#ifndef _DB_MANAGER_HPP_
#define _DB_MANAGER_HPP_
#include "global.hpp"
#include <mysql/mysql.h>
#include <queue>
#include <semaphore.h>
class DBManager
{
public:
	static DBManager& Instance()
	{
		static DBManager _dbManager;
		return _dbManager;
	}
	int connect(const char* host, int port, const char* userName, const char* passWord, const char* dbName, int connCount = 10)
	{
		if (connCount <= 0)
		{
			LOG_ERROR("��������ݿ�������С�ڵ���0");
			return -1;
		}
		for (int i = 0; i < connCount; i++)
		{
			MYSQL* sql = nullptr;
			sql = mysql_init(sql);
			if (!sql) LOG_ERROR("��ʼ��MySQL���� %d ʧ��", i);
			sql = mysql_real_connect(sql, host, userName, passWord, dbName, port, nullptr, 0);
			if(!sql) LOG_ERROR("��%d������MySQLʧ��", i);
			_connQueue.push(sql);
		}
		_maxConn = connCount;
		sem_init(&_semID, 0, _maxConn);
		return _connQueue.size();
	}

	MYSQL* getConn()
	{
		MYSQL* sql = nullptr;
		if (_connQueue.empty())
		{
			LOG_WARNING("sql���ӷ�æ");
			return nullptr;
		}
		sem_wait(&_semID);
		
		std::lock_guard<std::mutex> locker(_mtx);
		sql = _connQueue.front();
		_connQueue.pop();
		return sql;
	}

	void freeConn(MYSQL* sql)
	{
		if (sql == nullptr)return;
		std::lock_guard<std::mutex> locker(_mtx);
		_connQueue.push(sql);
		sem_post(&_semID);
	}

	void terminal()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		while (!_connQueue.empty())
		{
			auto sql = _connQueue.front();
			_connQueue.pop();
			mysql_close(sql);
		}
		mysql_library_end();
	}

	int getValidCount()
	{
		std::lock_guard<std::mutex> locker(_mtx);
		return _connQueue.size();
	}

private:
	DBManager() {}
	~DBManager() { terminal(); }
	int _maxConn;
	std::queue<MYSQL*> _connQueue;
	std::mutex _mtx;
	sem_t _semID;
};



#endif // !_DB_MANAGER_HPP_
