#pragma once

#include"Cell.hpp"
#include"InetEvent.hpp"
#include"CellThread.hpp"

//网络消息接收处理服务类
class CellServer
{
public:
	CellServer(int id) {
		_serverId = id;
		_taskServer._serverId = id;
		_pInetEvent = nullptr;
		//_isOnRun = false;
		_isFdReadChange = true;
	}
	~CellServer() {
		Close();
	}
	void setEventObj(InetEvent *event) {
		_pInetEvent = event;
	}
	void ClearClients()
	{
		for (auto iter : _clients) {
			delete iter.second;
		}
		_clients.clear();
		for (auto iter : _clientsBuff) {
			delete iter;
		}
		_clientsBuff.clear();
	}
	void Close() {
		CellLog::Info("CellServer<%d> Close begin\n",_serverId);
		_taskServer.Close();
		_thread.Close();
		CellLog::Info("CellServer<%d> Close end\n", _serverId);
	}
	//处理网络消息
	void OnRun(CellThread* pThread) {
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty()) {
				//客户缓冲-新客户端加入
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto client : _clientsBuff) {
					_clients[client->sockfd()] = client;
					client->_serverId = _serverId;
					if (_pInetEvent)
						_pInetEvent->OnJoin(client);
					client->resetHeart();
				}
				_clientsBuff.clear();
				_isFdReadChange = true;
			}
			if (_clients.empty()) {
				//无客户端，跳过
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				_oldTime = CellTime::getNowInMilliSec();
				continue;
			}
			//socket 集合
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdErr;
			//清空
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdErr);
			if (_isFdReadChange) {
				//设置
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients) {
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd()) {
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdReadBak, &fdRead, sizeof(fd_set));
				_isFdReadChange = false;
			}
			else {
				memcpy(&fdRead, &_fdReadBak, sizeof(fd_set));
			}
			memcpy(&fdWrite, &_fdReadBak, sizeof(fd_set));
			//memcpy(&fdErr, &_fdReadBak, sizeof(fd_set));
			//非阻塞
			timeval tout = { 0,1 };
			int ret = select((int)_maxSock + 1, &fdRead, &fdWrite, &fdErr, &tout);
			if (ret < 0) {
				CellLog::Info("CellServer%d.OnRun select error\n", _serverId);
				pThread->Exit();
				break;
			}
			//接收客户端数据
			ReadData(fdRead);
			//发送客户端数据
			WriteData(fdWrite);
			//异常数据
			ErrData(fdErr);
			//检查心跳
			CheckHeart();
		}
		CellLog::Info("CellServer%d.OnRun exit\n", _serverId);
	}
	//检查心跳
	void CheckHeart() {
		auto nowTime = CellTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		if (dt >= 1000) {
			std::vector<CellClient*> temp;
			for (auto iter : _clients)
			{
				if (iter.second->checkHeart(dt))
				{
//					if (_pInetEvent)
//						_pInetEvent->OnLeave(iter.second);
//					_isFdReadChange = true;
					temp.push_back(iter.second);
				}
			}
			for (auto client : temp)
			{//发送心跳
				msgHeartS *heart = new msgHeartS();
				client->SendData((msgHead *)heart);
				delete heart;
				client->resetHeart();
				//CellLog::Info("send heart %d\n", client->sockfd());
				//				Close(client->sockfd());
//				_clients.erase(client->sockfd());
				//delete client;
			}
			temp.clear();
			//
			_oldTime = nowTime;
		}
	}
	//处理异常
	void ErrData(fd_set& fdErr) {
		//选择读或写
		//发送心跳判断是否异常
#ifdef _WIN32

#else

#endif
	}
	void OnClientLeave(CellClient* client) {
		if (_pInetEvent)
			_pInetEvent->OnLeave(client);
		_isFdReadChange = true;
		delete client;
	}
	//处理发送数据
	void WriteData(fd_set& fdWrite) {
#ifdef _WIN32
		for (size_t n = 0; n < fdWrite.fd_count; n++)
		{//可发送
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
//			else {
//				CellLog::Info("WriteData not end error. \n");
//			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	//处理接收数据
	void ReadData(fd_set& fdRead) {
#ifdef _WIN32
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
//			else {
//				CellLog::Info("ReadData not end error. \n");
//			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	//接收数据
	int RecvData(CellClient *client) {
		//读取消息
		if(client->ReadData()<=0){
			CellLog::Info("client <Socket=%d> end,exit \n", (int)client->sockfd());
			return -1;
		}
		_pInetEvent->OnNetRecv(client);
		//判断消息头
		while (client->hasMsg())
		{
			//处理消息
			OnNetMsg(client, client->frontMsg());
			//移出消息
			client->popFrontMsg();
		}
		return 0;
	}
	//响应消息
	virtual void OnNetMsg(CellClient* client, msgHead *header) {
		_pInetEvent->OnNetMsg(this, client, header);
	}
	void addClient(CellClient *client) {
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(client);
	}
	void Start() {
		_taskServer.Start();
		_thread.Start(
			nullptr, 
			[this](CellThread* pThread) {	
				OnRun(pThread);	
				},
			[this](CellThread* pThread) {
				ClearClients();
				}
			);
	}
	size_t getClientCount() {
		return _clients.size() + _clientsBuff.size();
	}
	void addSendTask(CellClient* client, msgHead* header) {
		//lambda实现任务传递
		_taskServer.addTask([client,header]() {
			client->SendData(header);
			delete header;
		});
	}
private:
	//正式队列
	std::map<SOCKET,CellClient*> _clients;
	//缓冲队列
	std::vector<CellClient*> _clientsBuff;
	std::mutex _mutex;
	//网络事件对象
	InetEvent *_pInetEvent;
	//任务
	CellTaskServer _taskServer;
	//备份客户
	fd_set _fdReadBak;
	SOCKET _maxSock;
	//cellServer Id
	int _serverId = -1;
	//旧的是时间戳
	time_t _oldTime;
	//客户是否变化
	bool _isFdReadChange;
	//线程
	CellThread _thread;

};
