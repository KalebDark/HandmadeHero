// HandmadeHero.cpp : Defines the entry point for the application.
//

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif

#ifndef WINVER
#define WINVER 0x601
#endif

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdint.h>
#include <Xinput.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/hh405051(v=vs.85).aspx
//#pragma comment(lib,XINPUT_DLL_W)
//#pragma comment(lib,"xinput1_4.lib")
// xinput9_1_0.lib is for Win7
// xinput 1.4 is for Win8 and above
#pragma comment(lib,"xinput9_1_0.lib")

#define MAX_LOADSTRING 100

// Global Variables:
bool Running = false;

struct Win32OffscreenBuffer {
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct Win32WindowSize {
	int width;
	int height;
};

Win32OffscreenBuffer buffer;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void RenderGradient(Win32OffscreenBuffer* buffer, int xOffset, int yOffset) {
	uint8_t* row = (uint8_t*)buffer->memory;
	for (int y = 0; y < buffer->height; ++y) {
		uint32_t* pixel = (uint32_t*)row;
		for (int x = 0; x < buffer->width; ++x) {
			uint8_t blue = x + xOffset;
			uint8_t green = y + yOffset;
			*pixel++ = (green << 8 | blue);
		}
		row += buffer->pitch;
	}
}

Win32WindowSize Win32GetWindowSize(HWND hWnd) {
	Win32WindowSize size;
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	size.width = clientRect.right - clientRect.left;
	size.height = clientRect.bottom - clientRect.top;
	return size;
}

void Win32ResizeDIBSection(Win32OffscreenBuffer* buffer, int width, int height) {
	if (buffer->memory) {
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	const int bytesPerPixel = 4;
	buffer->pitch = buffer->width * bytesPerPixel;

	int bitmapMemorySize = (buffer->width * buffer->height) * bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	ZeroMemory(buffer->memory, bitmapMemorySize);
}

void Win32CopyBufferToWindow(HDC hDC, int windowWidth, int windowHeight, Win32OffscreenBuffer* buffer) {
	StretchDIBits(hDC, 0, 0, windowWidth, windowHeight, 0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

void Render(HDC hDC, int windowWidth, int windowHeight, Win32OffscreenBuffer* buffer) {
	static int xOffset = 0;
	static int yOffset = 0;

	RenderGradient(buffer, xOffset, yOffset);
	xOffset += 1;
	yOffset += 1;

	//PatBlt(deviceContext, 0, 0, width, height, 0);
	Win32CopyBufferToWindow(hDC, windowWidth, windowHeight, buffer);

	// Invalidate screen and repost WM_PAINT
	//RedrawWindow(msg.hwnd, 0, 0, RDW_INVALIDATE);
}

void UpdateControllers() {
	for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
		XINPUT_STATE controllerState;
		if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
			XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

			int32_t up = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
			int32_t down = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
			int32_t left = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
			int32_t right = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
			int32_t start = pad->wButtons & XINPUT_GAMEPAD_START;
			int32_t back = pad->wButtons & XINPUT_GAMEPAD_BACK;
			int32_t leftShoulder = pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
			int32_t rightShoulder = pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

			int32_t aButton = pad->wButtons & XINPUT_GAMEPAD_A;
			int32_t bButton = pad->wButtons & XINPUT_GAMEPAD_B;
			int32_t xButton = pad->wButtons & XINPUT_GAMEPAD_X;
			int32_t yButton = pad->wButtons & XINPUT_GAMEPAD_Y;

			int16_t leftThumbStickX = pad->sThumbLX;
			int16_t leftThumbStickY = pad->sThumbLY;

			int16_t rightThumbStickX = pad->sThumbRX;
			int16_t rightThumbStickY = pad->sThumbRY;
		}
	}
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"HandmadeHero";

	if (!RegisterClassEx(&wcex)) {
		return FALSE;
	}

	// Perform application initialization:
	HWND hWnd;
	hWnd = CreateWindow(L"HandmadeHero", L"HandmadeHero Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		return FALSE;
	}

	HDC deviceContext = GetDC(hWnd);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	Win32ResizeDIBSection(&buffer, width, height);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	Running = true;

	// Main message loop:
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

		Win32WindowSize size = Win32GetWindowSize(hWnd);

		//UpdateControllers();
		Render(deviceContext, size.width, size.height, &buffer);
	}

	UnregisterClass(L"HandmadeHero", hInstance);
	ReleaseDC(hWnd, deviceContext);

	return (int)msg.wParam;
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
			break;
		}

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP: {
			uint32_t keyCode = wParam;
			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);
			if (wasDown != isDown) {
				if (keyCode == 'W') {
				}
				else if (keyCode == 'A') {
				}
				else if (keyCode == 'S') {
				}
				else if (keyCode == 'D') {
				}
				else if (keyCode == 'Q') {
				}
				else if (keyCode == 'E') {
				}
				else if (keyCode == VK_UP) {
				}
				else if (keyCode == VK_LEFT) {
				}
				else if (keyCode == VK_DOWN) {
				}
				else if (keyCode == VK_RIGHT) {
				}
				else if (keyCode == VK_ESCAPE) {
					OutputDebugString(L"ESCAPE: ");
					if (isDown) {
						OutputDebugString(L"IsDown ");
					}
					if (wasDown) {
						OutputDebugString(L"WasDown");
					}
					OutputDebugString(L"\n");
				}
				else if (keyCode == VK_SPACE) {
				}
			}

			int32_t altKeyWasDown = (lParam & (1 << 29));
			if ((keyCode == VK_F4) && altKeyWasDown) {
				Running = false;
			}
			break;
		}

		case WM_PAINT: {
			hdc = BeginPaint(hWnd, &ps);

			Win32WindowSize size = Win32GetWindowSize(hWnd);
			//PatBlt(hdc, 0, 0, size.width, size.height, 0);
			Win32CopyBufferToWindow(hdc, size.width, size.height, &buffer);

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

