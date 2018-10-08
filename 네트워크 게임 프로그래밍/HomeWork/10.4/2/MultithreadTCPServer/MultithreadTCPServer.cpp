#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define SERVERPORT 9000
#define BUFSIZE    1024

int g_down_num = 0;
std::vector<float> g_downInfo_arr;

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

DWORD WINAPI ShowDownInfo(LPVOID arg)
{
	while (1) {
		COORD Pos;
		Pos.X = 0;
		Pos.Y = 0;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);

		CONSOLE_CURSOR_INFO cci;
		cci.bVisible = false;
		cci.dwSize = 1;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);

		int cnt = 0;
		for (float f : g_downInfo_arr) {
			cnt++;
			printf("%d번 파일 다운로드 : %f%% \n", cnt, f);
		}

	}
	return 0;
}


// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE];
	int len;

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);		// getpeename socker desc를 으용하여 socetaddr 정보를 가져옴

	while (1) {
		FILE *fp = NULL;
		FILEINFO dataInfo;

		//고정 길이로 데이터의 정보를 가져옴
		retval = recvn(client_sock, (char*)&dataInfo, sizeof(dataInfo), 0);
		if (retval == SOCKET_ERROR) {
			err_display((char*)"recv()");
			break;
		}

		g_downInfo_arr.push_back(0.f);

		fp = fopen(dataInfo.name, "wb");
		int down_num = g_down_num++;

		int cnt = dataInfo.size / BUFSIZE;
		int maxcnt = cnt;

		while (cnt) {
			// 가변길이
			retval = recvn(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display((char*)"recv()");
				break;
			}
			else if (retval == 0)
				break;

			fwrite(buf, 1, BUFSIZE, fp);
			g_downInfo_arr[down_num] = (float)(maxcnt - cnt) * 100.0f / (float)maxcnt;
			cnt--;
		}
		//고정길이
		retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display((char*)"recv()");
			break;
		}
		else if (retval == 0)
			break;
		// 가변길이
		retval = recvn(client_sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display((char*)"recv()");
			break;
		}
		else if (retval == 0)
			break;

		//system("cls");
		fwrite(buf, 1, len, fp);
		//printf("파일 수신 완료.\n");
		//printf("수신받은 파일 이름 : %s(%d바이트)\n", dataInfo.name, dataInfo.size);

		fclose(fp);
		//closesocket(client_sock);
		g_downInfo_arr[down_num] = 100.0f;
		//printf("%d", g_down_num);
	}
	//closesocket(client_sock);
	//printf("[TCP 서버] 클라이언트 종료 : IP 주소=%s, 포트번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	closesocket(client_sock);
	return 0;
}

int main(int argc, char *argv[])
{
	int retval;

	std::vector<FILEINFO> vfi;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;

	hThread = CreateThread(NULL, 0, ShowDownInfo,NULL, 0, NULL);

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		//printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			//inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		if(hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}