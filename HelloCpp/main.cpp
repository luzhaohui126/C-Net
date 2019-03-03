//2019.3.3
#include"EasyTcpClient.hpp"
#include"CellMsgStream.hpp"
//function
//lambda

//int funA(int a,int b) {
//	printf("funA\n");
//	return 1;
//}

class MyClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(msgHead *header) {
		_msgCount++;
		switch (header->cmdCode)
		{
		case CMD_LOGIN_RESULT:
		{
			//msgLoginResult *res = (msgLoginResult *)header;
			//printf("%d login result %d %d\n", (int)_sock, res->dataLen, res->resState);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			CellReadStream r(header);
			auto len = r.ReadUint16();
			auto cmd = r.getMsgCmd();
			printf("len=%d cmd=%d\n", len, cmd);
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
			msgLogoutResult *res = (msgLogoutResult *)header;
			printf("\n%d logout result %d %d\n", (int)_pClient->sockfd(), res->dataLen, res->resState);
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


int main()
{
	CellWriteStream s;
	s.setMsgCmd(CMD_LOGOUT);
	s.WriteInt8(11);
	s.WriteInt16(22);
	s.WriteInt32(33);
	s.WriteInt64(44);
	s.WriteFloat(55.05f);
	s.WriteDouble(66.06);
	s.WriteString("abcdef");
	int b[] = { 1,2,3,4,5,6,7,8,9 };
	s.WriteArray(b, 8);
	s.finish();

	MyClient client;
	client.Connect("127.0.0.1",4567);
	client.SendData(s.data(),(int)s.length());
	while (client.isRun())
	{
		client.OnRun();
		CellThread::Sleep(10);
	}

	//std::list<int>a;
	//std::function<int(int,int) >call = funA;
	//int n=call(0,1);

	//std::function<void ()>call;
	//call = []() ->int{
	//	printf("call\n");
	//	return 0;
	//};
	//call();

//	while (true)
//	{
//		Sleep(1);
//	}
	getchar();
	return 0;
}

