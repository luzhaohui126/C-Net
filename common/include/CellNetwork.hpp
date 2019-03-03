#pragma once
#include"Cell.hpp"
//2019.3.3
class CellNetwork
{
private:
	CellNetwork() {
#ifdef  _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#else
		//忽略异常信号
		signal(SIGPIPE, SIG_IGN);
		//if (singal(SIGPIPE, SIG_IGN) == SIG_ERR)
		//	return (1);
#endif
	}
	~CellNetwork() {
#ifdef  _WIN32
		//清除Windows socket环境
		WSACleanup();
#endif
	}
public:
	static void Init() {
		static CellNetwork sObj;
	}
private:

};

