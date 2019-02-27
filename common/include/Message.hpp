#pragma once
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER,
	CMD_HEART_C2S,
	CMD_HEART_S2C,
	CMD_ERR
};
//msgHeader
struct msgHead {
	msgHead() {
		dataLen = sizeof(msgHead);
		cmdCode = CMD_ERR;
	}
	unsigned short dataLen;
	unsigned short cmdCode;
};
//DataPackage
struct msgLogin :public msgHead {
	msgLogin() {
		dataLen = sizeof(msgLogin);
		cmdCode = CMD_LOGIN;
	}
	char userName[32];
	char userPass[32];
	char data[32];
};
struct msgLoginResult :public msgHead {
	msgLoginResult() {
		dataLen = sizeof(msgLoginResult);
		cmdCode = CMD_LOGIN_RESULT;
		resState = 0;
		resMsg[0] = 0;
	}
	int resState;
	char resMsg[92];
};
struct msgLogout :public msgHead {
	msgLogout() {
		dataLen = sizeof(msgLogout);
		cmdCode = CMD_LOGOUT;
	}
	char userName[32];
};
struct msgLogoutResult :public msgHead {
	msgLogoutResult() {
		dataLen = sizeof(msgLogoutResult);
		cmdCode = CMD_LOGOUT_RESULT;
		resState = 0;
	}
	int resState;
};
struct msgNewUser :public msgHead {
	msgNewUser() {
		dataLen = sizeof(msgNewUser);
		cmdCode = CMD_NEW_USER;
		sock = 0;
	}
	SOCKET sock;
};
struct msgHeartC :public msgHead {
	msgHeartC() {
		dataLen = sizeof(msgHeartC);
		cmdCode = CMD_HEART_C2S;
	}
};
struct msgHeartS :public msgHead {
	msgHeartS() {
		dataLen = sizeof(msgHeartS);
		cmdCode = CMD_HEART_S2C;
	}
};
