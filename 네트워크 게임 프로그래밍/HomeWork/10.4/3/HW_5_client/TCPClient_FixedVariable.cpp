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

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

	// 윈속 초기화
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
		//보내고자하는 파일 정보 저장
		FILE *fp;
		SENDINFO dataInfo;

		printf("전송할 파일 이름 : ");
		scanf("%s", dataInfo.name);

		fp = fopen(dataInfo.name, "rb");
		if (fp == NULL) {
			printf("파일을 열지 못했습니다.");
			exit(1);
		}
		else {
			fseek(fp, 0, SEEK_END);
			dataInfo.file_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		}

		// 데이터 보내기(고정 길이), 데이터의 정보
		retval = send(sock, (char *)&dataInfo, sizeof(SENDINFO), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}

		//데이터 통신에 사용 될 변수
		char buf[BUFSIZE];
		int cnt = dataInfo.file_size / BUFSIZE;
		int maxcnt = cnt;
		int len;

		while (cnt) {
			// 가변길이
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
		printf("전송 파일 이름 : %s(%d바이트)\n", dataInfo.name, dataInfo.file_size);
		printf("전송이 완료되었습니다.\n");
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();

	//delete testdata;

	return 0;
}