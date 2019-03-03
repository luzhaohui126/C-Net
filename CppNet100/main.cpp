#pragma once
//2019.3.3
#include<string>
#include"EasyTcpClient.hpp"
#include"CellMsgStream.hpp"

#ifdef _WIN32
	#define EXPORT_DLL _declspec(dllexport)
#else
	#define EXPORT_DLL
#endif

extern "C"
{
	typedef void(*OnNetMsgCallBack)(void* csObj, void* data, int len);
}

class NativeTcpClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(msgHead *header) {
		if (_callBack) {
			_callBack(_csObj, header, header->dataLen);
		}
	}
	void setCallBack(void *csObj, OnNetMsgCallBack cb) {
		_csObj = csObj;
		_callBack = cb;
	}
private:
	void* _csObj = nullptr;
	OnNetMsgCallBack _callBack = nullptr;
};

extern "C"
{
	/////////////////////////////////////CellClient
	EXPORT_DLL void* CellClientCreate(void* csObj, OnNetMsgCallBack cb, int sendSize = SEND_BUFF_SIZE, int recvSize = RECV_BUFF_SIZE) {
		NativeTcpClient* pClient = new NativeTcpClient();
		pClient->setCallBack(csObj, cb);
		pClient->InitSocket(sendSize, recvSize);
		return pClient;
	}
	EXPORT_DLL bool CellClientConnect(NativeTcpClient* pClient, const char *ip, int port) {
		if (pClient && ip) {
			return pClient->Connect(ip, port) != SOCKET_ERROR;
		}
		return false;
	}
	EXPORT_DLL bool CellClientOnRun(NativeTcpClient* pClient) {
		if (pClient) {
			return pClient->OnRun();
		}
		return false;
	}
	EXPORT_DLL void CellClientClose(NativeTcpClient* pClient) {
		if (pClient) {
			pClient->Close();
			delete pClient;
			pClient = nullptr;
		}
	}
	EXPORT_DLL int CellClientSend(NativeTcpClient* pClient, const char *pData, int len) {
		if (pClient) {
			return pClient->SendData(pData, len);
		}
		return SOCKET_ERROR;
	}
	EXPORT_DLL int CellClientSendStream(NativeTcpClient* pClient, CellWriteStream* wStream) {
		if (pClient&&wStream) {
			wStream->finish();
			return pClient->SendData(wStream->data(), (int)wStream->length());
		}
		return SOCKET_ERROR;
	}

	////////////////////////////////////CellStream
	////////CellWriteStream
	EXPORT_DLL void* CellWriteStreamCreate(int nSize) {
		CellWriteStream* wStream = new CellWriteStream();
		return wStream;
	}
	EXPORT_DLL bool CellWriteStreamWriteInt8(CellWriteStream* wStream, int8_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteInt16(CellWriteStream* wStream, int16_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteInt32(CellWriteStream* wStream, int32_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteInt64(CellWriteStream* wStream, int64_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteUInt8(CellWriteStream* wStream, uint8_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteUInt16(CellWriteStream* wStream, uint16_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteUInt32(CellWriteStream* wStream, uint32_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteUInt64(CellWriteStream* wStream, uint64_t n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteFloat(CellWriteStream* wStream, float n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteDouble(CellWriteStream* wStream, double n) {
		if (wStream) {
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStreamWriteString(CellWriteStream* wStream, char* n, int len) {
		if (wStream) {
			return wStream->WriteArray(n, len);
		}
		return false;
	}

	////////CellReadStream
	EXPORT_DLL void* CellReadStreamCreate(char *data, int nSize) {
		CellReadStream* rStream = new CellReadStream(data, nSize);
		return rStream;
	}
	EXPORT_DLL int CellReadStreamPos(CellReadStream* rStream) {
		return rStream->getReadPos();
	}
	EXPORT_DLL int CellReadStreamSize(CellReadStream* rStream) {
		return rStream->getSize();
	}
	EXPORT_DLL int8_t CellReadStreamReadInt8(CellReadStream* rStream, int8_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL uint8_t CellReadStreamReadUInt8(CellReadStream* rStream, uint8_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL int16_t CellReadStreamReadInt16(CellReadStream* rStream, int16_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL uint16_t CellReadStreamReadUInt16(CellReadStream* rStream, uint16_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL int32_t CellReadStreamReadInt32(CellReadStream* rStream, int32_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL uint32_t CellReadStreamReadUInt32(CellReadStream* rStream, uint32_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL uint32_t CellReadStreamOnlyReadUInt32(CellReadStream* rStream, uint32_t n = 0) {
		if (rStream) {
			return rStream->Read(n, false);
		}
		return n;
	}
	EXPORT_DLL int64_t CellReadStreamReadInt64(CellReadStream* rStream, int64_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL uint64_t CellReadStreamReadUInt64(CellReadStream* rStream, uint64_t n = 0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL float CellReadStreamReadFloat(CellReadStream* rStream, float n = 0.0f) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL double CellReadStreamReadDouble(CellReadStream* rStream, double n = 0.0) {
		if (rStream) {
			return rStream->Read(n);
		}
		return n;
	}
	EXPORT_DLL uint32_t CellReadStreamReadString(CellReadStream* rStream, char *data, uint32_t len) {
		if (data && rStream)
		{
			return rStream->Read(data, len);
		}
		return 0;
	}
}

extern "C"
{
	int _declspec(dllexport) Add(int a, int b) {
		return a + b;
	}

	typedef void(*CallBack1)(const char* str);

	void _declspec(dllexport) TestCall1(const char* str, CallBack1 cb) {
			std::string s = "Hello ";
			s += str;
			cb(s.c_str());
		}
}
