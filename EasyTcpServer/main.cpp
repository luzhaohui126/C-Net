//2019.3.3
//#include"Alloctor.h"
#include"Cell.hpp"
#include"EasyTcpServer.hpp"
#include"CellMsgStream.hpp"
#include<stdlib.h>
#include<thread>

class MyServer : public EasyTcpServer
{
public:
	MyServer(int port) : EasyTcpServer(port)
	{
	}
	~MyServer() {

	}
	virtual void OnJoin(CellClient* client)
	{
		EasyTcpServer::OnJoin(client);
	}
	virtual void OnLeave(CellClient* client)
	{
		EasyTcpServer::OnLeave(client);
	}
	virtual void OnNetMsg(CellServer *pCellServer, CellClient* client, msgHead* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer,client,header);
		//重置心跳
		client->resetHeart();
		switch (header->cmdCode)
		{
		case CMD_LOGIN:
			{
				//CellLog::Info("recv login\n");
				msgLogin *login = (msgLogin *)header;
				//
				//msgLoginResult *res = new msgLoginResult();
				//strcpy(res->resMsg, "登录成功");
				//pCellServer->addSendTask(client,(msgHead *)res);
				msgLoginResult res;
				strcpy(res.resMsg, "登录成功");
				if (SOCKET_ERROR == client->SendData(&res)) {
					//发送缓冲区满
					CellLog::Info("send full %d\n",client->sendFullCount());
				}
				else {
					OnNetSend(client);
				}
			}
			break;
		case CMD_LOGOUT:
			{
				CellReadStream r(header);
				auto len = r.ReadUint16();
				auto cmd = r.getMsgCmd();
				printf("len=%d,cmd=%d\n", len, cmd);
				auto n1 = r.ReadInt8();
				auto n2 = r.ReadInt16();
				auto n3 = r.ReadInt32();
				auto n4 = r.ReadInt64();
				auto n5 = r.ReadFloat();
				auto n6 = r.ReadDouble();
				printf("n1=%d,n2=%d,n3=%d,n4=%d,n5=%f,n6=%f\n", n1, n2, n3, n4, n5, n6);
				char sz[32] = {};
				uint32_t sn = 0;
				sn = r.ReadArray(sz, sizeof(sz));
				sz[sn] = 0;
				printf("%d %s\n", sn, sz);
				int nd[10] = {};
				sn = r.ReadArray(nd, 10);
				printf("%d", sn);
				for (int n = 0; n < sn; n++)
				{
					printf(",%d ", nd[n]);
				}
				//msgLogout *logout = (msgLogout *)header;
				//			CellLog::Info("%d logout %d %s \n", (int)clientSock, logout->dataLen, logout->userName);
				//LogoutResult res;
				//_sendCount += pClient->SendData((Head *)&res);
				CellWriteStream s;
				s.setMsgCmd(CMD_LOGOUT_RESULT);
				s.WriteInt8(n1);
				s.WriteInt16(n2);
				s.WriteInt32(n3);
				s.WriteInt64(n4);
				s.WriteFloat(n5);
				s.WriteDouble(n6);
				s.WriteString(sz);
				s.WriteArray(nd, sn);
				s.finish();
				client->SendData(s.data(), (int)s.length());
			}
			break;
		case CMD_HEART_C2S:
			{
				msgHeartC *res = (msgHeartC *)header;
				client->SendData((msgHead *)res);
			}
			break;
		case CMD_HEART_S2C:
			{
				msgHeartS *res = (msgHeartS *)header;
				CellLog::Info("%d reply heart\n", (int)client->sockfd());
			}
			break;
		default:
		{
			//CellLog::Info("%d err cmd %d %d\n", (int)pClient->sockGet(), header->cmdCode, header->dataLen);
			//Head err;
			//_sendCount += pClient->SendData(&err);
		}
		break;
		}
	}
	virtual void OnNetRecv(CellClient* client) {
			EasyTcpServer::OnNetRecv(client);
	}
	virtual void OnNetSend(CellClient* client) {
		EasyTcpServer::OnNetSend(client);
	}
private:
};

int main(int argc, char *argv[]) {
	CellLog::Instance().setLogPath("server.txt", "a");
	int port=4567;
	if(argc>=2){
		int p=atoi(argv[1]);
		if(p>=1000&&p<=9999){
			port=p;
		}
	}
	MyServer server(port);		//1:0-64

	server.InitSocket();
	server.Bind();
	server.Listen(16);
	server.Start(4);
	//主线程获取命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			break;
		}
	}
	server.Close();
	CellLog::Info("exit,task end\n");
	//while (true)Sleep(1);
	return 0;
}
