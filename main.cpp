#include <windows.h>
#include <wininet.h>
#include <cstdio>
#pragma comment(lib, "WinInet.lib")

constexpr INT CLIENT_WIDTH = 400;
constexpr INT CLIENT_HEIGHT = 150;
constexpr COLORREF TRANS_COLOR = RGB(249, 201, 201);
constexpr WCHAR WND_CLASS[] = L"Tongji Clock";// 主窗口类名
RECT wndSize = { 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT };
DATE TERM_START_DATE = 0;

// 全局变量:
HINSTANCE hInst = NULL;                                // 当前实例
HBRUSH hBgBrush = NULL;
HDC dc = NULL;
SYSTEMTIME time;
WCHAR wcHitokoto[64] = { 0 };
WCHAR wcStr[128] = { 0 };

// 此代码模块中包含的函数的前向声明:
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID GetHitokoto();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 初始化全局字符串
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//KillTimer(hWnd, 1);
	//DeleteObject(hBgBrush);
	//ReleaseDC(hWnd, dc);

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;	
	wcex.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = WND_CLASS;
	wcex.hIconSm = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_APPLICATION));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	//获取可用桌面大小  
	RECT r;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	r.left = r.right - CLIENT_WIDTH;
	r.top = r.bottom - CLIENT_HEIGHT;

	HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		WND_CLASS, nullptr, WS_POPUP,
		r.left, r.top, CLIENT_WIDTH, CLIENT_HEIGHT,
		GetDesktopWindow(), nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(hWnd, TRANS_COLOR, 255, LWA_COLORKEY);
	hBgBrush = CreateSolidBrush(TRANS_COLOR);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

DATE ToDate(int year, WORD month, WORD day, WORD h = 0, WORD m = 0, WORD s = 0, WORD ms = 0)
{
	SYSTEMTIME stm = { year, month, 0, day, h, m, s, ms };
	DATE res;
	if (!SystemTimeToVariantTime(&stm, &res)) {/* Handle error */ }
	return res;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			SetTimer(hWnd, 0, 1000, NULL); //设定时器
			SetTimer(hWnd, 1, 3600 * 1000, NULL); //设定时器
			dc = GetDC(hWnd);
			SetTextColor(dc, RGB(165, 165, 165));
			SetBkMode(dc, TRANSPARENT); // 不画背景色
			DeleteObject(SelectObject(dc, CreateFont(
				26, 12, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SCRIPT, L"仿宋"
			)));
			TERM_START_DATE = ToDate(2021, 3, 1);
			GetHitokoto();
		}
		break;

	case WM_TIMER:
		{
			if (wParam == 0)
			{
				DATE date;
				GetLocalTime(&time);
				SystemTimeToVariantTime(&time, &date);
				INT termWeek = (date - TERM_START_DATE + 7) / 7;

				WCHAR wcDateStr[64];
				GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &time, L"dddd", wcDateStr, _countof(wcDateStr), NULL);

				wsprintf(wcStr, L"第%d周 %s\n%hu/%hu/%hu %02d:%02d:%02d\n%s",
					termWeek, wcDateStr, 
					time.wYear, time.wMonth, time.wDay, 
					time.wHour, time.wMinute, time.wSecond,
					wcHitokoto);

				FillRect(dc, &wndSize, hBgBrush); // clear window
				DrawText(dc, wcStr, lstrlen(wcStr), &wndSize, DT_CENTER | DT_WORDBREAK | DT_MODIFYSTRING);
			}
			else if (wParam == 1) {
				GetHitokoto();
			}
		}
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0); // 发送移动拖拽窗口移动指令
		break;

	//case WM_RBUTTONDBLCLK:
	//	GetHitokoto();
	//	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

VOID GetHitokoto()
{
	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	BOOL bRet = FALSE;

	do {
		hInternet = InternetOpenA("WinInetGet/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
		if (NULL == hInternet) break;

		hConnect = InternetConnectA(hInternet, "v1.hitokoto.cn", INTERNET_DEFAULT_HTTPS_PORT
			, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		if (NULL == hConnect) break;

		hRequest = HttpOpenRequestA(hConnect, "GET", "/?c=a&c=b&c=c&encode=text&charset=utf-8",
			NULL, NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_SECURE, 0);
		if (NULL == hRequest) break;

		bRet = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
		if (!bRet) break;

		CHAR szResult[1024] = { 0 };
		CHAR szBuffer[1024] = { 0 };
		INT point = 0;
		DWORD dwByteRead = 0;

		while (InternetReadFile(hRequest, szBuffer, sizeof(szBuffer), &dwByteRead) && dwByteRead > 0)
		{
			memcpy(szResult + point, szBuffer, dwByteRead);
			point += dwByteRead;
			ZeroMemory(szBuffer, dwByteRead);
		}

		MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, szResult, -1, wcHitokoto, _countof(wcHitokoto));
	} while (FALSE);

	if (hInternet) InternetCloseHandle(hInternet);
	if (hConnect) InternetCloseHandle(hConnect);
	if (hRequest) InternetCloseHandle(hRequest);
}