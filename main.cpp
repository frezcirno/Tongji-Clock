#include <windows.h>
#include <cstdio>

constexpr auto CLIENT_WIDTH = 400;
constexpr auto CLIENT_HEIGHT = 150;
constexpr auto TRANS_COLOR = RGB(249, 201, 201);

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HBRUSH bgBrush = NULL;
RECT wndSize = { 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT };
HDC dc;
SYSTEMTIME time;

wchar_t str[64];

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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
	//DeleteObject(bgBrush);
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
	wcex.lpszClassName = L"Tongji Clock";
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
		szWindowClass, nullptr, WS_POPUP,
		r.left, r.top, CLIENT_WIDTH, CLIENT_HEIGHT,
		GetDesktopWindow(), nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(hWnd, TRANS_COLOR, 255, LWA_COLORKEY);
	bgBrush = CreateSolidBrush(TRANS_COLOR);

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
			SetTimer(hWnd, 1, 1000, NULL); //设定时器
			dc = GetDC(hWnd);
			SetTextColor(dc, RGB(10, 15, 10));
			SetBkMode(dc, TRANSPARENT); // 不画背景色
			DeleteObject(SelectObject(dc, CreateFont(
				50, 20,    //高度50, 宽取20表示由系统选择最佳值  
				0, 0,    //文本倾斜，与字体倾斜都为0  
				FW_HEAVY,    //粗体  
				0, 0, 0,        //非斜体，无下划线，无中划线  
				DEFAULT_CHARSET,    //字符集  
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,        //一系列的默认值  
				DEFAULT_PITCH | FF_DONTCARE,
				L"等线 Light"    //字体名称  
			)));
		}
		break;

	case WM_TIMER:
		{
			if (wParam == 1)
			{
				DATE date;
				GetLocalTime(&time);
				SystemTimeToVariantTime(&time, &date);
				DATE termStartDate = ToDate(2021, 3, 1);
				INT termWeek = (date - termStartDate + 7) / 7;

				wchar_t dateStr[64];
				GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &time, L"dddd", dateStr, _countof(dateStr), NULL);

				wsprintf(str, L"%hu/%hu/%hu\n第%d周 %s\n%02d:%02d:%02d",
					time.wYear, time.wMonth, time.wDay, termWeek, dateStr, time.wHour, time.wMinute, time.wSecond);

				FillRect(dc, &wndSize, bgBrush); // clear window
				DrawText(dc, str, lstrlen(str), &wndSize, DT_CENTER | DT_WORDBREAK | DT_MODIFYSTRING);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0); // 发送移动拖拽窗口移动指令
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}