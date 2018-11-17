#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"

#define SERVERIP		"127.0.0.1"
#define SERVERPORT		9000
#define FILENAMESIZE	512
#define BUFSIZE			1024

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);
//진행 상황을 알려주는 프로그래스바
//INT_PTR CALLBACK ProgDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

SOCKET sock; // 소켓
char FileNameBuf[FILENAMESIZE + 1]; // 파일 이름 송수신 버퍼
char CommunicationBuf[BUFSIZE];
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hSendButton; // 보내기 버튼
HWND hEdit1, hEdit2; // 편집 컨트롤

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;

	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hSendButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit1, EM_SETLIMITTEXT, FILENAMESIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
											  /*HWND hProgDlg;
											  hProgDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PROGRESS_WND), hDlg, ProgDlgProc);
											  if (hProgDlg != NULL)
											  ShowWindow(hProgDlg, SW_SHOW);*/
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
													   //GetDlgItemText(hDlg, IDC_EDIT1, FileNameBuf, FILENAMESIZE +1);
			SetEvent(hWriteEvent); // 쓰기 완료 알리기
			SetFocus(hEdit1);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[FILENAMESIZE + 256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
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

// TCP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");


	FILE *fp;														//파일에 정보를 담을 포인터
	int FileSize = -1;												//받은 파일 크기

																	// 서버와 데이터 통신
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기

													// 문자열 길이가 0이면 보내지 않음
		if (strlen(FileNameBuf) == 0) {
			EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
			SetEvent(hReadEvent); // 읽기 완료 알리기
			continue;
		}

		// 받고자 하는 파일의 이름을 보냄
		retval = send(sock, FileNameBuf, FILENAMESIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		DisplayText("[TCP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

		//데이터의 크기를 받음
		retval = recvn(sock, (char*)&FileSize, sizeof(FileSize), 0);
		DisplayText("[TCP 클라이언트] 파일 크기는 %d 입니다..\r\n", FileSize);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		//파일의 크기가 0보다 클 경우 데이터를 전송 받음
		//데이터크기를 넘지 않고 반복하면서 버프사이즈 만큼 데이터를 받음
		if (FileSize > 0)
		{
			fp = fopen(FileNameBuf, "wb");
			int cnt = 0;
			int maxcnt = FileSize / BUFSIZE;

			while (cnt < maxcnt)
			{
				// 가변길이
				retval = recvn(sock, CommunicationBuf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR) {
					err_display((char*)"recv()");
					break;
				}
				else if (retval == 0)
					break;

				fwrite(CommunicationBuf, 1, BUFSIZE, fp);
				cnt++;
				//DisplayText("[TCP 클라이언트] 다운로드는 %f 입니다..\r\n", (float)cnt/maxcnt);
			}

			int RemainingBufSize;
			//마지막에 남은 데이터를 받음
			RemainingBufSize = FileSize - ((FileSize / BUFSIZE)*BUFSIZE);
			retval = recvn(sock, CommunicationBuf, RemainingBufSize, 0);
			if (retval == SOCKET_ERROR) {
				err_display((char*)"recv()");
				break;
			}
			else if (retval == 0)
				break;

			fwrite(CommunicationBuf, 1, RemainingBufSize, fp);

			fclose(fp);
		}


		EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
		SetEvent(hReadEvent); // 읽기 완료 알리기
	}

	return 0;
}

//INT_PTR CALLBACK ProgDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	//static std::list<std::pair<HWND, DWORD>> ThreadIDs; //hDlg, ThreadID
//	UNREFERENCED_PARAMETER(lParam);
//	switch (message)
//	{
//	case WM_INITDIALOG:
//	{
//		/*DWORD ThreadID;
//		CreateThread(NULL, 0, ThreadProgDlg, (LPVOID)hDlg, 0, &ThreadID);
//		ThreadIDs.emplace_back(std::make_pair(hDlg, ThreadID));*/
//		break;
//	}
//	case WM_CLOSE:
//	{
//		/*for (auto i = ThreadIDs.begin(); i != ThreadIDs.end(); ++i)
//			if ((*i).first == hDlg)
//			{
//				PostThreadMessage((*i).second, WM_CLOSE, 0, 0);
//				ThreadIDs.erase(i);
//				break;
//			}*/
//		EndDialog(hDlg, LOWORD(wParam));
//		return (INT_PTR)TRUE;
//	}
//	}
//	return (INT_PTR)FALSE;
//}
