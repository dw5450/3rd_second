#include "stdafx.h"

#define SERVERPORT 9000
#define BUFSIZE 1024

struct FILEINFO
{
	char name[256];
	size_t size;
};

int main(int argc, char *argv[])
{
	if (argc != 2) //입력 받은 인자가 4가 아닐 경우.
	{
		cout << "다음 인자에 맞춰서 입력해주세요." << endl;
		cout << "[FILENAME] [PORTNUM]" << endl;;
		exit(0);
	}

	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	// 대기 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit((char*)"socket()");

	// 대기 소켓의 통신 설정(Protocol, IPv4, PortNum)
	{
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		/*
		INADDR_ANY :
		서버가 ip주소를 여러개 갖을 경우 혹은 주기적으로 ip주소가 바뀔 경우를 대비해서
		서버에 해당하는 모든 ip주소로 접근이 가능하도록 허용하기 위함.
		*/
		serveraddr.sin_port = htons(atoi(argv[1]));

		retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit((char*)"bind()");
	}

	// 클라이언트로부터 요청 대기(대기 소켓을 통해.)
	retval = listen(listen_sock, SOMAXCONN);
	/*
	backlog : 동시 접속 시 큐에 담아넣을 수 있는 최대 허용치
	가급적이면 SOMAXCONN으로 한다.
	Why?
	서버의 하드웨어가 바뀔 경우를 대비해서.
	*/
	if (retval == SOCKET_ERROR) err_quit((char*)"listen()");

	cout << "[TCP 서버] 소켓 통신 준비 완료\n클라이언트 요청 대기중...";

	// 데이터 통신에 사용할 변수
	SOCKET client_sock = 0;
	SOCKADDR_IN clientaddr;
	int addrlen;
	BYTE buf[BUFSIZE];
	ZeroMemory(buf, BUFSIZE);

	while (true)
	{
		// 서버에 접속한 클라이언트 확인 (대기 소켓을 통해 확인.)
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		/*
		클라이언트로부터 받은 요청이 있으면
		↓아래 명령 실행↓
		(accept()함수 내부에서 무한 반복해서 클라이언트의 접속을 대기한다.)
		*/
		if (client_sock == INVALID_SOCKET)
		{
			err_display((char*)"send()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		cout << endl << "[TCP 서버] 클라이언트 접속: IP 주소=" << inet_ntoa(clientaddr.sin_addr)
			<< ", 포트번호=" << ntohs(clientaddr.sin_port) << endl;
		/*
		위와 같은 정보들은 로그파일로 덤핑해서 관리를 하는 것이 좋다.
		*/

		// 클라이언트와의 데이터 통신
		FILE* fp = NULL;
		FILEINFO finfo;
		size_t Cnt_recv;
		size_t Remain_recv;
		/*
		파일을 한꺼번에 통째로 받아오는 것이 아니라
		버퍼 크기 만큼 나눠서
		여러번 받아온다.
		파일 크기 / 버퍼 크기

		여기서 한가지 유의해야 할 점이
		파일 크기를 버퍼 크기로 나누었을 때의 나머지 값을 빼먹으면 안된다는 것이다.
		받은 파일 크기 = 받아온 횟수 * 버퍼 크기 + 나머지 버퍼 크기
		받아온 횟수 = 파일 크기 / 버퍼 크기
		나머지 버퍼 크기 = 파일 크기 % 버퍼 크기
		*/
		while (true)
		{
			// 클라이언트로부터 데이터 받기 (고정길이 : 데이터 Descriptor를 주로 받아온다.)
			retval = recvn(client_sock, (char*)&finfo, sizeof(finfo), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char*)"recv()");
				break;
			}
			else if (retval == 0) // 받은 데이터가 없을 때
				break;

			fopen_s(&fp, finfo.name, "wb");
			// 받은 데이터 정보 출력
			cout << "[TCP" << inet_ntoa(clientaddr.sin_addr) << ":" << ntohs(clientaddr.sin_port) << "]로 부터 다음 파일을 전송받습니다." << endl;
			cout << "FileName : " << finfo.name << ", FileSize : " << finfo.size << "(byte)" << endl;

			Cnt_recv = finfo.size / BUFSIZE;
			Remain_recv = finfo.size % BUFSIZE;

			while (Cnt_recv)
			{
				retval = recvn(client_sock, (char*)buf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display((char*)"recv()");
					break;
				}
				else if (retval == 0) // 받은 데이터가 없을 때
					break;
				fwrite(buf, retval, 1, fp);
				Cnt_recv--;
				// 데이터 수신 진행도 출력
				printf_s("Received progress : %.3f%%\n", ((float)(finfo.size - (Cnt_recv * BUFSIZE + Remain_recv)) / finfo.size) * 100);
			}

			// 남은 데이터 받아오기
			if (Remain_recv)
			{
				retval = recvn(client_sock, (char*)buf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR)
					err_display((char*)"recv()");
				fwrite(buf, retval, 1, fp);
				cout << "Received progress : 100.000%" << endl;
			}

			fclose(fp);
		}

		// 소켓 메모리 해제(클라이언트 소켓부터.)
		closesocket(client_sock);
		cout << "[TCP 서버] 클라이언트 종료: IP 주소=" << inet_ntoa(clientaddr.sin_addr) << ", 포트번호=" << ntohs(clientaddr.sin_port) << endl;
	}

	// 소켓 메모리 해제(서버의 대기소켓)
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}