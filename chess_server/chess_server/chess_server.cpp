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
        cout << "소켓 생성 실패" << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "바인드 실패" << endl;
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "리스닝 실패" << endl;
        return 1;
    }

    cout << "서버가 9000번 포트에서 대기 중입니다..." << endl;

    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
    if (clientSocket == INVALID_SOCKET) {
        cout << "클라이언트 연결 실패" << endl;
        return 1;
    }

    cout << "클라이언트 연결 성공!" << endl;

    while (true) {
        int keyCode = 0;
        int recvBytes = recv(clientSocket, (char*)&keyCode, sizeof(keyCode), 0);
        if (recvBytes <= 0) {
            cout << "클라이언트 연결 끊김" << endl;
            break;
        }

        cout << "받은 키: " << keyCode << endl;

        // 좌표 업데이트
        updatePiecePosition(keyCode);

        // 좌표 전송
        int coords[2] = { pieceX, pieceY };
        send(clientSocket, (char*)coords, sizeof(coords), 0);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}