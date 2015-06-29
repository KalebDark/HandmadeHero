// HandmadeHero.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Main.h"
#include <stdint.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

bool Running = false;

const int BytesPerPixel = 4;

BITMAPINFO bitmapInfo;
void* bitmapMemory;
int bitmapWidth;
int bitmapHeight;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void RenderGradient(int xOffset, int yOffset) {
	int pitch = bitmapWidth * BytesPerPixel;
	uint8_t* row = (uint8_t*)bitmapMemory;
	for (int y = 0; y < bitmapHeight; ++y) {
		uint32_t* pixel = (uint32_t*)row;
		for (int x = 0; x < bitmapWidth; ++x) {
			uint8_t blue = x + xOffset;
			uint8_t green = y + yOffset;
			*pixel++ = (green << 8 | blue);
		}
		row += pitch;
	}
}

void Win32ResizeDIBSection(int width, int height) {
	if (bitmapMemory) {
		VirtualFree(bitmapMemory, 0, MEM_RELEASE);
	}

	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	//hBitmap = CreateDIBSection(bitmapDeviceContext, &bitmapInfo, DIB_RGB_COLORS, &bitmapMemory, 0, 0);
	int bitmapMemorySize = (bitmapWidth * bitmapHeight) * BytesPerPixel;
	bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	ZeroMemory(bitmapMemory, bitmapMemorySize);
}

void Win32UpdateWindow(HDC hDC, int x, int y, int windowWidth, int windowHeight) {
	StretchDIBits(hDC, x, y, bitmapWidth, bitmapHeight, x, y, windowWidth, windowHeight, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HANDMADEHERO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	Running = true;

	// Main message loop:
	int xOffset = 0;
	int yOffset = 0;
	MSG msg;
	while (Running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				Running = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		RenderGradient(xOffset, yOffset);
		xOffset += 1;
		yOffset += 1;

		RedrawWindow(msg.hwnd, 0, 0, RDW_INVALIDATE);
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szWindowClass;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	HWND hWnd;

	// Store instance handle in our global variable
	hInst = hInstance;

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
		case WM_COMMAND:
			wmId = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			return DefWindowProc(hWnd, message, wParam, lParam);

		case WM_SIZE: {
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			Win32ResizeDIBSection(width, height);
			break;
		}

		case WM_PAINT: {
			hdc = BeginPaint(hWnd, &ps);

			int x = ps.rcPaint.left;
			int y = ps.rcPaint.top;
			int width = ps.rcPaint.right - ps.rcPaint.left;
			int height = ps.rcPaint.bottom - ps.rcPaint.top;

			Win32UpdateWindow(hdc, x, y, width, height);

			//PatBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, 0);

			EndPaint(hWnd, &ps);
			break;
		}

		case WM_DESTROY:
			//PostQuitMessage(0);
			Running = false;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

