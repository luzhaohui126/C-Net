#pragma once

#include<cstdint>
#include<string.h>
#include"CellStream.hpp"
//消息流
class CellRecvMsgStream : public CellStream
{
public:
	CellRecvMsgStream(msgHead *header) : CellStream((char *)header, header->dataLen) {
		ReadUint16();
		getMsgCmd();
	}
	uint16_t getMsgCmd() {
		uint16_t cmd = CMD_ERR;
		return ReadUint16(cmd);
	}
private:
};

class CellSendMsgStream : public CellStream
{
public:
	CellSendMsgStream(msgHead *header) : CellStream((char *)header, header->dataLen) {
		Write<uint16_t>(0);
	}
	CellSendMsgStream(int nSize = 1024) : CellStream(nSize) {

		Write<uint16_t>(0);
	}
	void setMsgCmd(uint16_t cmd) {
		Write<uint16_t>(cmd);
	}
	bool WriteString(const char* str, int len) {
		return WriteArray(str, len);
	}
	bool WriteString(const char* str) {
		return WriteString(str, strlen(str));
	}
	bool WriteString(std::string& string) {
		return WriteArray(string.c_str(), string.length());
	}
	void finish(){
		int pos = length();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
private:
};

