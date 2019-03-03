#pragma once

#include"Cell.hpp"
#include"CellNetwork.hpp"
#include"Message.hpp"
#include"CellTimestamp.hpp"
#include"CellClient.hpp"

#include<stdlib.h>
#include<atomic>

class EasyTcpClient
{
public:
	std::atomic_int _recvCount;
	std::atomic_int _msgCount;
	void Init() {
		_recvCount = 0;
		_msgCount = 0;
		_pClient = nullptr;
		_isConn = false;
	}
	EasyTcpClient() {
		Init();
	}
	virtual ~EasyTcpClient() {
		Close();
	}
	//关闭
	void Close() {
		if (_pClient) {
			CellLog::Info("<%d> close socket \n", _pClient->sockfd());
			delete _pClient;
			_pClient = nullptr;
		}
		_isConn = false;
	}
	//初始化
	void InitSocket(int sendSize = SEND_BUFF_SIZE, int recvSize = RECV_BUFF_SIZE) {
		CellNetwork::Init();

		if (_pClient) {
			Close();
		}
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			CellLog::Info("Err:invalid socket \n");
		}
		else {
			//printf("Ok: create socket %d\n",(int)_sock);
			_pClient = new CellClient(_sock, sendSize, recvSize);
		}
	}
	//连接服务器
	int Connect(const char *ip,int port) {
		if (!_pClient) {
			InitSocket();
		}
		sockaddr_in _sout = {};
		_sout.sin_family = AF_INET;
		_sout.sin_port = htons(port);
#ifdef	_WIN32
		_sout.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sout.sin_addr.s_addr = inet_addr(_ip);
#endif
		int ret = connect(_pClient->sockfd(), (sockaddr *)&_sout, sizeof(_sout));
		if (SOCKET_ERROR == ret) {
			_isConn = false;
			CellLog::Info("%d Err: connect %s:%d\n", (int)_pClient->sockfd(),ip,port);
		}
		else {
			_isConn = true;
			//CellLog::Info("%d Ok: connect %s:%d\n", (int)_pClient->sockfd(),ip,port);
		}
		return ret;
	}
	//查询
	bool OnRun() {
		if (!isRun()) {
			//未连接
			return false;
		}
		//socket set
		SOCKET sock = _pClient->sockfd();

		fd_set fdRead;
		fd_set fdWrite;
		//clear
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		//setup
		FD_SET(sock, &fdRead);
		if (_pClient->isSend()) {
			FD_SET(sock, &fdWrite);
		}
		//no block
		timeval tout = { 0,1 };
		int ret = select((int)sock + 1, &fdRead, &fdWrite, nullptr, &tout);
		if (ret < 0) {
			CellLog::Info("%d select end\n", (int)sock);
			Close();
			return false;
		}
		if (FD_ISSET(sock, &fdRead)) {
			//have recv
			FD_CLR(sock, &fdRead);
			if (-1 == RecvData()) {
				CellLog::Info("%d server read end\n", (int)sock);
				Close();
				return false;
			}
		}
		if (FD_ISSET(sock, &fdWrite)) {
			FD_CLR(sock, &fdWrite);
			if (-1 == _pClient->SendDataReal()) {
				CellLog::Info("%d server write end\n", (int)sock);
				Close();
				return false;
			}
		}
		return true;
	}
	//接收缓冲区
	int RecvData() {
		if (isRun()) {
			int nLen = _pClient->ReadData();
			_recvCount++;
			if (nLen <= 0) {
				CellLog::Info("%d connect break , task end\n", (int)_pClient->sockfd());
				Close();
				return -1;
			}
			while (_pClient->hasMsg())
			{
				OnNetMsg((msgHead *)_pClient->frontMsg());
				_pClient->popFrontMsg();
			}
		}
		return 0;
	}
	//是否工作中
	bool isRun()
	{
		return _pClient && _isConn;
	}
	//响应网络消息
	virtual void OnNetMsg(msgHead *header) = 0;
	//发送
	int SendData(msgHead *header) {
		return SendData((const char *)header,header->dataLen);
	}
	int SendData(const char *pData, int len) {
		if (!isRun())
			return SOCKET_ERROR;
		return _pClient->SendData(pData,len);
	}
	protected:
	CellClient *_pClient;
private:
	bool _isConn;
};

