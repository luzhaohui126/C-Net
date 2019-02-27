#pragma once

#include"Cell.hpp"
#include"CellTimestamp.hpp"
#include"CellTask.hpp"
#include"CellClient.hpp"
#include"CellServer.hpp"
#include"CellThread.hpp"
#include"CellNetwork.hpp"

//TCP服务类
class EasyTcpServer :public InetEvent
{
private:
	//线程-多server
	CellThread _thread;
	//socket,ip,port
	SOCKET _sock;
	unsigned int _addr;
	int _port;
	//每秒消息计数
	CellTimestamp _tTime;
	//消息处理对象，内部线程
	std::vector<CellServer*> _cellServers;

protected:
	std::atomic_int _recvCount;
	std::atomic_int _sendCount;
	std::atomic_int _msgCount;
	std::atomic_int _clientCount;
public:
	void Init() {
		_sock = INVALID_SOCKET;
		_addr = INADDR_ANY;
		_port = 4567;
		_sendCount = 0;
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
	}
	EasyTcpServer() {
		Init();
	}
	EasyTcpServer(int port) {
		Init();
		_port = port;
	}
	EasyTcpServer(const char *ip, int port) {
		Init();
		if (ip) {
			_addr = inet_addr(ip);
		}
		else {
			_addr = INADDR_ANY;
		}
		_port = port;
	}
	~EasyTcpServer() {
		Close();
	}
	//初始化Socket
	SOCKET InitSocket() {
		CellNetwork::Init();
		if (isRun()) {
			//关闭
			Close();
		}
		//建立一个socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			CellLog::Info("Err:invalid socket \n");
		}
		else {
			CellLog::Info("Ok: create socket %d\n", (int)_sock);
		}
		return _sock;
	}
	//绑定端口
	bool Bind() {
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(_port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = _addr;
#else
		_sin.sin_addr.s_addr = _addr;
#endif
		if (SOCKET_ERROR == bind(_sock, (sockaddr *)&_sin, sizeof(_sin))) {
			CellLog::Info("Err:error %d bind %d\n", (int)_sock, _port);
			return false;
		}
		else {
			CellLog::Info("Ok: socket %d bind %d\n", (int)_sock, _port);
		}
		return true;
	}
	//监听端口
	bool Listen(int nListen) {
		int ret = listen(_sock, nListen);
		if (SOCKET_ERROR == ret) {
			CellLog::Info("Err:%d port=%d listen error %d\n", (int)_sock, _port, nListen);
		}
		else {
			CellLog::Info("Ok:%d port=%d listen %d \n", (int)_sock, _port, nListen);
		}
		return ret;
	}
	//接受客户端连接
	SOCKET Accept() {
		sockaddr_in clientAddr = {};
#ifdef _WIN32
		int nAddrLen = sizeof(clientAddr);
#else
		socklen_t nAddrLen = sizeof(clientAddr);
#endif
		SOCKET clientSock = INVALID_SOCKET;
		clientSock = accept(_sock, (sockaddr *)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == clientSock) {
			CellLog::Info("Err:%d accept invalid socket %d\n", (int)_sock, (int)clientSock);
		}
		else {
			addClientToServer(new CellClient(clientSock));
			//获取IP:inet_ntoa(clientAddr.in_addr)
		}
		return clientSock;
	}
	void addClientToServer(CellClient* client) {
		//最少缓冲队列优先
		auto pMin = _cellServers[0];
		for (auto ser : _cellServers) {
			if (pMin->getClientCount() > ser->getClientCount()) {
				pMin = ser;
			}
		}
		pMin->addClient(client);
	}
	void Start(int nCellServer) {
		for (int i = 0; i < nCellServer; i++)
		{
			auto server = new CellServer(i+1);		//2:-20376,1-64,...7-64
			//auto server = std::make_shared<CellServer>(_sock);
			_cellServers.push_back(server);
			//注册事件对象
			server->setEventObj(this);
			//启动服务线程-释放问题
			server->Start();
		}
		_thread.Start(
			nullptr,
			[this](CellThread* pThread) {
				OnRun(pThread);
			});
	}
	//关闭
	void Close() {
		CellLog::Info("EasyTcpServer Close Begin\n");
		_thread.Close();
		if (isRun()) {
			CellLog::Info("close socket %d \n", (int)_sock);
#ifdef _WIN32
			closesocket(_sock);
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
			for (auto iter:_cellServers) {
				delete iter;
			}
			_cellServers.clear();
		}
		CellLog::Info("EasyTcpServer Close End\n");
	}
	//处理网络消息
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//计算并输出每秒收到的网络消息
	void time4msg()
	{
		//包计数
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0) {
			CellLog::Info("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recv<%d>,msg<%d>,send<%d>\n",
				(int)_cellServers.size(), t1, (int)_sock, (int)_clientCount,(int)_recvCount, (int)_msgCount,(int)_sendCount);
			_tTime.update();
			_sendCount = 0;
			_recvCount = 0;
			_msgCount = 0;
		}
	}
	//客户端离开
	virtual void OnLeave(CellClient* client) {
		_clientCount--;
	}
	//客户端加入
	virtual void OnJoin(CellClient* client) {
		_clientCount++;
	}
	//消息事件
	virtual void OnNetMsg(CellServer *pCellServer, CellClient* client, msgHead* header) {
			_msgCount++;
	}
	//接收事件
	virtual void OnNetRecv(CellClient* client) {
		_recvCount++;
	}
	//发送事件
	virtual void OnNetSend(CellClient* client) {
		_sendCount++;
	}
private:
	void OnRun(CellThread* pThread) {
		while(pThread->isRun()){
			time4msg();
			//socket 集合
			fd_set fdRead;
			//清空
			FD_ZERO(&fdRead);
			//设置
			FD_SET(_sock, &fdRead);
			//非阻塞
			timeval tout = { 0,1 };
			int ret = select((int)_sock + 1, &fdRead, nullptr, nullptr, &tout);
			if (ret < 0) {
				CellLog::Info("TcpServer.OnRun select error\n");
				pThread->Exit();
				break;
			}
			if (FD_ISSET(_sock, &fdRead)) {
				//有接收
				FD_CLR(_sock, &fdRead);
				// 4 accept 等待接受客户端连接
				Accept();
			}
		}
		CellLog::Info("TcpServer.OnRun exit\n");
	}

};
