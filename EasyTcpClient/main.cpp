//2019.3.3
//  main.cpp
//  client
//
//  Created by 鲁 肇慧 on 2019/2/2.
//  Copyright © 2019年 鲁 肇慧. All rights reserved.
//
#include"EasyTcpClient.hpp"

bool SystemExit = true;
int nStop = 0;

class MyClient: public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(msgHead *header) {
		_msgCount++;
		switch (header->cmdCode)
		{
		case CMD_LOGIN_RESULT:
		{
			msgLoginResult *res = (msgLoginResult *)header;
			//printf("%d login result %d %d\n", (int)_sock, res->dataLen, res->resState);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			msgLogoutResult *res = (msgLogoutResult *)header;
			printf("%d logout result %d %d\n", (int)_pClient->sockfd(), res->dataLen, res->resState);
		}
		break;
		case CMD_NEW_USER:
		{
			msgNewUser *res = (msgNewUser *)header;
			printf("%d new user %d  socket=%d\n", (int)_pClient->sockfd(), res->dataLen, (int)res->sock);
		}
		break;
		case CMD_HEART_S2C:
		{
			msgHeartS *res = (msgHeartS *)header;
			//回复
			int ret = SendData(res);
			//printf("%d reply heart %d send %d\n", (int)_sock, res->dataLen,ret);
		}
		break;
		}
	}
private:

};

//客户端数量
const int cCount = 10000;
//发送线程数量
const int tCount = 4;
//发送包数
const int pCount = 100000;
//客户端数组
EasyTcpClient* client[cCount];

CellTimestamp _tTime1;
CellTimestamp _tTime2;
std::atomic_int _readyCount = { 0 };
std::atomic_int _sendCount = { 0 };
int minDelay = 10;
std::atomic_int _delayTime = { 200 };

void recvThread(CellThread *pThread,int begin,int end) {
	CellLog::Info("recv thread - %d-%d\n", begin, end);
	while (pThread->isRun())
	{
		for (int n = begin; n < end; n++)
		{
			if (nStop) {
				if(0==(n%(nStop+1)))
					continue;
			}
			client[n]->OnRun();
		}
	}
	CellLog::Info("recv thread - %d-%d exit\n", begin, end);
}

void showThread(CellThread *pThread) {
	while (pThread->isRun()) {
		auto t1 = _tTime2.getElapsedSecond();
		if (t1 >= 1.0) {
			int recvCount = 0;
			int msgCount = 0;
			int clientCount = 0;
			for (int n = 0; n < cCount; n++) {
				if (client[n]->isRun())clientCount++;
				recvCount += client[n]->_recvCount;
				msgCount += client[n]->_msgCount;
				client[n]->_recvCount = 0;
				client[n]->_msgCount = 0;
			}
			CellLog::Info("thread<%d>,time<%lf>,clients<%d-%d>,recv<%d>,msg<%d>,send<%d> %d\n",
				(int)_readyCount, t1, cCount, clientCount, recvCount, msgCount, (int)_sendCount, (int)_delayTime);
			if (_sendCount) {
				if (_sendCount < pCount) {
					if (_delayTime >= minDelay)
						_delayTime -= minDelay;
				}
				else {
					_delayTime += minDelay;
				}
				if (0 == _delayTime) {
					minDelay = 1;
				}
			}
			_sendCount = 0;
			_tTime2.update();
		}
		std::chrono::milliseconds t(1);
		std::this_thread::sleep_for(t);
	}
}

void sendThread(CellThread *pThread,int id,const char *ip,int port) {
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;
	const int nLen = 10;
	msgLogin login[nLen];
	//
	CellLog::Info("send pthread<%d> %d-%d\n", id,begin,end);
	for (int n = begin; n < end; n++)
	{
		client[n] = new MyClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Connect(ip,port);
	}
	CellLog::Info("pthread<%d> Connect <%d-%d>\n", id, begin, end);
	_readyCount++;
	while (_readyCount<tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	//recv
	//std::thread t(recvThread, begin,end);
	//t.detach();
	CellThread thread;
	thread.Start(
		nullptr,
		[=](CellThread *pThread) {
			recvThread(pThread, begin, end);
		}
	);
	for (size_t i = 0; i < nLen; i++)
	{
		strcpy(login[i].userName, "lzh");
		strcpy(login[i].userPass, "020630");
	}
	int ret;
	int sc;
	while (pThread->isRun())
	{
		sc = 0;
		for (int n = begin; n < end; n++)
		{
			//ret = client[n]->SendData((msgHead *)&login,sizeof(login));
			ret = client[n]->SendData((msgHead *)&login);
			if (SOCKET_ERROR != ret) {
				_sendCount++;
				sc = 0;
			}
		}
		std::chrono::milliseconds t(_delayTime);
		std::this_thread::sleep_for(t);
	}
	thread.Close();
	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}
	CellLog::Info("send pthread<%d> exit\n", id);
}

int main(int argc,char *argv[]) {
	CellLog::Instance().setLogPath("client.txt", "a");
	char ip[32]="127.0.0.1";
	//char ip[32] = "192.168.2.211";
	int port=4567;
	if(argc>=2){
		int p=atoi(argv[1]);
		if(p>=1000&&p<=9999){
			port=p;
		}
	}
	if(argc>=3){
		if(strlen(argv[2])<16){
			strcpy(ip,argv[2]);
		}
	}
	CellThread threadShow;

	threadShow.Start(
		nullptr,
		[=](CellThread *pThread) {
		showThread(pThread);
		}
	);

	CellThread thread[tCount];

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		thread[n].Start(
			nullptr,
			[=](CellThread *pThread) {
				sendThread(pThread, n + 1, ip, port);
			}
		);
	}

	//主线程获取命令
	char cmdBuf[256] = {};
	while (true)
	{
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			SystemExit = false;
			break;
		}
		else if (0 == strcmp(cmdBuf, "0")) {
			nStop = 0;
		}
		else if (0 == strcmp(cmdBuf, "1")) {
			nStop = 1;
			printf("stop 1\n");
		}
		else if (0 == strcmp(cmdBuf, "2")) {
			nStop = 2;
			printf("stop 2\n");
		}
		else {
			printf("err cmd\n");
		}
	}
	threadShow.Close();
	for (int n = 0; n < tCount; n++) {
		thread[n].Close();
	}
	CellLog::Info("exit,task end\n");
    return 0;
}
