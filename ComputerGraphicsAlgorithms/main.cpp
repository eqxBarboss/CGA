// ComputerGraphicsAlgorithms.cpp : Определяет точку входа для приложения.
//

#include <memory>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <functional>

#include "framework.h"
#include "main.h"

#include "Game.h"

#define MAX_LOADSTRING 100
#define WIDTH 1920
#define HEIGHT 1080
#define IDT_TIMER 1

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hWnd;

void OnInvalidated();
void OnCreate(HWND hWnd);

std::unique_ptr<cga::Game> game;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	game = std::make_unique<cga::Game>(GetTickCount64, OnInvalidated, WIDTH, HEIGHT);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COMPUTERGRAPHICSALGORITHMS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COMPUTERGRAPHICSALGORITHMS));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COMPUTERGRAPHICSALGORITHMS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_COMPUTERGRAPHICSALGORITHMS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, WIDTH, HEIGHT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

    switch (message)
    {
	case WM_CREATE:
		{
            OnCreate(hWnd);
		}
		break;
	case WM_TIMER:
		game->GameCycle();
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
			case IDM_OPEN:
				{
					OPENFILENAME ofn;       // common dialog box structure
					TCHAR szFile[260] = { 0 };       // if using TCHAR macros

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if (GetOpenFileName(&ofn) == TRUE)
					{
						auto fileName = std::string(ofn.lpstrFile);
						game->LoadScene(fileName);
					}
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_KEYDOWN:
		game->OnKeyDown(wParam);
		break;
	case WM_KEYUP:
		game->OnKeyUp(wParam);
		break;
	case WM_MOUSEMOVE:
		{
			auto xPos = GET_X_LPARAM(lParam);
			auto yPos = GET_Y_LPARAM(lParam);
			game->OnMouseMove(xPos, yPos);
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			auto memoryDC = CreateCompatibleDC(hdc);
			HBITMAP map = CreateBitmap(WIDTH, HEIGHT, 1, 8 * sizeof(COLORREF), (void*)game->GetCurrentBuffer().data);
			auto old = SelectObject(memoryDC, map);

			BitBlt(hdc, 0, 0, WIDTH, HEIGHT, memoryDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);

			SelectObject(memoryDC, old);
			DeleteObject(map);
			DeleteDC(memoryDC);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION)  // Nothing to do :(
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);


	switch (wParam)
	{
	case WM_MOUSEWHEEL:
		const auto delta = static_cast<std::make_signed_t<WORD>>(HIWORD(info->mouseData));
		game->OnWheelScroll(delta / 20);
		break;
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void OnInvalidated()
{
	InvalidateRect(hWnd, NULL, false);
}

void OnCreate(HWND hWnd)
{
	SetWindowsHookExW(WH_MOUSE_LL, MouseHookProc, nullptr, 0);
	SetTimer(hWnd, IDT_TIMER, 15, (TIMERPROC)NULL);
}