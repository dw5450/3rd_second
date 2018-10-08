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
	if (argc != 2) //�Է� ���� ���ڰ� 4�� �ƴ� ���.
	{
		cout << "���� ���ڿ� ���缭 �Է����ּ���." << endl;
		cout << "[FILENAME] [PORTNUM]" << endl;;
		exit(0);
	}

	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	// ��� ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit((char*)"socket()");

	// ��� ������ ��� ����(Protocol, IPv4, PortNum)
	{
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		/*
		INADDR_ANY :
		������ ip�ּҸ� ������ ���� ��� Ȥ�� �ֱ������� ip�ּҰ� �ٲ� ��츦 ����ؼ�
		������ �ش��ϴ� ��� ip�ּҷ� ������ �����ϵ��� ����ϱ� ����.
		*/
		serveraddr.sin_port = htons(atoi(argv[1]));

		retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit((char*)"bind()");
	}

	// Ŭ���̾�Ʈ�κ��� ��û ���(��� ������ ����.)
	retval = listen(listen_sock, SOMAXCONN);
	/*
	backlog : ���� ���� �� ť�� ��Ƴ��� �� �ִ� �ִ� ���ġ
	�������̸� SOMAXCONN���� �Ѵ�.
	Why?
	������ �ϵ��� �ٲ� ��츦 ����ؼ�.
	*/
	if (retval == SOCKET_ERROR) err_quit((char*)"listen()");

	cout << "[TCP ����] ���� ��� �غ� �Ϸ�\nŬ���̾�Ʈ ��û �����...";

	// ������ ��ſ� ����� ����
	SOCKET client_sock = 0;
	SOCKADDR_IN clientaddr;
	int addrlen;
	BYTE buf[BUFSIZE];
	ZeroMemory(buf, BUFSIZE);

	while (true)
	{
		// ������ ������ Ŭ���̾�Ʈ Ȯ�� (��� ������ ���� Ȯ��.)
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		/*
		Ŭ���̾�Ʈ�κ��� ���� ��û�� ������
		��Ʒ� ��� �����
		(accept()�Լ� ���ο��� ���� �ݺ��ؼ� Ŭ���̾�Ʈ�� ������ ����Ѵ�.)
		*/
		if (client_sock == INVALID_SOCKET)
		{
			err_display((char*)"send()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		cout << endl << "[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=" << inet_ntoa(clientaddr.sin_addr)
			<< ", ��Ʈ��ȣ=" << ntohs(clientaddr.sin_port) << endl;
		/*
		���� ���� �������� �α����Ϸ� �����ؼ� ������ �ϴ� ���� ����.
		*/

		// Ŭ���̾�Ʈ���� ������ ���
		FILE* fp = NULL;
		FILEINFO finfo;
		size_t Cnt_recv;
		size_t Remain_recv;
		/*
		������ �Ѳ����� ��°�� �޾ƿ��� ���� �ƴ϶�
		���� ũ�� ��ŭ ������
		������ �޾ƿ´�.
		���� ũ�� / ���� ũ��

		���⼭ �Ѱ��� �����ؾ� �� ����
		���� ũ�⸦ ���� ũ��� �������� ���� ������ ���� �������� �ȵȴٴ� ���̴�.
		���� ���� ũ�� = �޾ƿ� Ƚ�� * ���� ũ�� + ������ ���� ũ��
		�޾ƿ� Ƚ�� = ���� ũ�� / ���� ũ��
		������ ���� ũ�� = ���� ũ�� % ���� ũ��
		*/
		while (true)
		{
			// Ŭ���̾�Ʈ�κ��� ������ �ޱ� (�������� : ������ Descriptor�� �ַ� �޾ƿ´�.)
			retval = recvn(client_sock, (char*)&finfo, sizeof(finfo), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char*)"recv()");
				break;
			}
			else if (retval == 0) // ���� �����Ͱ� ���� ��
				break;

			fopen_s(&fp, finfo.name, "wb");
			// ���� ������ ���� ���
			cout << "[TCP" << inet_ntoa(clientaddr.sin_addr) << ":" << ntohs(clientaddr.sin_port) << "]�� ���� ���� ������ ���۹޽��ϴ�." << endl;
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
				else if (retval == 0) // ���� �����Ͱ� ���� ��
					break;
				fwrite(buf, retval, 1, fp);
				Cnt_recv--;
				// ������ ���� ���൵ ���
				printf_s("Received progress : %.3f%%\n", ((float)(finfo.size - (Cnt_recv * BUFSIZE + Remain_recv)) / finfo.size) * 100);
			}

			// ���� ������ �޾ƿ���
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

		// ���� �޸� ����(Ŭ���̾�Ʈ ���Ϻ���.)
		closesocket(client_sock);
		cout << "[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=" << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ��ȣ=" << ntohs(clientaddr.sin_port) << endl;
	}

	// ���� �޸� ����(������ ������)
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}