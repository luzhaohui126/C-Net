#pragma once

//网络事件接口
class InetEvent
{
public:
	//客户端离开
	virtual void OnLeave(CellClient* client) = 0;
	//客户端加入
	virtual void OnJoin(CellClient* client) = 0;
	//消息事件
	virtual void OnNetMsg(CellServer *pCellServer, CellClient* cClient, msgHead* header) = 0;
	//接收事件
	virtual void OnNetRecv(CellClient* client) = 0;
	//发送事件
	virtual void OnNetSend(CellClient* client) = 0;
private:
};
