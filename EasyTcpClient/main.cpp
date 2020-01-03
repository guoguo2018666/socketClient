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

#include "CELLBuffer.hpp"
#include "CELLNetWork.hpp"



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
	int nClientNum = 1;
	std::vector<EazyTcpClient*> clientsVector;
	for (int i = 0; i < nClientNum; i++) {
		//EazyTcpClient client;

		clientsVector.push_back(new EazyTcpClient());	
	}

	
	for (int i = 0; i < nClientNum; i++) {
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
	std::thread threadRecv(revcThread, clientsVector,0, nClientNum);
	threadRecv.detach();
	
	//Login login;
	//strcpy_s(login.userName, "li");
	//strcpy_s(login.passWord, "liPassword");
	HeartC2S heartc2s;
	while (!g_bExit) {
		
		for (int i = 0; i < nClientNum; i++) {
		
			for (int j = 0; j < 10; j++) {
				sendCount++;
				clientsVector[i]->SendData(&heartc2s);
			}

			//if (!clientsVector[i]->OnRun()) {
				//break;
			//}
			
		}
	
	}

	for (int i = 0; i < nClientNum; i++) {
		clientsVector[i]->CloseSocket();
		delete clientsVector[i];
	}

}
#include"CELLStream.hpp"
int main() {

	LoginResult ret = {};
	std::cout << "xxxxxxxxxxxxx���ȡ�" << sizeof(LoginResult) << "]" << std::endl;

	std::cout << "int32_t--" << sizeof(int32_t) << "----" << sizeof(uint32_t) << std::endl;
	CELLStream cellstream;
	cellstream.writeInt32(5);

	//int32_t n5 = 0;	
	//cellstream.readInt32(n5);
	//std::cout << "read5--" << n5 << std::endl;

	char* str = "heihei";
	cellstream.writeArray(str, strlen(str));

	char a[] = "abcdefadd";
	cellstream.writeArray(a, strlen(a));


	int b[] = { 1,2,3,4,5 };
	cellstream.writeArray(b, 5);

	//---------��
	int32_t n5 = 0; 
	n5 = cellstream.readInt32(n5);
	std::cout << "n5[" << n5 <<"]"<< std::endl;

	char* strRead = new char[6];
	auto nRet1 = cellstream.readArray(strRead, 6);
	std::cout << "strRead[" << strRead << "]" << std::endl;
	delete[]strRead;


	char aRet[9];
	auto nRet2 = cellstream.readArray(aRet, 9);
	std::cout << "aRet[" << aRet << "]" << std::endl;

	int bRet[5];
	auto nRet3 = cellstream.readArray(bRet, 5);
	for (int n = 0; n < 5; n++) {
		std::cout << "bRet[" << bRet[n] << "]" << std::endl;
	}
	


	/*CELLStream cellstream;
	char a[] = "abcdefadd";
	cellstream.writeArray(a, strlen(a));
	//char b[] = "123456789";
	//cellstream.writeArray(b, strlen(b));

	cellstream.writeInt32(5);


	char aRet[9];
	auto nRet2 = cellstream.readArray(aRet, 9);
	std::cout << "aRet[" << aRet << "]" << std::endl;

	//char bRet[9];
	//auto nRet3 = cellstream.readArray(bRet, 9);
	//std::cout << "bRet[" << bRet << "]" << std::endl;

	int32_t n5 = 0;
	n5 = cellstream.readInt32(n5);
	std::cout << "n5[" << n5 << "]" << std::endl;*/

	//EazyTcpClient client;
	//client.InitSocket();

	//client.ConnectServer("127.0.0.1", 14567);

	//int nThreadNum = 1;
	//for (int i = 0; i < nThreadNum; i++) {
	//	std::thread tSend(sendThread, i+1);
	//	tSend.detach();
	//}


	////������Ļ�����߳�
	//std::thread t1(cmdThread, &client);
	//t1.detach();
	//while (!g_bExit)
	//{
	//	timePrintf();
	//}
	//
	return 0;

}


