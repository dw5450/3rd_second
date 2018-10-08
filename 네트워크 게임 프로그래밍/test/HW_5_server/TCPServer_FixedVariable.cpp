#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 131072 

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID IpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&IpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)IpMsgBuf, msg, MB_ICONERROR);
	LocalFree(IpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID IpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&IpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)IpMsgBuf);
	LocalFree(IpMsgBuf);
}

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

struct FILEINFO {
	char name[256];
	UINT size;
};

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit((char*)"socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit((char*)"bind()");

	//listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit((char*)"listen()");

	// ��������ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE];
	int len;

	while (1) {
		//accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display((char*)"accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		while (1) {
			FILE *fp = NULL;
			FILEINFO dataInfo;

			retval = recvn(client_sock, (char*)&dataInfo, sizeof(dataInfo), 0);
			if (retval == SOCKET_ERROR) {
				err_display((char*)"recv()");
				break;
			}

			printf("������ �����մϴ�.\n");

			fp = fopen(dataInfo.name, "wb");

			int cnt = dataInfo.size / BUFSIZE;
			int maxcnt = cnt;

			while (cnt) {
				// ��������
				retval = recvn(client_sock, buf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR) {
					err_display((char*)"recv()");
					break;
				}
				else if (retval == 0)
					break;

				fwrite(buf, 1, BUFSIZE, fp);
				system("cls");
				printf("%f ", (float)(maxcnt - cnt) * 100 / maxcnt);
				cnt--;
			}
			//��������
			retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display((char*)"recv()");
				break;
			}
			else if (retval == 0)
				break;
			// ��������
			retval = recvn(client_sock, buf, len, 0);
			if (retval == SOCKET_ERROR) {
				err_display((char*)"recv()");
				break;
			}
			else if (retval == 0)
				break;

			system("cls");
			fwrite(buf, 1, len, fp);
			printf("���� ���� �Ϸ�.\n");
			printf("���Ź��� ���� �̸� : %s(%d����Ʈ)\n", dataInfo.name, dataInfo.size);

			fclose(fp);
			//closesocket(client_sock);
			printf("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ�=%s, ��Ʈ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		}
	}
	closesocket(client_sock);
	closesocket(listen_sock);

	WSACleanup();
	return 0;
}