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
		std::cout << "请输入命令：";
		std::cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			std::cout << "接收退出命令，退出cmdThread线程，退出" << std::endl;
			g_bExit = true;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			//向服务器发送请求命令
			Login login;
			strcpy_s(login.userName, "li");
			strcpy_s(login.passWord, "liPassword");
			client->SendData(&login);

			//接受服务器返回的数据
			//LoginResult loginRet = {};
			//recv(sock, (char *)&loginRet, sizeof(loginRet), 0);
			//std::cout << "服务器返回结果：" << loginRet.result << std::endl;
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			//向服务器发送请求命令
			LoginOut loginout;
			strcpy_s(loginout.userName, "li");
			client->SendData(&loginout);

			//接受服务器返回的数据
			//LoginOutResult loginRet = {};
			//recv(sock, (char *)&loginRet, sizeof(loginRet), 0);
			//client->RecvData();
			//std::cout << "服务器返回结果：" << loginRet.result << std::endl;
		}
		else {
			std::cout << "无效命令，请重新输入" << std::endl;
		}
	}
}
CellTimestemp timestemp;
std::atomic_int recvCount = 0;
std::atomic_int sendCount = 0;
void timePrintf() {
	auto a1 = timestemp.getSecond();
	if (a1 >= 1.0) {
		//std::cout << "[" << a1 << "]秒中socket[" << cSock << "]--recv的数据个数[" << _recvCount << "]" << std::endl;
		std::cout << "[" << a1 << "]秒中" << "--send的数据个数[" << sendCount << "]" << std::endl;
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
			std::cout <<"线程["<<id<< "]----client[" << i << "]连接服务器失败[" << std::endl;
		}
		else if (clientConnect == 0) {
			std::cout << "线程[" << id << "]----client[" << i << "]连接服务器成功[" << std::endl;
		}

	}

	//啓動接收綫程
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
	std::cout << "xxxxxxxxxxxxx长度【" << sizeof(LoginResult) << "]" << std::endl;

	EazyTcpClient client;
	client.InitSocket();

	client.ConnectServer("127.0.0.1", 14567);

	int nThreadNum = 1;
	for (int i = 0; i < nThreadNum; i++) {
		std::thread tSend(sendThread, i+1);
		tSend.detach();
	}


	//启动屏幕输入线程
	std::thread t1(cmdThread, &client);
	t1.detach();
	while (!g_bExit)
	{
		timePrintf();
	}
	
	return 0;

}


