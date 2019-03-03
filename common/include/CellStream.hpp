#pragma once
//2019.3.3
#include<cstdint>
#include<string.h>
#include"CellLog.hpp"
//字节流
class CellStream
{
public:
	void Init() {
		_nWritePos = 0;
		_nReadPos = 0;
		_bDelete = true;
	}
	CellStream(char *pData, int nSize, bool bDelete = false) {
		Init();
		_nSize = nSize;
		_nWritePos = nSize;
		_pBuff = pData;
		_bDelete = bDelete;
	}
	CellStream(int nSize = 4096) {
		Init();
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}
	virtual ~CellStream() {
		if (_bDelete && _pBuff) {
			delete _pBuff;
			_pBuff = nullptr;
		}
	}
	char* data() {
		return _pBuff;
	}
	size_t length() {
		return _nWritePos;
	}
	inline void setWritePos(int n) {
		_nWritePos = n;
	}
	inline int getWritePos() {
		return _nWritePos;
	}
	inline int getReadPos() {
		return _nReadPos;
	}
	inline int getSize() {
		return _nSize;
	}
	inline bool canRead(int n) {
		return (_nReadPos + n) <= _nWritePos;
	}
	inline bool canWrite(int n) {
		return (_nWritePos + n) <= _nSize;
	}
	inline void push(int n) {
		_nWritePos += n;
	}
	inline void pop(int n) {
		_nReadPos += n;
	}
	inline void push(char *pData, int nLen) {
		memcpy(_pBuff + _nWritePos, pData, nLen);
	}
	inline void pop(char *pData, int nLen) {
		memcpy(pData, _pBuff + _nReadPos, nLen);
	}
public:
	template<typename T>
	bool Read(T& n, bool bOffset = true) {
		auto nLen = sizeof(T);
		if (canRead((int)nLen)) {
			//读取缓冲区
			pop((char *)&n, (int)nLen);
			if (bOffset)
				pop((int)nLen);
			return true;
		}
		CellLog::Info("CellStream Read error\n");
		return false;
	}
	template<typename T>
	uint32_t ReadArray(T* pData,uint32_t len) {
		uint32_t rLen;
		if (Read<uint32_t>(rLen,false)) {
			auto nLen = sizeof(T)*rLen;
			if (canRead( (int)nLen + sizeof(uint32_t)))
			{	//读取缓冲区
				pop(sizeof(uint32_t));
				pop((char *)pData, min(rLen, len) * sizeof(T));
				pop((int)nLen);
				return min(rLen,len);
			}
		}
		CellLog::Info("CellStream ReadArray error\n");
		return 0;
	}
	template<typename T>
	bool Write(T n) {
		auto nLen = sizeof(T);
		if (canWrite((int)nLen)) {
			//存入发送缓冲区
			push((char *)&n, (int)nLen);
			push((int)nLen);
			return true;
		}
		CellLog::Info("CellStream Write error\n");
		return false;
	}
	template<typename T>
	bool WriteArray(T* pData, uint32_t len) {
		auto nLen = sizeof(T)*len;
		if (canWrite((int)nLen + sizeof(uint32_t))) {
			//先写入数据长度
			WriteInt32(len);
			//存入发送缓冲区
			push((char *)pData, (int)nLen);
			push((int)nLen);
			return true;
		}
		CellLog::Info("CellStream WriteArray error\n");
		return false;
	}
	int8_t ReadInt8(int8_t n = 0) {
		Read(n);
		return n;
	}
	uint8_t ReadUint8(uint8_t n = 0) {
		Read(n);
		return n;
	}
	int16_t ReadInt16(int16_t n = 0) {
		Read(n);
		return n;
	}
	uint16_t ReadUint16(uint16_t n = 0) {
		Read(n);
		return n;
	}
	int32_t ReadInt32(int32_t n = 0) {
		Read(n);
		return n;
	}
	uint32_t ReadUint32(uint32_t n = 0) {
		Read(n);
		return n;
	}
	int64_t ReadInt64(int64_t n = 0) {
		Read(n);
		return n;
	}
	uint64_t ReadUint64(uint64_t n = 0) {
		Read(n);
		return n;
	}
	float ReadFloat(float f = 0.0) {
		Read(f);
		return f;
	}
	double ReadDouble(double f = 0.0) {
		Read(f);
		return f;
	}
	bool WriteInt8(int8_t n) {
		return Write(n);
	}
	bool WriteInt16(int16_t n) {
		return Write(n);
	}
	bool WriteInt32(int32_t n) {
		return Write(n);
	}
	bool WriteInt64(int64_t n) {
		return Write(n);
	}
	bool WriteFloat(float f) {
		return Write(f);
	}
	bool WriteDouble(double f) {
		return Write(f);
	}
private:
	//数据缓冲区
	char *_pBuff;
	int _nSize;
	//写入数据的尾部
	int _nWritePos;
	//读入数据的尾部
	int _nReadPos;
	//是否删除
	bool _bDelete;
};
