#pragma once

#include<cstdint>
#include<string.h>
#include"CellStream.hpp"
//消息流
class CellReadStream : public CellStream
{
public:
	CellReadStream(msgHead *header) : CellReadStream((char *)header, header->dataLen) {
	}
	CellReadStream(char *data ,int size, bool bDelete = false) : CellStream(data,size, bDelete) {
		push(size);
	}
	uint16_t getMsgCmd() {
		uint16_t cmd = CMD_ERR;
		return ReadUint16(cmd);
	}
private:
};

class CellWriteStream : public CellStream
{
public:
	//CellWriteStream(msgHead *header) : CellStream((char *)header, header->dataLen) {
	//	Write<uint16_t>(0);
	//}
	CellWriteStream(int nSize = 1024) : CellStream(nSize) {

		Write<uint16_t>(0);
	}
	void setMsgCmd(uint16_t cmd) {
		Write<uint16_t>(cmd);
	}
	bool WriteString(const char* str, int len) {
		return WriteArray(str, len);
	}
	bool WriteString(const char* str) {
		return WriteString(str, (int)strlen(str));
	}
	bool WriteString(std::string& string) {
		return WriteArray(string.c_str(),(int)string.length());
	}
	void finish(){
		int pos = (int)length();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
private:
};

