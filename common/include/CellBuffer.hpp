#pragma once
#include"Cell.hpp"
//2019.3.3
class CellBuffer
{
public:
	CellBuffer(int nSize = 1024) {
		_nLast = 0;
		_nSize = nSize;
		_fullCount = 0;
		_pBuff = new char[_nSize];

	}
	~CellBuffer() {
		if (_pBuff) {
			delete _pBuff;
			_pBuff = nullptr;
		}
	}
	char* data()
	{
		return _pBuff;
	}
	bool push(const char *pData,const int nLen) {
		if ((_nLast + nLen) <= _nSize) {
			//存入发送缓冲区
			memcpy(_pBuff + _nLast, pData, nLen);
			_nLast += nLen;
			if (_nLast == _nSize) {
				++_fullCount;
			}
			return true;
		}
		else {
			//缓冲区满-拓展
			++_fullCount;
		}
		return false;
	}
	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0){
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		else {
			n = 0;
		}
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}

	int WriteToSocket(SOCKET sock) {
		int ret = 0;
		if (_nLast > 0 && INVALID_SOCKET != sock) {
			ret = send(sock, _pBuff, _nLast, 0);
			_nLast = 0;
			_fullCount = 0;
		}
		return ret;
	}
	int ReadToSocket(SOCKET sock) {
		if (_nSize - _nLast) {
			char *szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sock, szRecv, _nSize - _nLast, 0);
			if (nLen > 0) {
				_nLast += nLen;
			}
			return nLen;
		}
		return -1;
	}
	int FullCount() {
		return _fullCount;
	}
	bool hasMsg() {
		if (_nLast>= sizeof(msgHead))
		{
			msgHead* header = (msgHead*)_pBuff;
			//判断消息缓冲区的数据长度大于消息长度
			return (_nLast >= header->dataLen);
		}
		return false;
	}
private:
	char *_pBuff;
	int _nLast;
	int _nSize;
	int _fullCount;
};
