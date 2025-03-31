#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 3000
#define MAX_CLIENTS 10

struct ClientInfo {
    SOCKET socket;
    WSAOVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[128];
    int pieceX;
    int pieceY;
    int clientID;
};

HANDLE completionPort;

void updatePiecePosition(ClientInfo* client, int keyCode) {
    const int BOARD_SIZE = 8;
    switch (keyCode) {
    case VK_UP: if (client->pieceY > 0) client->pieceY--; break;
    case VK_DOWN: if (client->pieceY < BOARD_SIZE - 1) client->pieceY++; break;
    case VK_LEFT: if (client->pieceX > 0) client->pieceX--; break;
    case VK_RIGHT: if (client->pieceX < BOARD_SIZE - 1) client->pieceX++; break;
    }
}

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    LPOVERLAPPED overlapped;
    DWORD flags = 0;

    while (true) {
        BOOL result = GetQueuedCompletionStatus(completionPort, &bytesTransferred, &completionKey, &overlapped, INFINITE);
        if (!result) continue;

        ClientInfo* client = (ClientInfo*)completionKey;

        if (bytesTransferred == 0) {
            cout << "클라이언트 연결 종료: ID " << client->clientID << endl;
            closesocket(client->socket);
            delete client;
            continue;
        }

        int keyCode = *(int*)client->buffer;
        cout << "클라이언트 " << client->clientID << "로부터 받은 키: " << keyCode << endl;

        updatePiecePosition(client, keyCode);

        int coords[2] = { client->pieceX, client->pieceY };
        send(client->socket, (char*)coords, sizeof(coords), 0);

        memset(client->buffer, 0, sizeof(client->buffer));
        client->wsaBuf.buf = client->buffer;
        client->wsaBuf.len = sizeof(client->buffer);
        WSARecv(client->socket, &client->wsaBuf, 1, NULL, &flags, &client->overlapped, NULL);
    }
    return 0;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (serverSocket == INVALID_SOCKET) {
        cout << "소켓 생성 실패" << endl;
        return 1;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "바인드 실패" << endl;
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "리스닝 실패" << endl;
        return 1;
    }

    completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);

    cout << "서버가 3000번 포트에서 대기 중입니다..." << endl;

    int clientCount = 0;

    while (true) {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);

        if (clientSocket == INVALID_SOCKET) continue;

        ClientInfo* client = new ClientInfo;
        client->socket = clientSocket;
        client->pieceX = 0;
        client->pieceY = 0;
        client->clientID = ++clientCount;

        // Overlapped 구조체를 ZeroMemory로 초기화
        ZeroMemory(&client->overlapped, sizeof(client->overlapped));

        // WSABUF 구조체 초기화
        ZeroMemory(client->buffer, sizeof(client->buffer));
        client->wsaBuf.buf = client->buffer;
        client->wsaBuf.len = sizeof(client->buffer);

        // I/O 완료 포트에 소켓을 연결
        CreateIoCompletionPort((HANDLE)clientSocket, completionPort, (ULONG_PTR)client, 0);

        // 첫 번째 비동기 수신 요청
        DWORD flags = 0;
        int recvResult = WSARecv(
            client->socket,
            &client->wsaBuf,
            1,
            NULL,
            &flags,
            &client->overlapped,
            NULL
        );

        if (recvResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            cout << "WSARecv 실패: " << WSAGetLastError() << endl;
            closesocket(client->socket);
            delete client;
            continue;
        }

        cout << "클라이언트 연결 성공: ID " << client->clientID << endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}