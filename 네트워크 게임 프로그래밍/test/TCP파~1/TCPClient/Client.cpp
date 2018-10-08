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
	if (argc != 4) //�Է� ���� ���ڰ� 4�� �ƴ� ���.
	{
		cout << "���� ���ڿ� ���缭 �Է����ּ���." << endl;
		cout << "[FILENAME_EXE] [SERVER_IP] [SERVER_PORT] [SEND_FILENAME]" << endl;;
		exit(0);
	}
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit((char*)"socket()");

	// ������ ��� ����(Protocol, IPv4, PortNum) <- ���� ��� ����
	{
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
		serveraddr.sin_port = htons(atoi(argv[2]));

		retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit((char*)"connect()");
	}

	// ������ ��ſ� ����� ����
	BYTE buf[BUFSIZE + 1];
	ZeroMemory(buf, BUFSIZE + 1);
	FILE* fp = NULL;
	FILEINFO finfo;

	size_t Cnt_send;
	size_t Remain_send;

	strcpy_s(finfo.name, 256, argv[3]);
	// ���� �о���� (���̳ʸ� ���·�.)
	fopen_s(&fp, finfo.name, "rb");
	if (!fp)
	{
		closesocket(sock);
		MessageBox(NULL, TEXT("������ �������� �ʽ��ϴ�."), TEXT("FileError"), MB_ICONERROR);
		exit(0);
	}
	fseek(fp, 0L, SEEK_END);
	finfo.size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	Cnt_send = finfo.size / BUFSIZE;
	Remain_send = finfo.size % BUFSIZE;
	// �������� ������ ���

	// ���� ���� ����
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
		// ������ �۽� ���൵ ���
		printf_s("Sended progress : %.2f%%\n", ((float)(finfo.size - (Cnt_send * BUFSIZE + Remain_send)) / finfo.size) * 100);
	}

	// ���� ������ �޾ƿ���
	if (Remain_send)
	{
		fread(buf, Remain_send, 1, fp);
		retval = sendn(sock, (char*)buf, Remain_send, 0);
		if (retval == SOCKET_ERROR)
			err_display((char*)"send()");
		cout << "Sended progress : 100.00%" << endl;
	}

	fclose(fp);

	// ���� �޸� ����
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}