#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    1024 

struct SENDINFO {
	char name[256];
	UINT file_size;
};

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	while(1)
	{
		//���������ϴ� ���� ���� ����
		FILE *fp;
		SENDINFO dataInfo;

		printf("������ ���� �̸� : ");
		scanf("%s", dataInfo.name);

		fp = fopen(dataInfo.name, "rb");
		if (fp == NULL) {
			printf("������ ���� ���߽��ϴ�.");
			exit(1);
		}
		else {
			fseek(fp, 0, SEEK_END);
			dataInfo.file_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		}

		// ������ ������(���� ����), �������� ����
		retval = send(sock, (char *)&dataInfo, sizeof(SENDINFO), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}

		//������ ��ſ� ��� �� ����
		char buf[BUFSIZE];
		int cnt = dataInfo.file_size / BUFSIZE;
		int maxcnt = cnt;
		int len;

		while (cnt) {
			// ��������
			len = BUFSIZE;
			fread(buf, 1, len, fp);

			retval = send(sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display((char*)"send()");
				break;
			}
			system("cls");
			printf("%f ", (float)(maxcnt - cnt) * 100 / maxcnt);

			cnt--;
		}

		len = dataInfo.file_size - ((dataInfo.file_size / BUFSIZE)*BUFSIZE);
		fread(buf, 1, len, fp);
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR)err_display((char*)"send()");

		retval = send(sock, buf, len, 0);
		system("cls");
		if (retval == SOCKET_ERROR) err_display((char*)"send()");
		printf("���� ���� �̸� : %s(%d����Ʈ)\n", dataInfo.name, dataInfo.file_size);
		printf("������ �Ϸ�Ǿ����ϴ�.\n");
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();

	//delete testdata;

	return 0;
}