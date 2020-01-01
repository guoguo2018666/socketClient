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
#include <vector>

#include "messageHeader.hpp"
#include "EaayTcpClient.hpp"



bool g_bExit = false;
void cmdThread(EazyTcpClient *client) {
	while (true) {
		char cmdBuf[128] = {};
		std::cout << "���������";
		std::cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			std::cout << "�����˳�����˳�cmdThread�̣߳��˳�" << std::endl;
			g_bExit = true;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			//�������������������
			Login login;
			strcpy_s(login.userName, "li");
			strcpy_s(login.passWord, "liPassword");
			client->SendData(&login);

			//���ܷ��������ص�����
			//LoginResult loginRet = {};
			//recv(sock, (char *)&loginRet, sizeof(loginRet), 0);
			//std::cout << "���������ؽ����" << loginRet.result << std::endl;
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			//�������������������
			LoginOut loginout;
			strcpy_s(loginout.userName, "li");
			client->SendData(&loginout);

			//���ܷ��������ص�����
			//LoginOutResult loginRet = {};
			//recv(sock, (char *)&loginRet, sizeof(loginRet), 0);
			//client->RecvData();
			//std::cout << "���������ؽ����" << loginRet.result << std::endl;
		}
		else {
			std::cout << "��Ч�������������" << std::endl;
		}
	}
}
CellTimestemp timestemp;
std::atomic_int recvCount = 0;
std::atomic_int sendCount = 0;
void timePrintf() {
	auto a1 = timestemp.getSecond();
	if (a1 >= 1.0) {
		//std::cout << "[" << a1 << "]����socket[" << cSock << "]--recv�����ݸ���[" << _recvCount << "]" << std::endl;
		std::cout << "[" << a1 << "]����" << "--send�����ݸ���[" << sendCount << "]" << std::endl;
		sendCount = 0;
		timestemp.update();

	}
}

void revcThread(std::vector<EazyTcpClient*> &clientsVector,int nBegin, int nEnd) {

	while (!g_bExit) {

		for (int i = nBegin; i < nEnd; i++) {

			if (!clientsVector[i]->OnRun()) {
				//break;
			}

		}

	}

}

void sendThread(int id) {
	std::vector<EazyTcpClient*> clientsVector;
	for (int i = 0; i < 40; i++) {
		//EazyTcpClient client;

		clientsVector.push_back(new EazyTcpClient());	
	}

	
	for (int i = 0; i < 40; i++) {
		clientsVector[i]->InitSocket();
		int clientConnect = clientsVector[i]->ConnectServer("127.0.0.1", 14567);
		if (clientConnect == -1) {
			std::cout <<"�߳�["<<id<< "]----client[" << i << "]���ӷ�����ʧ��[" << std::endl;
		}
		else if (clientConnect == 0) {
			std::cout << "�߳�[" << id << "]----client[" << i << "]���ӷ������ɹ�[" << std::endl;
		}

	}

	//���ӽ��վQ��
	std::thread threadRecv(revcThread, clientsVector,0,40);
	threadRecv.detach();
	
	//Login login;
	//strcpy_s(login.userName, "li");
	//strcpy_s(login.passWord, "liPassword");
	HeartC2S heartc2s;
	while (!g_bExit) {
		
		for (int i = 0; i < 40; i++) {
		
			for (int j = 0; j < 10; j++) {
				sendCount++;
				clientsVector[i]->SendData(&heartc2s);
			}

			//if (!clientsVector[i]->OnRun()) {
				//break;
			//}
			
		}
	
	}

	for (int i = 0; i < 40; i++) {
		clientsVector[i]->CloseSocket();
		delete clientsVector[i];
	}

}

int main() {

	LoginResult ret = {};
	std::cout << "xxxxxxxxxxxxx���ȡ�" << sizeof(LoginResult) << "]" << std::endl;

	EazyTcpClient client;
	client.InitSocket();

	client.ConnectServer("127.0.0.1", 14567);

	int nThreadNum = 1;
	for (int i = 0; i < nThreadNum; i++) {
		std::thread tSend(sendThread, i+1);
		tSend.detach();
	}


	//������Ļ�����߳�
	std::thread t1(cmdThread, &client);
	t1.detach();
	while (!g_bExit)
	{
		timePrintf();
	}
	
	return 0;

}


