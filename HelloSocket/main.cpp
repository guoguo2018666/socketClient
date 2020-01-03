

#ifdef _WIN32
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

#include <iostream>
#include <vector>



//结构化数据-方式1
struct DataPackage
{
	int age;
	char name[32];
};

enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//消息头
struct DataHeader
{
	int32_t  dataLength;//数据长度
	short  cmd;
};
//命令
struct Login:public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char  userName[32];
	char  passWord[32];
};

struct LoginResult:public DataHeader
{
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int  result;
};

struct LoginOut:public DataHeader
{
	LoginOut() {
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}
	char  userName[32];	
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sockID = 0;
	}
	int  sockID;
};

struct LoginOutResult:public DataHeader
{
	LoginOutResult() {
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int  result;
};

int processor(SOCKET cSock) {

	DataHeader header = {};
	int nLen = recv(cSock, (char*)&header, sizeof(header), 0);
	if (nLen <= 0) {
		std::cout << "客户端已退出，任务结束" << std::endl;
		return -1;
	}

	switch (header.cmd)
	{
	case CMD_LOGIN:
	{
		Login login = {};
		recv(cSock, (char*)&login + sizeof(DataHeader), sizeof(login) - sizeof(DataHeader), 0);
		//忽略判断用户名和密码是否正确
		std::cout << "数据长度：" << login.dataLength << " ,命令：" << "login" << std::endl;

		//DataHeader retheader = {};
		//send(_cSock, (char*)&header, sizeof(header), 0);
		LoginResult ret = {};
		send(cSock, (char*)&ret, sizeof(ret), 0);
		break;

	}
	case CMD_LOGINOUT:
	{
		Login loginout = {};
		recv(cSock, (char*)&loginout + sizeof(DataHeader), sizeof(loginout) - sizeof(DataHeader), 0);
		//忽略判断用户名和密码是否正确
		std::cout << "数据长度：" << loginout.dataLength << " ,命令：" << "logout" << std::endl;
		//DataHeader retheader = {};
		//send(_cSock, (char*)&header, sizeof(header), 0);
		LoginResult ret = {};
		send(cSock, (char*)&ret, sizeof(ret), 0);
		break;
	}
	default:
		header.cmd = CMD_ERROR;
		header.dataLength = 0;
		send(cSock, (char*)&header, sizeof(header), 0);
		break;
	}

	
	return 0;

}

std::vector<SOCKET> g_clients;
int main() {
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver,&dat);
#endif

	//1建立1个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		std::cout << "ERROR,创建套接字失败" << std::endl;
	}
	else {
		std::cout << "创建套接字成功" << std::endl;
	}

	//2绑定
	sockaddr_in _sin;
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(14567);
	//_sin.sin_addr.S_un.S_addr = inet_addr(INADDR_ANY);
	//_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	_sin.sin_addr.s_addr = INADDR_ANY;
#endif 

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		std::cout << "ERROR,绑定端口失败" << std::endl;
	}
	else {
		std::cout << "绑定端口成功" << std::endl;
	}

	//3监听端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		std::cout << "ERROR,监听端口失败" << std::endl;
	}
	else {
		std::cout << "监听端口成功" << std::endl;
	}


	while (true)
	{
		fd_set fdReads;
		fd_set fdWrites;
		fd_set fdExecpts;
		FD_ZERO(&fdReads);
		FD_ZERO(&fdWrites);
		FD_ZERO(&fdExecpts);
		FD_SET(_sock, &fdReads);
		FD_SET(_sock, &fdWrites);
		FD_SET(_sock, &fdExecpts);
		SOCKET maxSock = _sock;
		for (int n = 0; n < (int)g_clients.size(); n++) {
			FD_SET(g_clients[n], &fdReads);
			if (maxSock < g_clients[n]) {
				maxSock = g_clients[n];
			}
		}
		timeval t = { 0,0 };
		int ret = select(maxSock + 1, &fdReads, &fdWrites, &fdExecpts, &t);
		if (ret < 0) {
			std::cout << "select发送错误，任务结束" << std::endl;
			break;
		}
		if (FD_ISSET(_sock, &fdReads)) {
			std::cout << "客户端有信息发过来" << std::endl;

			FD_CLR(_sock, &fdReads);

			//4 accept 等待接收客户端连接
			sockaddr_in _sinClient = {};
			int nAddrlen = sizeof(_sinClient);
			SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&_sinClient, &nAddrlen);
#else
			_cSock = accept(_sock, (sockaddr*)&_sinClient, (socklen_t*)&nAddrlen);
#endif
			if (INVALID_SOCKET == _cSock) {
				std::cout << "ERROR,accept失败" << std::endl;
			}
			else {
				std::cout << "accept成功" << std::endl;
			}

			std::cout << "新客户端加入：IP = " << inet_ntoa(_sinClient.sin_addr) << std::endl;
			for (int n = 0; n < (int)g_clients.size(); n++) {
				NewUserJoin userJoin;
			
				send(g_clients[n], (const char *)&userJoin, sizeof(userJoin), 0);
			}
			g_clients.push_back(_cSock);
			//FD_SET(_cSock, &fdReads);
			//FD_SET(_sock, &fdReads);
		}

#ifdef _WIN32
		for (int n = 0; n <= (int)fdReads.fd_count - 1; n++) {
			int ret = processor(fdReads.fd_array[n]);
			if (-1 == ret) {
				auto iter = std::find(g_clients.begin(), g_clients.end(), fdReads.fd_array[n]);
				if (iter != g_clients.end()){
					g_clients.erase(iter);
				}
			}
		}
#else
		for (int n = 0; n < (int)g_clients.size(); n++) {
			//FD_SET(g_clients[n], &fdReads);
			if (FD_ISSET(g_clients[n], &fdReads)) {
				int ret = processor(fdReads.fd_array[n]);
				if (-1 == ret) {
					auto iter = g_clients.begin()+n;
					if (iter != g_clients.end()) {
						g_clients.erase(iter);
					}
			}
		}
#endif
	}


	for (size_t n = 0; n < g_clients.size(); n++) {
#ifdef _WIN32	
		closesocket(g_clients[n]);
#else
		close(g_clients[n]);
#endif
	}

	//closesocket(_sock);

	//WSACleanup();
#ifdef _WIN32	
	//4关闭套接字
	closesocket(_sock);
	WSACleanup();
#else
	close(_sock);
#endif
	return 0;

}