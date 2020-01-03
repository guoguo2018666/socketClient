#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
#define FD_SETSIZE 10240
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include<arpa/inet.h>
#include<string.h>
#define SOCKET int
#define INVALID_SOCKET 0
#define SOCKET_ERROR -1
#endif // _WIN32


#include <thread>
#include <iostream>
#include "messageHeader.hpp"
#include "CELLTimestemp.hpp"
#include <atomic>

#include"ClientSocket.hpp"
 
#define RECV_BUFF_SIZE 10240

class EazyTcpClient
{
public:
	EazyTcpClient() {
		
	}
	virtual ~EazyTcpClient() {
		CloseSocket();
	}

	int InitSocket() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		if (_pClient) {
			delete _pClient;
			_pClient = nullptr;
		}
		
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock) {
			std::cout << "ERROR,创建套接字失败" << std::endl;
		}
		else {
			std::cout << "创建套接字成功" << std::endl;

			_pClient = new ClientSocket(_sock);
		}

		return 0;

	}

	//连接服务器
	int ConnectServer(char* ip, unsigned short port) {
		//2连接服务器
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif 

		int _ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == _ret) {
			std::cout << "ERROR,连接服务端失败" << std::endl;
			return -1;
		}
		else {
			std::cout << "连接服务端成功" << std::endl;
		}
		return 0;

	}

	//关闭socket
	void CloseSocket() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32	
			//4关闭套接字
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
		}

	}

	//发送数据
	int SendData(DataHeader *header) {
		//if (isRun()&& (header != nullptr)) {
		//std::cout << " 确实发送了" << std::endl;
			return send(_sock, (const char*)header, header->dataLength, 0);
		//}
		return 0;
	}


		//处理网络消息
	bool OnRun() {
		
		//if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			//timeval t = { 1,0 };	
			int ret = select(_sock + 1, &fdReads, 0, 0, 0);
	
			//std::cout << "select end" << std::endl;
			if (ret < 0) {
				std::cout << "select返回错误，结束" << std::endl;
				return false;
			}

			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData()) {
					std::cout << "processor返回错误，结束" << std::endl;
					return false;
				}
			}

			//std::cout << "OnRun return true" << std::endl;

			return true;
		//}
		//return false;

	}


	bool isRun() {
		if (_sock != INVALID_SOCKET) {
			return true;
		}
		return false;
	}


	int RecvDataNew() {

	}

	//接收缓冲区
	//char _szRecv[RECV_BUFF_SIZE] = {};
	//第二缓冲区--处理数据缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE ] = {};
	//第二缓冲区中未处理数据的长度
	int _unDoSize = 0;
	int RecvData() {

		//缓冲区
		//DataHeader header = {};
		//int nLen = recv(_sock, szRecv, sizeof(header), 0);
		//接收缓冲区缓冲区
		char *szRecv = _szMsgBuf + _unDoSize;
		//int nLen = recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		
		int nLen = recv(_sock, szRecv, RECV_BUFF_SIZE - _unDoSize, 0);
		//std::cout << "ret返回了,没卡住" << std::endl;
		if (nLen <= 0) {
			std::cout << "与服务器断开连接，任务结束" << std::endl;
			return -1;
		}
		//std::cout << "接受到的数据长度[" << nLen << "]" << std::endl;

		//将消息缓冲区的数据copy到消息第二缓冲区
		//memcpy(_szMsgBuf+ _unDoSize, _szRecv, nLen);
		_unDoSize += nLen;
		//std::cout << "szMsgBuf的未处理的剩余长度[" << _unDoSize <<"]"<< std::endl;

		while(_unDoSize >= sizeof(DataHeader)){
			
			DataHeader *header = (DataHeader *)_szMsgBuf;
			
			if (_unDoSize >= header->dataLength) {
				
				//缓冲区剩余未处理消息的长度
				int nSize = _unDoSize - header->dataLength;
				//recv(_sock, _szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				OnNetMessage(header);
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				_unDoSize = nSize;
			}
			else {
				//std::cout << "break" << std::endl;
				//剩余未消息长度不够一条完整消息
				break;
			}
		}
		

		return 0;

	}

	void OnNetMessage(DataHeader *header) {
		if (header == nullptr) {
			return;
		}
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *login = (LoginResult *)header;
			
			//忽略判断用户名和密码是否正确
			//std::cout << "收到服务器消息CMD_LOGIN_RESULT，数据长度：" << login->dataLength << " ,结果：" << login->result << std::endl;
			break;
		}
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult *login = (LoginOutResult *)header;
			//recv(cSock, (char*)&login + sizeof(DataHeader), sizeof(login) - sizeof(DataHeader), 0);
			//忽略判断用户名和密码是否正确
			std::cout << "收到服务器消息CMD_LOGINOUT_RESULT，数据长度：" << login->dataLength << " ,结果：" << login->result << std::endl;
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *login = (NewUserJoin *)header;
			//recv(cSock, (char*)&login + sizeof(DataHeader), sizeof(login) - sizeof(DataHeader), 0);
			//忽略判断用户名和密码是否正确
			std::cout << "收到服务器消息CMD_NEW_USER_JOIN，数据长度：" << login->dataLength << " ,结果：" << login->sockID << std::endl;
			break;
		}
		case CMD_HEART_S2C:
		{
			HeartS2C *heartS2C = (HeartS2C *)(header);
			

			std::cout << "收到服務器[]的心跳" << std::endl;

			//向客戶端回送消息

			//std::shared_ptr<HeartS2C>ret(new HeartS2C());
			//pClient->SendData((DataHeader*)(ret.get()));

			//begin服務器發送與接收分離代碼，現在先注釋掉
			//addSendTask(pClient, (DataHeader*)ret);
			//end服務器發送與接收分離代碼，現在先注釋掉

			break;
		}
		default:
			std::cout << "收到服务器未知消息" << std::endl;
			//header.cmd = CMD_ERROR;
			//header.dataLength = 0;
			//send(cSock, (char*)&header, sizeof(header), 0);
			break;
		}

	}

	

private:
	SOCKET _sock;

	ClientSocket* _pClient = nullptr;

 

};









#endif

 