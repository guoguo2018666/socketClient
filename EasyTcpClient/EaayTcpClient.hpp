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
			std::cout << "ERROR,�����׽���ʧ��" << std::endl;
		}
		else {
			std::cout << "�����׽��ֳɹ�" << std::endl;

			_pClient = new ClientSocket(_sock);
		}

		return 0;

	}

	//���ӷ�����
	int ConnectServer(char* ip, unsigned short port) {
		//2���ӷ�����
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
			std::cout << "ERROR,���ӷ����ʧ��" << std::endl;
			return -1;
		}
		else {
			std::cout << "���ӷ���˳ɹ�" << std::endl;
		}
		return 0;

	}

	//�ر�socket
	void CloseSocket() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32	
			//4�ر��׽���
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
		}

	}

	//��������
	int SendData(DataHeader *header) {
		//if (isRun()&& (header != nullptr)) {
		//std::cout << " ȷʵ������" << std::endl;
			return send(_sock, (const char*)header, header->dataLength, 0);
		//}
		return 0;
	}


		//����������Ϣ
	bool OnRun() {
		
		//if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			//timeval t = { 1,0 };	
			int ret = select(_sock + 1, &fdReads, 0, 0, 0);
	
			//std::cout << "select end" << std::endl;
			if (ret < 0) {
				std::cout << "select���ش��󣬽���" << std::endl;
				return false;
			}

			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData()) {
					std::cout << "processor���ش��󣬽���" << std::endl;
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

	//���ջ�����
	//char _szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ�������--�������ݻ�����
	char _szMsgBuf[RECV_BUFF_SIZE ] = {};
	//�ڶ���������δ�������ݵĳ���
	int _unDoSize = 0;
	int RecvData() {

		//������
		//DataHeader header = {};
		//int nLen = recv(_sock, szRecv, sizeof(header), 0);
		//���ջ�����������
		char *szRecv = _szMsgBuf + _unDoSize;
		//int nLen = recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		
		int nLen = recv(_sock, szRecv, RECV_BUFF_SIZE - _unDoSize, 0);
		//std::cout << "ret������,û��ס" << std::endl;
		if (nLen <= 0) {
			std::cout << "��������Ͽ����ӣ��������" << std::endl;
			return -1;
		}
		//std::cout << "���ܵ������ݳ���[" << nLen << "]" << std::endl;

		//����Ϣ������������copy����Ϣ�ڶ�������
		//memcpy(_szMsgBuf+ _unDoSize, _szRecv, nLen);
		_unDoSize += nLen;
		//std::cout << "szMsgBuf��δ�����ʣ�೤��[" << _unDoSize <<"]"<< std::endl;

		while(_unDoSize >= sizeof(DataHeader)){
			
			DataHeader *header = (DataHeader *)_szMsgBuf;
			
			if (_unDoSize >= header->dataLength) {
				
				//������ʣ��δ������Ϣ�ĳ���
				int nSize = _unDoSize - header->dataLength;
				//recv(_sock, _szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				OnNetMessage(header);
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				_unDoSize = nSize;
			}
			else {
				//std::cout << "break" << std::endl;
				//ʣ��δ��Ϣ���Ȳ���һ��������Ϣ
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
			
			//�����ж��û����������Ƿ���ȷ
			//std::cout << "�յ���������ϢCMD_LOGIN_RESULT�����ݳ��ȣ�" << login->dataLength << " ,�����" << login->result << std::endl;
			break;
		}
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult *login = (LoginOutResult *)header;
			//recv(cSock, (char*)&login + sizeof(DataHeader), sizeof(login) - sizeof(DataHeader), 0);
			//�����ж��û����������Ƿ���ȷ
			std::cout << "�յ���������ϢCMD_LOGINOUT_RESULT�����ݳ��ȣ�" << login->dataLength << " ,�����" << login->result << std::endl;
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *login = (NewUserJoin *)header;
			//recv(cSock, (char*)&login + sizeof(DataHeader), sizeof(login) - sizeof(DataHeader), 0);
			//�����ж��û����������Ƿ���ȷ
			std::cout << "�յ���������ϢCMD_NEW_USER_JOIN�����ݳ��ȣ�" << login->dataLength << " ,�����" << login->sockID << std::endl;
			break;
		}
		case CMD_HEART_S2C:
		{
			HeartS2C *heartS2C = (HeartS2C *)(header);
			

			std::cout << "�յ�������[]������" << std::endl;

			//��͑��˻�����Ϣ

			//std::shared_ptr<HeartS2C>ret(new HeartS2C());
			//pClient->SendData((DataHeader*)(ret.get()));

			//begin�������l���c���շ��x���a���F����עጵ�
			//addSendTask(pClient, (DataHeader*)ret);
			//end�������l���c���շ��x���a���F����עጵ�

			break;
		}
		default:
			std::cout << "�յ�������δ֪��Ϣ" << std::endl;
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

 