#pragma once

#ifdef	_WIN32
#define FD_SETSIZE      10006
#define	WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>

#define	SOCKET int
#define INVALID_SOCKET	(SOCKET)(~0)
#define SOCKET_ERROR	(-1)
#endif

//#include<memory>
#include<map>
#include<atomic>
#include<vector>

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 8192
#define SEND_BUFF_SIZE 10240
#endif

#include"Message.hpp"
#include"CellLog.hpp"

class CellServer;
