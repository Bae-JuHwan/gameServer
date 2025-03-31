#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int pieceX = 0;
int pieceY = 0;
const int BOARD_SIZE = 8;

void updatePiecePosition(int keyCode) {
    switch (keyCode) {
    case VK_UP:
        if (pieceY > 0) pieceY--;
        break;
    case VK_DOWN:
        if (pieceY < BOARD_SIZE - 1) pieceY++;
        break;
    case VK_LEFT:
        if (pieceX > 0) pieceX--;
        break;
    case VK_RIGHT:
        if (pieceX < BOARD_SIZE - 1) pieceX++;
        break;
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int addrSize = sizeof(clientAddr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "���� ���� ����" << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "���ε� ����" << endl;
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "������ ����" << endl;
        return 1;
    }

    cout << "������ 9000�� ��Ʈ���� ��� ���Դϴ�..." << endl;

    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Ŭ���̾�Ʈ ���� ����" << endl;
        return 1;
    }

    cout << "Ŭ���̾�Ʈ ���� ����!" << endl;

    while (true) {
        int keyCode = 0;
        int recvBytes = recv(clientSocket, (char*)&keyCode, sizeof(keyCode), 0);
        if (recvBytes <= 0) {
            cout << "Ŭ���̾�Ʈ ���� ����" << endl;
            break;
        }

        cout << "���� Ű: " << keyCode << endl;

        // ��ǥ ������Ʈ
        updatePiecePosition(keyCode);

        // ��ǥ ����
        int coords[2] = { pieceX, pieceY };
        send(clientSocket, (char*)coords, sizeof(coords), 0);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}