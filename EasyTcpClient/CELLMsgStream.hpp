#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_
#include<cstdint>
#include <memory>
#include"CELLStream.hpp"
#include"messageHeader.hpp"
//字节流byte
class CELLSendMsgStream:public CELLStream
{
public:
	CELLSendMsgStream(DataHeader* pHeader):CELLStream((char*)pHeader,pHeader->dataLength) {
		
		//预先占领消息长度所需空间
		writeInt32(0);
	}
	CELLSendMsgStream(int nSize = 1024):CELLStream(nSize) {
		//预先占领消息长度所需空间
		writeInt32(0);
	}

	CELLSendMsgStream(char* pData, int nSize, bool bDelete = false):CELLStream(pData,nSize,bDelete) {
		//预先占领消息长度所需空间
		writeInt32(0);
	}

	void finsh() {
		int nPos = getWritePos();
		setWritePos(0);//移动写指针到头部
		writeInt32(nPos);
		setWritePos(nPos);
	}

	void setMsgCmd(uint16_t cmd) {
		writeInt16(cmd);
	}

	uint16_t getMsgCmd() {
		uint16_t cmd = CMD_ERROR;
		cmd = readInt16(cmd);
		return cmd;
	}
};


//字节流byte
class CELLRecvMsgStream :public CELLStream
{
public:
	CELLRecvMsgStream(DataHeader* pHeader) :CELLStream((char*)pHeader, pHeader->dataLength) {
		push(pHeader->dataLength);
	}
};



#endif 