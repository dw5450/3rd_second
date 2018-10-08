#include "stdafx.h"

#define SERVERPORT 9000
#define SERVERADDR "127.0.0.1"
#define BUFSIZE 1024

struct FILEINFO
{
	char name[256];
	size_t size;
};

int main(int argc, char *argv[])
{
	if (argc != 4) //입력 받은 인자가 4가 아닐 경우.
	{
		cout << "다음 인자에 맞춰서 입력해주세요." << endl;
		cout << "[FILENAME_EXE] [SERVER_IP] [SERVER_PORT] [SEND_FILENAME]" << endl;;
		exit(0);
	}
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit((char*)"socket()");

	// 소켓의 통신 설정(Protocol, IPv4, PortNum) <- 서버 통신 정보
	{
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
		serveraddr.sin_port = htons(atoi(argv[2]));

		retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit((char*)"connect()");
	}

	// 데이터 통신에 사용할 변수
	BYTE buf[BUFSIZE + 1];
	ZeroMemory(buf, BUFSIZE + 1);
	FILE* fp = NULL;
	FILEINFO finfo;

	size_t Cnt_send;
	size_t Remain_send;

	strcpy_s(finfo.name, 256, argv[3]);
	// 파일 읽어오기 (바이너리 형태로.)
	fopen_s(&fp, finfo.name, "rb");
	if (!fp)
	{
		closesocket(sock);
		MessageBox(NULL, TEXT("파일이 존재하지 않습니다."), TEXT("FileError"), MB_ICONERROR);
		exit(0);
	}
	fseek(fp, 0L, SEEK_END);
	finfo.size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	Cnt_send = finfo.size / BUFSIZE;
	Remain_send = finfo.size % BUFSIZE;
	// 서버와의 데이터 통신

	// 파일 정보 전송
	retval = sendn(sock, (char*)&finfo, sizeof(finfo), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display((char*)"send()");
		exit(0);
	}

	while (Cnt_send)
	{
		fread(buf, BUFSIZE, 1, fp);
		retval = sendn(sock, (char*)buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display((char*)"send()");
			break;
		}
		else if (retval == 0)
			break;
		Cnt_send--;
		// 데이터 송신 진행도 출력
		printf_s("Sended progress : %.2f%%\n", ((float)(finfo.size - (Cnt_send * BUFSIZE + Remain_send)) / finfo.size) * 100);
	}

	// 남은 데이터 받아오기
	if (Remain_send)
	{
		fread(buf, Remain_send, 1, fp);
		retval = sendn(sock, (char*)buf, Remain_send, 0);
		if (retval == SOCKET_ERROR)
			err_display((char*)"send()");
		cout << "Sended progress : 100.00%" << endl;
	}

	fclose(fp);

	// 소켓 메모리 해제
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}