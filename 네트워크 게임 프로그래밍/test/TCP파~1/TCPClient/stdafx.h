// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 특정 포함 파일이 들어 있는
// 포함 파일입니다.
//

#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <list>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>

using namespace std;

// 윈속 함수 에러 출력
void err_quit(char* msg);
void err_display(char* msg);

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char* buf, int len, int flags);

// 사용자 정의 데이터 송신 함수
int sendn(SOCKET s, char* buf, int len, int flags);