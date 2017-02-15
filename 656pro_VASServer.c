// TCP socket.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include"commandRecognition.h"
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE 100

#define BUF_SIZE 100

void findIP(char *ip, int size)   //get the local ip address
{
	WORD v = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(v, &wsaData); // 加载套接字库  

	struct hostent *phostinfo = gethostbyname("");
	char *p = inet_ntoa(*((struct in_addr *)(*phostinfo->h_addr_list)));
	strncpy(ip, p, size - 1);
	ip[size - 1] = '\0';
	WSACleanup();
}

int main() 
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//创建套接字
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
	//绑定套接字
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
	sockAddr.sin_family = PF_INET;  //使用IPv4地址
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
	sockAddr.sin_port = htons(10101);  //端口
	printf("Server is now activated...\n");
	char ip[20] = { 0 };
	findIP(ip, sizeof(ip));
	printf("IP address of Server: %s\n" , ip);
	printf("Port number: 10101.\n");
	bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	//进入监听状态
	listen(servSock, 20);
	//接收客户端请求
	SOCKADDR clntAddr;
	int nSize = sizeof(SOCKADDR);
	char buffer[BUF_SIZE] = { 0 };  //缓冲区
	while (1) 
	{
		SOCKET clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);
		//printf("client: %s\n", clntAddr);
		int strLen = recv(clntSock, buffer, BUF_SIZE, 0);  //接收客户端发来的数据
		printf("Received client's message.\n");
		printf("%s", buffer);
		printf("\n");

		//recognize the key words of the message:
		seqstring t, pLight;
		int next[50];
		strcpy(t.string, buffer);
		t.length = strlen(t.string);
		strcpy(pLight.string, "light");
		pLight.length = strlen(pLight.string);

		getnext(pLight, next);
		int isMatch;
		isMatch = kmp(t, pLight, next);
		if (isMatch != -1)
		{
			send(clntSock, pLight.string, pLight.length, 0);  //将数据原样返回
		}
		else
			send(clntSock, "Don't match", strlen("Don't match"), 0);  //将数据原样返回

		//send(clntSock, buffer, strLen, 0);  //将数据原样返回
		closesocket(clntSock);  //关闭套接字
		memset(buffer, 0, BUF_SIZE);  //重置缓冲区
	}
	//关闭套接字
	closesocket(servSock);
	//终止 DLL 的使用
	WSACleanup();
	return 0;
}