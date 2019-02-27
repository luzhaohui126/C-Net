#pragma once

#include"Cell.hpp"
#include"CellTask.hpp"

class CellLog
{
public:
	CellLog() {
		//启动任务
		_taskServer.Start();
	}
	~CellLog() {
		//关闭任务
		_taskServer.Close();
		if (_logFile)
		{
			Info("CellLog fclose(_logFile)\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
public:
	static CellLog& Instance()
	{
		static  CellLog sLog;
		return sLog;
	}
	void setLogPath(const char* logPath,const char* mode) {
		if (_logFile) {
			fclose(_logFile);
		}
		_logFile = fopen(logPath, mode);
		if (_logFile) {
			Info("CellLog::setLogPath sucecess,<%s,%s>\n",logPath,mode);
		}
		else {
			Info("CellLog::setLogPath failed,<%s,%s>\n", logPath, mode);
		}
	}
	//Info
	//Debug
	//Warning
	//Error
	void getTime(std::tm* _tm){
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
#ifdef _WIN32
		localtime_s(_tm,&tt);
#else
		localtime_r(&tt,_tm);
#endif
	}
	static void Info(const char* pStr)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			//实时写入
			if (pLog->_logFile)
			{
				std::tm now;
				pLog->getTime(&now);
				fprintf(pLog->_logFile, "%s", "Info:");
				fprintf(pLog->_logFile, "[%04d-%02d-%02d %02d:%02d:%02d]",
					(now.tm_year + 1900), now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
				fprintf(pLog->_logFile,"%s" ,pStr);
				fflush(pLog->_logFile);
			}
			printf("%s", pStr);
		});
	}
	//log::Info(...)
	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			std::tm now;
			pLog->getTime(&now);
			printf("[%d:%d:%d] ", now.tm_hour, now.tm_min, now.tm_sec);
			if (pLog->_logFile)
			{
				fprintf(pLog->_logFile, "%s", "Info:");
				fprintf(pLog->_logFile, "[%04d-%02d-%02d %02d:%02d:%02d]",
					(now.tm_year + 1900), now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
				fprintf(pLog->_logFile, pformat, args ...);
				fflush(pLog->_logFile);
			}
			printf(pformat, args ...);
		});
	}
private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};

