#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <ws2tcpip.h>
#include "resource.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"chessclient";
LPCTSTR lpszWindowName = L"Chess Client";

int pieceX = 0;
int pieceY = 0;

HBITMAP hChessPiece = NULL;

SOCKET sock;
bool connected = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

bool InitConnection(const char* ip);
void SendKeyToServer(int key);
bool ReceivePositionFromServer();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
    g_hInst = hInstance;
    WNDCLASSEX WndClass = { sizeof(WndClass), CS_HREDRAW | CS_VREDRAW, WndProc,
        0, 0, hInstance, LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_HAND), (HBRUSH)GetStockObject(WHITE_BRUSH),
        NULL, lpszClass, LoadIcon(NULL, IDI_QUESTION) };

    RegisterClassEx(&WndClass);

    HWND hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
        200, 100, 800, 800, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG Message;
    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (int)Message.wParam;
}

bool InitConnection(const char* ip)
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3000);
    InetPtonA(AF_INET, ip, &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    connected = true;
    return true;
}

void SendKeyToServer(int key)
{
    if (connected) {
        send(sock, (char*)&key, sizeof(int), 0);
    }
}

bool ReceivePositionFromServer()
{
    if (connected) {
        int coords[2];
        int received = recv(sock, (char*)coords, sizeof(coords), 0);
        if (received == sizeof(coords)) {
            pieceX = coords[0];
            pieceY = coords[1];
            return true;
        }
    }
    return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    static const int BOARD_SIZE = 8;
    static HBRUSH hBrushWhite = CreateSolidBrush(RGB(240, 217, 181));
    static HBRUSH hBrushBlack = CreateSolidBrush(RGB(181, 136, 99));
    PAINTSTRUCT ps;
    HDC hDC;
    RECT rect;

    switch (iMessage) {
    case WM_CREATE:
    {
        hChessPiece = (HBITMAP)LoadImage(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
        if (!hChessPiece)
            MessageBox(hWnd, L"비트맵 로딩 실패", L"Error", MB_OK | MB_ICONERROR);

        char ip[100] = "127.0.0.1";
        if (!InitConnection(ip)) {
            MessageBox(hWnd, L"서버 연결 실패", L"Error", MB_OK | MB_ICONERROR);
        }
        return 0;
    }

    case WM_KEYDOWN:
        if (connected) {
            SendKeyToServer((int)wParam);
            if (ReceivePositionFromServer()) {
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        return 0;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);

        int squareWidth = (rect.right - rect.left) / BOARD_SIZE;
        int squareHeight = (rect.bottom - rect.top) / BOARD_SIZE;

        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                RECT squareRect = {
                    col * squareWidth,
                    row * squareHeight,
                    (col + 1) * squareWidth,
                    (row + 1) * squareHeight
                };

                HBRUSH brush = ((row + col) % 2 == 0) ? hBrushWhite : hBrushBlack;
                FillRect(hDC, &squareRect, brush);

                if (row == pieceY && col == pieceX && hChessPiece) {
                    HDC memDC = CreateCompatibleDC(hDC);
                    SelectObject(memDC, hChessPiece);
                    BITMAP bmp;
                    GetObject(hChessPiece, sizeof(bmp), &bmp);
                    StretchBlt(hDC, squareRect.left, squareRect.top, squareWidth, squareHeight,
                        memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                    DeleteDC(memDC);
                }
            }
        }

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (connected) {
            closesocket(sock);
            WSACleanup();
        }
        DeleteObject(hBrushWhite);
        DeleteObject(hBrushBlack);
        if (hChessPiece) DeleteObject(hChessPiece);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}