#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

// using namespace 사용하지 말것.

constexpr short SERVER_PORT = 3000;

void print_error_message(int s_err) {
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, s_err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::wcout << L" 에러 " << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

int main() {
	std::wcout.imbue(std::locale("korean"));

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);;
	if (s_socket <= 0) std::cout << "ERROR" << "원인";
	else std::cout << "Socket Created.\n";
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(s_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(SOCKADDR_IN));
	listen(s_socket, SOMAXCONN);
	INT addr_size = sizeof(SOCKADDR_IN);
	SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);

	while (true) {
		char recv_buffer[1024];
		WSABUF recv_wsabuf[1];
		recv_wsabuf[0].len = sizeof(recv_buffer);
		recv_wsabuf[0].buf = recv_buffer;
		DWORD recv_bytes;
		DWORD recv_flag = 0;
		auto ret = WSARecv(c_socket, recv_wsabuf, 1, &recv_bytes, &recv_flag, NULL, NULL);

		if (SOCKET_ERROR == ret) {
			std::cout << "Error at WSARecv : Error Code = ";
			auto s_err = WSAGetLastError();
			std::cout << s_err << std::endl;
			print_error_message(s_err);
			exit(-1);
		}

		recv_buffer[recv_bytes] = 0;
		std::cout << "From Client : " << recv_buffer << std::endl;

		char buffer[1024];
		WSABUF send_wsabuf[1];
		send_wsabuf[0].buf = recv_buffer;
		send_wsabuf[0].len = recv_bytes;
		DWORD send_bytes;
		WSASend(c_socket, send_wsabuf, 1, &send_bytes, 0, NULL, NULL);
	}
	closesocket(c_socket);
	closesocket(s_socket);
	WSACleanup();
}