#pragma once
//2019.3.3
#include"Cell.hpp"
#include"CellBuffer.hpp"
#include"CellTimestamp.hpp"
//#include"CellObjectPool.hpp"

#define CLIENT_HEART_TIME 30000
//客户端数据类型
class CellClient //: public objectPoolBase<CellClient,20000>
{
public:
	int _serverId = -1;
	int _id = -1;
	CellClient(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SIZE, int recvSize = RECV_BUFF_SIZE)
	//	:_sendBuff(SEND_BUFF_SIZE),_recvBuff(RECV_BUFF_SIZE)
	{
		static int _count = 0;
		_id = _count++;
		_sockfd = sockfd;
		_pRecvBuff = new CellBuffer(RECV_BUFF_SIZE);
		_pSendBuff = new CellBuffer(SEND_BUFF_SIZE);
		resetHeart();
	}
	~CellClient() {
		Close();
	}
	void Close() {
		CellLog::Info("%d client %d Close\n", (_serverId<0)?_sockfd:_serverId, _id);
		if (_pRecvBuff) {
			delete _pRecvBuff;
			_pRecvBuff = nullptr;
		}
		if (_pSendBuff) {
			delete _pSendBuff;
			_pSendBuff = nullptr;
		}
		if (_sockfd != INVALID_SOCKET){
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}
	SOCKET sockfd() {
		return _sockfd;
	}
	int ReadData() {
		return _pRecvBuff->ReadToSocket(_sockfd);
	}
	bool hasMsg() {
		return _pRecvBuff->hasMsg();
	}
	msgHead* frontMsg()
	{
		return (msgHead*)_pRecvBuff->data();
	}

	void popFrontMsg()
	{
		if (hasMsg())
			_pRecvBuff->pop(frontMsg()->dataLen);
	}
	int sendFullCount() {
		return _pSendBuff->FullCount();
	}
	void resetSendTime() {
		_tSendTime.update();
	}
	bool isSendTime() {
		auto t = _tSendTime.getElapsedSecond();
		return (t >= 1.0);
	}
	bool isSend() {
		return _pSendBuff->hasMsg();
	}
	//实际发送数据
	int SendDataReal() {
		resetSendTime();
		return _pSendBuff->WriteToSocket(_sockfd);
	}
	//缓冲区发送
	int SendData(msgHead *header) {
		return SendData((const char *)header, header->dataLen);
	}
	int SendData(const char *pData,int len) {
		int ret = SOCKET_ERROR;
		if (pData) {
			//发送数据
			if (_pSendBuff->push(pData, len)) {
				return len;
			}
		}
		return ret;
	}
	void resetHeart() {
		_tHeartTime = 0;
	}
	bool checkHeart(time_t dt) {
		_tHeartTime += dt;
		if (_tHeartTime >= CLIENT_HEART_TIME) {
			CellLog::Info("over heart %d-%d\n",(int)_sockfd,(int)_tHeartTime);
			return true;
		}
		//CellLog::Info("heart %d-%d\n", (int)_sockfd, (int)_tHeartTime);
		return false;
	}
private:
	SOCKET _sockfd;
	//消息缓冲区
	CellBuffer* _pRecvBuff;
	//发送缓冲区
	CellBuffer* _pSendBuff;
	//
	CellTimestamp _tSendTime;
	time_t _tHeartTime;
};
