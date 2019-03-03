#pragma once

#include<cstdint>
#include<string.h>
#include"CellStream.hpp"
//消息流
<<<<<<< HEAD
class CellReadStream : public CellStream
{
public:
	CellReadStream(msgHead *header) : CellReadStream((char *)header, header->dataLen) {
	}
	CellReadStream(char *data ,int size, bool bDelete = false) : CellStream(data,size, bDelete) {
		push(size);
=======
class CellRecvMsgStream : public CellStream
{
public:
	CellRecvMsgStream(msgHead *header) : CellStream((char *)header, header->dataLen) {
		ReadUint16();
		getMsgCmd();
>>>>>>> 13f1061a9afa4d04566659da0bbd773a4115c4b8
	}
	uint16_t getMsgCmd() {
		uint16_t cmd = CMD_ERR;
		return ReadUint16(cmd);
	}
private:
};

<<<<<<< HEAD
class CellWriteStream : public CellStream
{
public:
	//CellWriteStream(msgHead *header) : CellStream((char *)header, header->dataLen) {
	//	Write<uint16_t>(0);
	//}
	CellWriteStream(int nSize = 1024) : CellStream(nSize) {
=======
class CellSendMsgStream : public CellStream
{
public:
	CellSendMsgStream(msgHead *header) : CellStream((char *)header, header->dataLen) {
		Write<uint16_t>(0);
	}
	CellSendMsgStream(int nSize = 1024) : CellStream(nSize) {
>>>>>>> 13f1061a9afa4d04566659da0bbd773a4115c4b8

		Write<uint16_t>(0);
	}
	void setMsgCmd(uint16_t cmd) {
		Write<uint16_t>(cmd);
	}
	bool WriteString(const char* str, int len) {
		return WriteArray(str, len);
	}
	bool WriteString(const char* str) {
<<<<<<< HEAD
		return WriteString(str, (int)strlen(str));
	}
	bool WriteString(std::string& string) {
		return WriteArray(string.c_str(),(int)string.length());
	}
	void finish(){
		int pos = (int)length();
=======
		return WriteString(str, strlen(str));
	}
	bool WriteString(std::string& string) {
		return WriteArray(string.c_str(), string.length());
	}
	void finish(){
		int pos = length();
>>>>>>> 13f1061a9afa4d04566659da0bbd773a4115c4b8
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
private:
};

