﻿// ComputerGraphicsAlgorithms.cpp : Определяет точку входа для приложения.
//

#include <memory>
#include <algorithm>
#include <cstdlib>
#include <cmath>

#include "framework.h"
#include "main.h"

#include "Game.h"
#include "Obj.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hWnd;
HDC hdc;           
HDC memoryDC;
BITMAP bitmap;
HBITMAP hBitmap;

void DrawScene();

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
	game = std::make_unique<cga::Game>(GetTickCount64, DrawScene);

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

		game->GameCycle();
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
      CW_USEDEFAULT, 0, game->width, game->height, nullptr, nullptr, hInstance, nullptr);

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
    switch (message)
    {
	case WM_CREATE:
		{
			hdc = GetDC(hWnd);
			hBitmap = CreateCompatibleBitmap(hdc, game->width, game->height);
			memoryDC = CreateCompatibleDC(hdc);
			ReleaseDC(hWnd, hdc);
			SelectObject(memoryDC, hBitmap);
			DeleteObject(hBitmap);

			auto brush = CreateSolidBrush(RGB(255, 255, 255));
			auto prev = SelectObject(memoryDC, brush);
			Rectangle(memoryDC, 0, 0, game->width, game->height);
			SelectObject(memoryDC, prev);
			DeleteObject(brush);
		}
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
						game->ReloadScene(fileName);
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
			//game->OnMouseMove(xPos, yPos);
		}
		break;
	case WM_MOUSEWHEEL:
		{
			// TODO ...
			game->OnWheelScroll(-5);
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);
			BitBlt(hdc, 0, 0, game->width, game->height, memoryDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
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

inline void RasterizeLine(HDC dc, glm::vec4 a, glm::vec4 b)
{
	MoveToEx(dc, a.x, a.y, NULL);
	LineTo(dc, b.x, b.y);

	//auto dx = b.x - a.x;
	//auto dy = b.y - a.y;

	//auto steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	//auto Xinc = dx / (float)steps;
	//auto Yinc = dy / (float)steps;

	//auto X = a.x;
	//auto Y = a.y;

	//for (int i = 0; i <= steps; i++)
	//{
	//	SetPixel(dc, X, Y, RGB(0, 0, 0));
	//	X += Xinc;
	//	Y += Yinc;
	//}
}

void DrawScene()
{
	SelectObject(memoryDC, GetStockObject(WHITE_BRUSH));
	Rectangle(memoryDC, 0, 0, game->width, game->height);

	for (const auto& polygon : game->obj.polygons)
	{
		//MoveToEx(memoryDC, game->obj.vertices[polygon.verticesIndices[0] - 1].x, game->obj.vertices[polygon.verticesIndices[0] - 1].y, nullptr);

		for (int i = 0; i < polygon.verticesIndices.size() - 1; i++)
		{
			RasterizeLine(memoryDC, game->obj.vertices[polygon.verticesIndices[i] - 1], game->obj.vertices[polygon.verticesIndices[i + 1] - 1]);
		}

		RasterizeLine(memoryDC, game->obj.vertices[polygon.verticesIndices[polygon.verticesIndices.size() - 1] - 1], game->obj.vertices[polygon.verticesIndices[0] - 1]);

		//LineTo(memoryDC, game->obj.vertices[polygon.verticesIndices[0] - 1].x, game->obj.vertices[polygon.verticesIndices[0] - 1].y);
	}

	InvalidateRect(hWnd, NULL, false);
}