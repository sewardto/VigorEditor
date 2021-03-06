//MiniWord.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MiniWord.h"

#define MAX_LOADSTRING 100
#define MAXL 500

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HDC hdc, Bkhdc;
HBITMAP hBitmap;
BITMAP s_bm;
PAINTSTRUCT ps;
UINT uFindReplaceMsg;  // message identifier for FINDMSGSTRING

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Welcome(HWND, UINT, WPARAM, LPARAM);
void openNewFile(wchar_t * Address, HDC hdc); //打开文件
void saveFile(wchar_t * Address); //保存文件
void createNewFile(HDC hdc); //新建文件
void wstrcpy(wchar_t * desStr, const wchar_t * srcStr);//复制宽字符串

//Window
static int nCharX, nCharY;						//字符长宽
static int nWindowX, nWindowY;					//页面长宽
static int nWindowCharX, nWindowCharY;			//页面最大字符
static int scrFirstLine, scrLastLine;			//输出篇幅
												// Carets
class Caret crt;

int x, y;										//字符位置
TEXTMETRIC tm;									//字体信息

//Scroll
SCROLLINFO si;
static int xScrPos, yScrPos;
#define ScrCrtPosX ((crt.CaretPosX) - (nCharX) * (xScrPos - 4))
#define ScrCrtPosY ((crt.CaretPosY) - (yScrPos))

//Select
static COLORREF crPrevText;
static COLORREF crPrevBk;
static bool fTextSelected = false;

//Select
selectPos selectBegin;	//{x, y}
selectPos selectEnd;

Article Ar;
line tmpL = Ar.GetLine(0);

wchar_t curFilePath[MAX_PATH + 1] = { 0 }; //记录正在编辑的文件的地址
bool saveMark = false; //记录当前文件点击保存时是否需要选择地址；
bool isModified = false; //记录当前文件是否已更改

/*搜索用*/
FINDREPLACE fr;       // common dialog box structure
wchar_t szFindWhat[MAXL];  // buffer receiving string
HWND hdlg = NULL;     // handle to Find dialog box
bool MarkSearch = false;

/*替换用*/
FINDREPLACE rp;       // common dialog box structure
wchar_t rpFindWhat[MAXL];  // buffer receiving string
wchar_t rpReplaceWith[MAXL]; //将要替换上去的串
HWND rdlg = NULL;     // handle to Replace dialog box
bool  replaceMark = false;

bool gobacktofirst = false;
//Background
HINSTANCE g_hInstance = NULL;

/*shift标记*/
bool ShiftPressed = false;

void paint(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MINIWORD, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINIWORD));
	uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINIWORD));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINIWORD);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MINIWORD));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);


	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPFINDREPLACE lpfr;

	if (message == uFindReplaceMsg)
	{
		// Get pointer to FINDREPLACE structure from lParam.
		lpfr = (LPFINDREPLACE)lParam;


		// If the FR_DIALOGTERM flag is set,
		// invalidate the handle that identifies the dialog box.
		if (lpfr->Flags & FR_DIALOGTERM)
		{
			hdlg = NULL;
			return 0;
		}

		// If the FR_FINDNEXT flag is set,
		// call the application-defined search routine
		// to search for the requested string.
		if (lpfr->Flags & FR_FINDNEXT)
		{
			tmpL = Ar.GetLine(crt.CaretPosY);

			line Res = Ar.onSearch(tmpL, (const wchar_t *)lpfr->lpstrFindWhat);
			if (Res) {

				hdc = GetDC(hWnd);
				if (fTextSelected) {
					/*SetTextColor(hdc, crPrevText);
					SetTextColor(hdc, crPrevBk);
					InvalidateRect(hWnd, NULL, TRUE);*/
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					fTextSelected = false;
				}

				tmpL = Res;
				crt.CaretPosX = tmpL->CharWidth(LF, hdc);
				crt.CaretPosY = Ar.GetNum(tmpL);
				SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);

				selectBegin = { tmpL->Getlen(LF) - wcslen(lpfr->lpstrFindWhat), crt.CaretPosY };
				selectEnd = { tmpL->Getlen(LF), crt.CaretPosY };
				fTextSelected = true;
				SendMessage(hWnd, WM_MASK, 0, 1L);

				ReleaseDC(hWnd, hdc);
				//MessageBox(hWnd, L"查找成功！", L"提示", MB_OK);
				MarkSearch = true;
				}
			else {
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					fTextSelected = false;
				}
				MessageBox(hWnd, L"对不起，没有找到您想要的内容", L"提示", MB_OK);
				crt.CaretPosX = 0;
				crt.CaretPosY = 0;
				SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
				tmpL->Gapmove();
				tmpL = Ar.L;
				tmpL->PointMoveto(0);
				paint(hWnd);

			}
		}

		if (lpfr->Flags & FR_REPLACE) {

			if (MarkSearch) {

				line Res = Ar.OnReplace(tmpL, lpfr->lpstrFindWhat, lpfr->lpstrReplaceWith);
				tmpL = Res;

				hdc = GetDC(hWnd);
				crt.CaretPosX = tmpL->CharWidth(LF, hdc);
				crt.CaretPosY = Ar.GetNum(tmpL);
				SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
				InvalidateRect(hWnd, NULL, TRUE);
				fTextSelected = false;
				ReleaseDC(hWnd, hdc);

				MarkSearch = false;
				replaceMark = true;

			}
			else {

				tmpL = Ar.GetLine(crt.CaretPosY);
				line Res = Ar.onSearch(tmpL, (const wchar_t *)lpfr->lpstrFindWhat);
				if (Res) {

					if (fTextSelected) {
						SendMessage(hWnd, WM_DEMASK, 0, 1L);
						fTextSelected = false;
					}

					line Res_1 = Ar.OnReplace(Res, lpfr->lpstrFindWhat, lpfr->lpstrReplaceWith);
					tmpL = Res_1;

					HDC hdc = GetDC(hWnd);
					crt.CaretPosX = tmpL->CharWidth(LF, hdc);
					crt.CaretPosY = Ar.GetNum(tmpL);
					SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
					InvalidateRect(hWnd, NULL, TRUE);
					fTextSelected = false;
					ReleaseDC(hWnd, hdc);

					MarkSearch = false;
					replaceMark = true;

				}
				else {
					MessageBox(hWnd, L"对不起，没有找到待替换的内容", L"提示", MB_OK);
				}

			}

		}
		return 0;
	}

	//if (replaceMark) {
	//	SendMessage(hWnd, WM_MASK, 0, 1L);
	//	replaceMark = false;
	//	return 0;
	//}

	switch (message)
	{
	case WM_CREATE: {

		hdc = GetDC(hWnd);
		SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
		GetTextMetricsW(hdc, &tm);

		nCharX = tm.tmAveCharWidth;
		nCharY = tm.tmHeight;

		LPWSTR *szArglist = NULL;
		int nArgs = 0;
		szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
		if (szArglist[1] != NULL)
		{
			wstrcpy(curFilePath, szArglist[1]); //保存下来当前保存的文件的地址
			saveMark = true;
			openNewFile(szArglist[1], hdc);
			isModified = false;

		}
		LocalFree(szArglist);

		ReleaseDC(hWnd, hdc);

		DialogBox(hInst, MAKEINTRESOURCE(IDD_WELCOM), hWnd, Welcome);

		return 0;
	}

	//case WM_INITDIALOG: {
	//	// 设置对话框大小可调节
	//	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_SIZEBOX);
	//	// 加载背影图片
	//	hBitmap = (HBITMAP)LoadImage(NULL, L"background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	//	if (hBitmap == NULL)
	//	{
	//		MessageBox(hWnd, L"LoadImage failed", L"Error", MB_ICONERROR);
	//		exit(0);
	//	}
	//	else
	//	{
	//		// 将背影图片放入HDC - Bkhdc
	//		hdc = GetDC(hWnd);
	//		Bkhdc= CreateCompatibleDC(hdc);
	//		SelectObject(Bkhdc, hBitmap);
	//		ReleaseDC(hWnd, hdc);
	//		// 得到位图信息
	//		GetObject(hBitmap, sizeof(s_bm), &s_bm);
	//	}
	//	return 0;
	//}
	//

	case WM_SETFOCUS:
		CreateCaret(hWnd, (HBITMAP)NULL, 1, nCharY);
		SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
		ShowCaret(hWnd);

		return 0;

	case WM_SIZE:
		hdc = GetDC(hWnd);
		nWindowY = HIWORD(lParam);
		nWindowX = LOWORD(lParam);
		nWindowCharX = nWindowX / nCharX;
		nWindowCharY = nWindowY / nCharY;

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = Ar.LineNum();
		si.nPage = nWindowY / nCharY;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 6 + Ar.MaxWidth(hdc) / nCharX;
		si.nPage = nWindowX / nCharX;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		paint(hWnd);
		SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
		SendMessage(hWnd, WM_MASK, 0, 0);

		ReleaseDC(hWnd, hdc);
		return 0;

	case WM_IME_STARTCOMPOSITION:
	{
		HIMC hImc = ImmGetContext(hWnd);
		COMPOSITIONFORM Composition;
		Composition.dwStyle = CFS_POINT;
		Composition.ptCurrentPos.x = ScrCrtPosX; // ime follow
		Composition.ptCurrentPos.y = crt.CaretPosY*nCharY; // set caret position
		ImmSetCompositionWindow(hImc, &Composition);
		ImmReleaseContext(hWnd, hImc);
		return 0;
	}

	case WM_HSCROLL:
		// Get all the vertial scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;

		// Save the position for comparison later on.
		GetScrollInfo(hWnd, SB_HORZ, &si);
		xScrPos = si.nPos;
		switch (LOWORD(wParam))
		{
			// User clicked the left arrow.
		case SB_LINELEFT:
			si.nPos -= 1;
			break;
			// User clicked the right arrow.
		case SB_LINERIGHT:
			si.nPos += 1;
			break;
			// User clicked the scroll bar shaft left of the scroll box.
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;
			// User clicked the scroll bar shaft right of the scroll box.
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;
			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hWnd, SB_HORZ, &si);

		// If the position has changed, scroll the window.
		if (si.nPos != xScrPos)
		{
			ScrollWindow(hWnd, nCharX *(xScrPos - si.nPos), 0, NULL, NULL);
			UpdateWindow(hWnd);
			xScrPos = si.nPos;
		}

		return 0;

	case WM_VSCROLL:
		// Get all the vertial scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);

		// Save the position for comparison later on.
		yScrPos = si.nPos;
		switch (LOWORD(wParam))
		{

			// User clicked the HOME keyboard key.
		case SB_TOP:
			si.nPos = si.nMin;
			break;
			// User clicked the END keyboard key.
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
			// User clicked the top arrow.
		case SB_LINEUP:
			si.nPos -= 1;
			break;
			// User clicked the bottom arrow.
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
			// User clicked the scroll bar shaft above the scroll box.
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
			// User clicked the scroll bar shaft below the scroll box.
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);

		// If the position has changed, scroll window and update it.
		if (si.nPos != yScrPos)
		{
			ScrollWindow(hWnd, 0, nCharY * (yScrPos - si.nPos), NULL, NULL);
			UpdateWindow(hWnd);
			yScrPos = si.nPos;
		}

		return 0;

	case WM_KEYDOWN: {
		hdc = GetDC(hWnd);

		switch (wParam)
		{
		case VK_SHIFT: {
			ShiftPressed = true;
			break;
		}

		case VK_HOME:
			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					crt.MvHome(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					fTextSelected = true;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					crt.MvHome(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
			}
			else {
				if (fTextSelected) {
					SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, 0);
					hdc = GetDC(hWnd);
				}
				crt.MvHome(tmpL, hdc);
			}
			break;

		case VK_END:
			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);

					crt.MvEnd(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					fTextSelected = true;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					crt.MvEnd(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
			}
			else {
				if (fTextSelected) {
					SendMessage(hWnd, WM_KEYDOWN, VK_RIGHT, 0);
					hdc = GetDC(hWnd);
				}
				crt.MvEnd(tmpL, hdc);
			}
			break;

		case VK_PRIOR:
			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					tmpL = crt.CtrCaretMv(Ar, crt.CaretPosX, max(0, crt.CaretPosY - nWindowCharY - 1), hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					fTextSelected = true;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					tmpL = crt.CtrCaretMv(Ar, crt.CaretPosX, max(0, crt.CaretPosY - nWindowCharY - 1), hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
				break;
			}
			if (fTextSelected) {
				SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, 0);
				hdc = GetDC(hWnd);
			}
			tmpL = crt.CtrCaretMv(Ar, crt.CaretPosX, max(0, crt.CaretPosY - nWindowCharY - 1), hdc);
			break;

		case VK_NEXT:
			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					tmpL = crt.CtrCaretMv(Ar, crt.CaretPosX, min(Ar.LineNum() - 1, crt.CaretPosY + nWindowCharY + 1), hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					fTextSelected = true;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					tmpL = crt.CtrCaretMv(Ar, crt.CaretPosX, min(Ar.LineNum() - 1, crt.CaretPosY + nWindowCharY + 1), hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
				break;
			}

			if (fTextSelected) {
				SendMessage(hWnd, WM_KEYDOWN, VK_RIGHT, 0);
				hdc = GetDC(hWnd);
			}
			tmpL = crt.CtrCaretMv(Ar, crt.CaretPosX, min(Ar.LineNum() - 1, crt.CaretPosY + nWindowCharY + 1), hdc);

			break;

		/*注：发送左移右移信号后，如果需要继续执行涉及到hdc的操作,请在所调用函数前重新获取hdc:  hdc = GetDC(hWnd);*/
		/*同理发送删除信号后，也需要重新hdc = GetDC(hWnd); 因为两个操作最后hdc都被Release了*/
		case VK_LEFT:

			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);

					crt.MvLeft(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					fTextSelected = true;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					crt.MvLeft(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
			}
			else if (fTextSelected) {
				selectPos fronter = (selectBegin.second < selectEnd.second) || (selectBegin.second == selectEnd.second&&selectBegin.first <= selectEnd.first) ? selectBegin : selectEnd;

				tmpL->Gapmove();
				tmpL = Ar.GetLine(fronter.second);
				tmpL->PointMoveto(fronter.first);
				crt.CaretPosX = tmpL->CharWidth(LF, hdc);
				crt.CaretPosY = fronter.second;

				SetTextColor(hdc, crPrevText);
				SetTextColor(hdc, crPrevBk);
				InvalidateRect(hWnd, NULL, TRUE);
				fTextSelected = false;

			}
			else crt.MvLeft(tmpL, hdc);
			break;

		case VK_RIGHT:

			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					crt.MvRight(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}

				else {
						fTextSelected = true;
						selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
						crt.MvRight(tmpL, hdc);
						selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
						SendMessage(hWnd, WM_MASK, 0, 1L);
				}

			}
			else  if (fTextSelected) {
				selectPos backer = (selectBegin.second < selectEnd.second) || (selectBegin.second == selectEnd.second&&selectBegin.first <= selectEnd.first) ? selectEnd : selectBegin;

				tmpL->Gapmove();
				tmpL = Ar.GetLine(backer.second);
				tmpL->PointMoveto(backer.first);
				crt.CaretPosX = tmpL->CharWidth(LF, hdc);
				crt.CaretPosY = backer.second;

				SetTextColor(hdc, crPrevText);
				SetTextColor(hdc, crPrevBk);
				InvalidateRect(hWnd, NULL, TRUE);
				fTextSelected = false;
			}
			else crt.MvRight(tmpL, hdc);
			break;

		case VK_UP:
			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					crt.MvUp(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					crt.MvUp(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					fTextSelected = true;
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
			}
			else {
				if (fTextSelected) {
					SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, 0);
					hdc = GetDC(hWnd);
				}
				crt.MvUp(tmpL, hdc);
			}
			break;

		case VK_DOWN:
			if (ShiftPressed) {
				HideCaret(hWnd);
				if (fTextSelected) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);
					crt.MvDown(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);

					if (selectBegin.second == selectEnd.second && selectBegin.first == selectEnd.first) {
						fTextSelected = false;
					}
				}
				else {
					fTextSelected = true;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };
					crt.MvDown(tmpL, hdc);
					selectEnd = { tmpL->Getlen(LF),crt.CaretPosY };
					SendMessage(hWnd, WM_MASK, 0, 0);
				}
			}
			else {
				if (fTextSelected) {
					SendMessage(hWnd, WM_KEYDOWN, VK_RIGHT, 0);
					hdc = GetDC(hWnd);
				}
				crt.MvDown(tmpL, hdc);
			}
			break;

		case VK_DELETE:

			if (fTextSelected) {
				selectPos p = Ar.Delete(selectBegin.second, selectBegin.first, selectEnd.second, selectEnd.first);
				crt.CaretPosY = p.second;
				crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);
				tmpL = Ar.GetLine(crt.CaretPosY);
				fTextSelected = false;
				if (ShiftPressed) {
					ShiftPressed = false;
					selectBegin = { tmpL->Getlen(LF),crt.CaretPosY };

				}
				Ar.Emptyredo();
			}
			else {
				crt.CtrDelete(Ar, tmpL, hdc);
			}
			ShowCaret(hWnd);
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);

			InvalidateRect(hWnd, NULL, TRUE);
			isModified = true;

			break;
		}

		ShowCaret(hWnd);
		SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);

		if (ScrCrtPosX >= nWindowX - 6)
			SendMessage(hWnd, WM_HSCROLL, SB_PAGEDOWN, 0);
		else if (ScrCrtPosY * nCharY >= nWindowY - 2)
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
		else if (ScrCrtPosX <= 2)
			SendMessage(hWnd, WM_HSCROLL, SB_PAGEUP, 0);
		else if (ScrCrtPosY <= 2)
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);

		//if (ScrCrtPosX > nWindowX || ScrCrtPosX <= 0)
		//	SetScrollRange(hWnd, SB_HORZ, 0, 2 + Ar.MaxWidth(hdc) / nCharX, 0);
		//if (ScrCrtPosY * nCharY > nWindowY || ScrCrtPosY < 0)
		//	SetScrollRange(hWnd, SB_VERT, 0, Ar.LineNum(), 0);

		ReleaseDC(hWnd, hdc);
		return 0;
	}

	case WM_KEYUP: {
		switch (wParam) {
		case VK_SHIFT:
			ShiftPressed = false;
		}
	}

	case WM_CONTEXTMENU:
	{
		RECT rect;
		POINT pt;
		// 获取鼠标右击是的坐标
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		//获取客户区域大小
		GetClientRect(hWnd, &rect);
		//把屏幕坐标转为客户区坐标
		ScreenToClient(hWnd, &pt);
		//判断点是否位于客户区域内
		if (PtInRect(&rect, pt))
		{
			//加载菜单资源
			HMENU hroot = LoadMenu((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDR_MENU1));
			if (hroot) {
				// 获取第一个弹出菜单
				HMENU hpop = GetSubMenu(hroot, 0);
				// 把客户区坐标还原为屏幕坐标
				ClientToScreen(hWnd, &pt);
				//显示快捷菜单
				TrackPopupMenu(hpop,
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
					pt.x,
					pt.y,
					0,
					hWnd,
					NULL);
				// 用完后要销毁菜单资源
				DestroyMenu(hroot);
			}
		}
		else {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	case WM_CHAR:
	{
		hdc = GetDC(hWnd);
		HideCaret(hWnd);
		if (fTextSelected) {
			SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 1L);
			fTextSelected = FALSE;
			if (wParam != 0x08)
			{
				SendMessage(hWnd, WM_CHAR, wParam, 1L);
				undo a = new Undo();
				Ar.UndoStack.push(a);
			}
			return 0;
		}
		switch (wParam)
		{
		case 0x08:
			/*Process a backspace.*/

			if (crt.CaretPosX == 0 && tmpL->IsFirstL()) break;
			crt.MvLeft(tmpL, hdc);
			SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 1L);
			break;
		case 0x09:

			Ar.Insert(crt.CaretPosY, L"\t");
			crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);
			Ar.Emptyredo();
			InvalidateRect(hWnd, NULL, TRUE);

			break;
		case 0x0D:
			crt.CtrEnter(Ar, tmpL, hdc);

			InvalidateRect(hWnd, NULL, TRUE);
			//Process an carriage return.

			break;

		default: {
			tmpL->Push((wchar_t)wParam, LF);
			undo a = new Undo(selectPos{ tmpL->Getlen(LF) - 1,crt.CaretPosY }, selectPos{ tmpL->Getlen(LF), crt.CaretPosY });
			Ar.UndoStack.push(a);
			Ar.Emptyredo();

			INT nCharWidth;
			GetCharWidth32W(hdc, (UINT)wParam, (UINT)wParam,
				&nCharWidth);
			crt.CaretPosX += nCharWidth;

			InvalidateRect(hWnd, NULL, TRUE);
			isModified = true;
		}
		}

		tmpL = Ar.GetLine(crt.CaretPosY);

		if (ScrCrtPosX >= nWindowX - 2 * nCharX || ScrCrtPosX <= 0)
			SetScrollRange(hWnd, SB_HORZ, 0, 6 + Ar.MaxWidth(hdc) / nCharX, 0);
		if (ScrCrtPosY * nCharY >= nWindowY - 4 * nCharY || ScrCrtPosY <= 0)
			SetScrollRange(hWnd, SB_VERT, 0, Ar.LineNum(), 0);

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &si);
		yScrPos = si.nPos;

		GetScrollInfo(hWnd, SB_HORZ, &si);
		xScrPos = si.nPos;

		SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
		ShowCaret(hWnd);
		ReleaseDC(hWnd, hdc);
		return 0;
	}
	/*鼠标点击事件*/
	case WM_LBUTTONDOWN:
	{
		if (fTextSelected) {
			hdc = GetDC(hWnd);
			SetTextColor(hdc, crPrevText);
			SetTextColor(hdc, crPrevBk);
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);
			fTextSelected = false;
		}

		hdc = GetDC(hWnd);
		POINTS ptsCursor = MAKEPOINTS(lParam);
		int y = min(ptsCursor.y / nCharY + yScrPos, Ar.LineNum() - 1);
		tmpL = Ar.GetLine(y);
		int x = min(ptsCursor.x + (xScrPos - 4) * nCharX, tmpL->CharWidth(hdc));
		tmpL = crt.CtrCaretMv(Ar, x, y, hdc);

		SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
		ReleaseDC(hWnd, hdc);
		selectBegin = { tmpL->Getlen(LF), crt.CaretPosY };
		selectEnd = selectBegin;

		return 0;
	}

	case WM_LBUTTONDBLCLK:
		tmpL = Ar.GetLine(crt.CaretPosY);

		tmpL->Gapmove();

		HideCaret(hWnd);
		hdc = GetDC(hWnd);
		crt.CaretPosX = tmpL->CharWidth(hdc);

		crPrevText = SetTextColor(hdc, RGB(255, 255, 255));
		crPrevBk = SetBkColor(hdc, RGB(0, 0, 0));

		TextOut(hdc, 4 * nCharX, ScrCrtPosY * nCharY, tmpL->GetPos(LF),
			tmpL->Getlen(LF));
		TextOut(hdc, tmpL->CharWidth(LF, hdc) + 4 * nCharX,
			ScrCrtPosY * nCharY, tmpL->GetPos(RG), tmpL->Getlen(RG));
		SetTextColor(hdc, crPrevText);
		SetBkColor(hdc, crPrevBk);
		ReleaseDC(hWnd, hdc);
		ShowCaret(hWnd);
		fTextSelected = TRUE;

		SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
		selectBegin = { 0, crt.CaretPosY };
		selectEnd = { tmpL->len, crt.CaretPosY };

		return 0;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON) {
			HideCaret(hWnd);
			hdc = GetDC(hWnd);
			bool changed = false;

			Caret tmpCaret = crt;

			POINTS ptsEnd = MAKEPOINTS(lParam);
			int y = min(ptsEnd.y / nCharY + yScrPos, Ar.LineNum() - 1);

			if (y >= 0 && y < crt.CaretPosY) {
				SendMessage(hWnd, WM_DEMASK, 0, 0);

				while (y < crt.CaretPosY)
					crt.MvUp(tmpL, hdc);
				changed = true;
			}
			else if (y > crt.CaretPosY) {
				SendMessage(hWnd, WM_DEMASK, 0, 0);
				while (y > crt.CaretPosY) {
					crt.MvDown(tmpL, hdc);
				}
				changed = true;
			}
			selectEnd = { tmpL->Getlen(LF), crt.CaretPosY };

			int x = min(ptsEnd.x + (xScrPos - 4) * nCharX, tmpL->CharWidth(hdc));
			x = max(0, x);

			if (x != crt.CaretPosX) {
				int nRightCharWidth, nLeftCharWidth;

				GetCharWidth32(hdc, (UINT)tmpL->Top(RG), (UINT)tmpL->Top(RG),
					&nRightCharWidth);
				GetCharWidth32(hdc, (UINT)tmpL->Top(LF), (UINT)tmpL->Top(LF),
					&nLeftCharWidth);

				if (x - crt.CaretPosX >= nRightCharWidth) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);

					while (x - crt.CaretPosX > nRightCharWidth) {
						crt.MvRight(tmpL, hdc);
						GetCharWidth32(hdc, (UINT)tmpL->arr[tmpL->gend], (UINT)tmpL->arr[tmpL->gend],
							&nRightCharWidth);
					}
					crt.MvRight(tmpL, hdc);
					changed = true;
				}
				else if (crt.CaretPosX - x >= nLeftCharWidth) {
					SendMessage(hWnd, WM_DEMASK, 0, 0);

					while (crt.CaretPosX - x > nLeftCharWidth) {
						crt.MvLeft(tmpL, hdc);
						GetCharWidth32(hdc, (UINT)tmpL->arr[tmpL->gstart], (UINT)tmpL->arr[tmpL->gstart],
							&nLeftCharWidth);
					}
					crt.MvLeft(tmpL, hdc);
					changed = true;
				}
			}
			if (changed) {

				SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
				selectEnd = { tmpL->Getlen(LF), crt.CaretPosY };
				fTextSelected = true;
				SendMessage(hWnd, WM_MASK, 0, 0);
			}
			ReleaseDC(hWnd, hdc);
			ShowCaret(hWnd);
		}
		return 0;

	case WM_MASK:
		if (fTextSelected) {

			hdc = GetDC(hWnd);

			int xWidth = 0, nCharWidth;
			line ttmpL;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(hWnd, SB_VERT, &si);
			yScrPos = si.nPos;
			GetScrollInfo(hWnd, SB_HORZ, &si);
			xScrPos = si.nPos;

			x = nCharX * (-xScrPos);
			y = nCharY * (-yScrPos);

			crPrevText = SetTextColor(hdc, RGB(255, 255, 255));
			crPrevBk = SetBkColor(hdc, RGB(73, 90, 128));
			if (selectEnd.second > selectBegin.second) {
				ttmpL = Ar.GetLine(selectBegin.second);
				for (int i = 0; i != selectBegin.first; ++i) {
					GetCharWidth32(hdc, (UINT)ttmpL->arr[i], (UINT)ttmpL->arr[i], &nCharWidth);
					xWidth += nCharWidth;
				}
				TextOut(hdc, x + xWidth + 4 * nCharX, (selectBegin.second)* nCharY + y, ttmpL->GetPos(LF) + selectBegin.first,
					ttmpL->Getlen(LF) - selectBegin.first);
				for (int i = selectBegin.second + 1; i <= selectEnd.second; i++) {
					ttmpL = ttmpL->next;
					TextOut(hdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
						ttmpL->Getlen(LF));
				}
			}
			else if (selectEnd.second < selectBegin.second) {
				ttmpL = Ar.GetLine(selectEnd.second);
				for (int i = 0; i != selectEnd.first; ++i) {
					GetCharWidth32(hdc, (UINT)ttmpL->arr[i], (UINT)ttmpL->arr[i], &nCharWidth);
					xWidth += nCharWidth;
				}
				TextOut(hdc, x + xWidth + 4 * nCharX, (selectEnd.second)* nCharY + y, ttmpL->GetPos(RG),
					ttmpL->Getlen(RG));
				int i;
				for (i = selectEnd.second + 1; i < selectBegin.second; i++) {
					ttmpL = ttmpL->next;
					TextOut(hdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
						ttmpL->Getlen(LF));
				}
				ttmpL = ttmpL->next;

				TextOut(hdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
					selectBegin.first);

			}
			else {
				ttmpL = Ar.GetLine(selectEnd.second);
				if (selectEnd.first - selectBegin.first > 0) {
					for (int i = 0; i != selectBegin.first; ++i) {
						GetCharWidth32(hdc, (UINT)tmpL->arr[i], (UINT)tmpL->arr[i], &nCharWidth);
						xWidth += nCharWidth;
					}
					TextOut(hdc, x + xWidth + 4 * nCharX, (selectBegin.second)* nCharY + y, tmpL->GetPos(LF) + selectBegin.first,
						selectEnd.first - selectBegin.first);
				}
				else if (selectEnd.first - selectBegin.first < 0) {
					for (int i = 0; i != selectEnd.first; ++i) {
						GetCharWidth32(hdc, (UINT)tmpL->arr[i], (UINT)tmpL->arr[i], &nCharWidth);
						xWidth += nCharWidth;
					}
					TextOut(hdc, x + xWidth + 4 * nCharX, (selectBegin.second)* nCharY + y, tmpL->GetPos(RG),
						selectBegin.first - selectEnd.first);
				}
			}
			SetTextColor(hdc, crPrevText);
			SetBkColor(hdc, crPrevBk);

			ReleaseDC(hWnd, hdc);
		}
		return 0;

	case WM_DEMASK: {
		/*if (fTextSelected)
		*/

		hdc = GetDC(hWnd);
		int xWidth = 0, nCharWidth;
		line ttmpL;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &si);
		yScrPos = si.nPos;
		GetScrollInfo(hWnd, SB_HORZ, &si);
		xScrPos = si.nPos;

		x = nCharX * (-xScrPos);
		y = nCharY * (-yScrPos);

		crPrevText = SetTextColor(hdc, RGB(0, 0, 0));
		crPrevBk = SetBkColor(hdc, RGB(255, 255, 255));
		if (selectEnd.second > selectBegin.second) {
			ttmpL = Ar.GetLine(selectBegin.second);
			for (int i = 0; i != selectBegin.first; ++i) {
				GetCharWidth32(hdc, (UINT)ttmpL->arr[i], (UINT)ttmpL->arr[i], &nCharWidth);
				xWidth += nCharWidth;
			}
			TextOut(hdc, x + xWidth + 4 * nCharX, (selectBegin.second)* nCharY + y, ttmpL->GetPos(LF) + selectBegin.first,
				ttmpL->Getlen(LF) - selectBegin.first);
			for (int i = selectBegin.second + 1; i <= selectEnd.second; i++) {
				ttmpL = ttmpL->next;
				TextOut(hdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
					ttmpL->Getlen(LF));
			}
		}
		else if (selectEnd.second < selectBegin.second) {
			ttmpL = Ar.GetLine(selectEnd.second);
			for (int i = 0; i != selectEnd.first; ++i) {
				GetCharWidth32(hdc, (UINT)ttmpL->arr[i], (UINT)ttmpL->arr[i], &nCharWidth);
				xWidth += nCharWidth;
			}
			TextOut(hdc, x + xWidth + 4 * nCharX, (selectEnd.second)* nCharY + y, ttmpL->GetPos(RG),
				ttmpL->Getlen(RG));
			int i;
			for (i = selectEnd.second + 1; i < selectBegin.second; i++) {
				ttmpL = ttmpL->next;
				TextOut(hdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
					ttmpL->Getlen(LF));
			}
			ttmpL = ttmpL->next;

			TextOut(hdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
				selectBegin.first);

		}
		else {
			ttmpL = Ar.GetLine(selectEnd.second);
			if (selectEnd.first - selectBegin.first > 0) {
				for (int i = 0; i != selectBegin.first; ++i) {
					GetCharWidth32(hdc, (UINT)tmpL->arr[i], (UINT)tmpL->arr[i], &nCharWidth);
					xWidth += nCharWidth;
				}
				TextOut(hdc, x + xWidth + 4 * nCharX, (selectBegin.second)* nCharY + y, tmpL->GetPos(LF) + selectBegin.first,
					selectEnd.first - selectBegin.first);
			}
			else if (selectEnd.first - selectBegin.first < 0) {
				for (int i = 0; i != selectEnd.first; ++i) {
					GetCharWidth32(hdc, (UINT)tmpL->arr[i], (UINT)tmpL->arr[i], &nCharWidth);
					xWidth += nCharWidth;
				}
				TextOut(hdc, x + xWidth + 4 * nCharX, (selectBegin.second)* nCharY + y, tmpL->GetPos(RG),
					selectBegin.first - selectEnd.first);
			}
		}
		SetTextColor(hdc, crPrevText);
		SetBkColor(hdc, crPrevBk);
	}
		return 0;


	case WM_KILLFOCUS:

		DestroyCaret();

		return 0;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);

		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_OPEN: //打开新文件
		{
			if (isModified == true) {
				//弹出消息框
				int X = MessageBox(NULL, _T("是否保存文件？"), _T("提示消息"), MB_YESNOCANCEL | MB_ICONQUESTION);
				if (X == IDCANCEL) return 0;//如果取消则不新建
				if (X == IDYES) {
					SendMessage(hWnd, WM_COMMAND, IDM_SAVE, 1L);
				}
			}

			wchar_t    szFileName[MAX_PATH] = { 0 };
			OPENFILENAME openFile = { 0 };
			openFile.lStructSize = sizeof(OPENFILENAME);
			openFile.nMaxFile = MAX_PATH;
			openFile.lpstrFile = szFileName;

			openFile.lpstrFilter = L"MarkDown(*.md)\0*.md\0Js(*.js)\0*.js\0Bat(*.bat)\0*.bat\0Text(*.txt)\0*.txt\0C++(*.cpp)\0*.cpp\0C头文件(*.h)\0*.h\0All Files(*.*)\0*.*\0\0";
			openFile.lpstrDefExt = (LPCWSTR)L"txt";
			openFile.nFilterIndex = 4;

			openFile.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_HIDEREADONLY;
			openFile.hInstance = (HMODULE)GetCurrentProcess();
			BOOL b = GetOpenFileNameW(&openFile);



			if (b) {
				wstrcpy(curFilePath, szFileName); //保存下来当前保存的文件的地址
				saveMark = true;
				hdc = GetDC(hWnd);
				openNewFile(szFileName, hdc);
				ReleaseDC(hWnd, hdc);
				isModified = false;
			}
			if (fTextSelected)fTextSelected = false;
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		case IDM_SAVE: //保存文件
		{

			if (!saveMark) {
				wchar_t  szFileName[MAX_PATH] = { 0 };
				OPENFILENAME openFile = { 0 };
				openFile.lStructSize = sizeof(OPENFILENAME);
				openFile.nMaxFile = MAX_PATH;

				openFile.lpstrFilter = L"MarkDown(*.md)\0*.md\0Js(*.js)\0*.js\0Bat(*.bat)\0*.bat\0Text(*.txt)\0*.txt\0C++(*.cpp)\0*.cpp\0C头文件(*.h)\0*.h\0All Files(*.*)\0*.*\0\0";
				openFile.lpstrDefExt = (LPCWSTR)L"txt";
				openFile.nFilterIndex = 4;

				openFile.lpstrFile = szFileName;
				openFile.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_HIDEREADONLY;
				openFile.hInstance = (HMODULE)GetCurrentProcess();
				BOOL b = GetSaveFileNameW(&openFile);
				//BOOL c = GetFileTitleW();

				if (b) { //如果成功获取了保存地址；
					wstrcpy(curFilePath, szFileName); //保存下来当前保存的文件的地址
					saveMark = true; //表示该文件已经存过，不需要再选择地址
					saveFile(szFileName);
					isModified = false;
				}
			}
			else {
				saveFile(curFilePath);
				isModified = false;
			}
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);

			break;
		}
		case IDM_SAVEAS: //另存为
		{
			wchar_t  szFileName[MAX_PATH] = { 0 };
			OPENFILENAME openFile = { 0 };
			openFile.lStructSize = sizeof(OPENFILENAME);
			openFile.nMaxFile = MAX_PATH;
			openFile.lpstrFile = szFileName;

			openFile.lpstrFilter = L"MarkDown(*.md)\0*.md\0Js(*.js)\0*.js\0Bat(*.bat)\0*.bat\0Text(*.txt)\0*.txt\0C++(*.cpp)\0*.cpp\0C头文件(*.h)\0*.h\0All Files(*.*)\0*.*\0\0";
			openFile.lpstrDefExt = (LPCWSTR)L"txt";
			openFile.nFilterIndex = 4;

			openFile.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_HIDEREADONLY;
			openFile.hInstance = (HMODULE)GetCurrentProcess();
			BOOL b = GetSaveFileNameW(&openFile);

			if (b) {
				wstrcpy(curFilePath, szFileName); //保存下来当前保存的文件的地址
				saveMark = true;
				saveFile(szFileName);
				
			}
			ShowCaret(hWnd);
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
		
			break;
		}
		case IDM_NEWFILE: //新建文件
		{
			if (isModified == true) {
				//弹出消息框
				int X = MessageBox(NULL, _T("是否保存文件？"), _T("提示消息"), MB_YESNOCANCEL | MB_ICONQUESTION);
				if (X == IDCANCEL) return 0;//如果取消则不新建
				if (X == IDYES) {
					SendMessage(hWnd, WM_COMMAND, IDM_SAVE, 1L);
				}
			}

			hdc = GetDC(hWnd);
			createNewFile(hdc);

			crt.CaretPosX = 0;
			crt.CaretPosY = 0;
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			saveMark  = false;
			isModified =false;
			fTextSelected = false;
			ReleaseDC(hWnd, hdc);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}

		/*复制*/
		case ID_COPY:
		{
			if (!OpenClipboard(hWnd))
				return FALSE;
			EmptyClipboard();

			/*(0,0,0,0)放入py,px,my,mx*/
			wchar_t * str = NULL;

			str = Ar.GetStr(selectEnd.second, selectEnd.first, selectBegin.second, selectBegin.first);

			HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE,
				(wcslen(str) + 1) * sizeof(wchar_t));

			if (hglb == NULL)
			{
				delete[] str;
				CloseClipboard();
				return FALSE;
			}

			wchar_t * temp = (wchar_t*)GlobalLock(hglb);
			wcsmove(temp, str);
			GlobalUnlock(hglb);

			SetClipboardData(CF_UNICODETEXT, hglb);

			delete[]str;
			break;
		}
		/*粘贴*/
		case ID_PASTE:
		{
			if (fTextSelected) {
				SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 1L);
				fTextSelected = FALSE;
				SendMessage(hWnd, WM_COMMAND, ID_PASTE, 1L);
				undo a = new Undo();
				Ar.UndoStack.push(a);
				return 0;
			}

			hdc = GetDC(hWnd);
			HideCaret(hWnd);
			if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
				break;
			if (OpenClipboard(hWnd)) {
				HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
				if (hglb != NULL) {
					wchar_t* temp = (wchar_t*)GlobalLock(hglb);
					if (temp == NULL) break;

					Ar.Insert(crt.CaretPosY, temp);

					crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);

					GlobalUnlock(hglb);

					Ar.Emptyredo();
				}
				CloseClipboard();
			}

			if (ScrCrtPosX >= nWindowX - 2 * nCharX || ScrCrtPosX <= 0)
				SetScrollRange(hWnd, SB_HORZ, 0, 6 + Ar.MaxWidth(hdc) / nCharX, 0);
			if (ScrCrtPosY * nCharY >= nWindowY - 10 * nCharY || ScrCrtPosY <= 0)
				SetScrollRange(hWnd, SB_VERT, 0, Ar.LineNum(), 0);

			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(hWnd, SB_VERT, &si);
			yScrPos = si.nPos;

			GetScrollInfo(hWnd, SB_HORZ, &si);
			xScrPos = si.nPos;

			InvalidateRect(hWnd, NULL, TRUE);

			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			ShowCaret(hWnd);

			ReleaseDC(hWnd, hdc);
			isModified = true;
			return 0;
		}
		/*撤销*/
		case ID_UNDO: {
			hdc = GetDC(hWnd);
			HideCaret(hWnd);
			if (Ar.UndoStack.empty())
				break;
			if (fTextSelected)fTextSelected = false;
			undo act = Ar.UndoStack.top();
			/*若插入则删除*/
			if (act->Op == Ins)
			{
				selectPos p = Ar.Delete(act->Begin.second, act->Begin.first, act->End.second, act->End.first, R);
				crt.CaretPosY = p.second;
				crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);
				delete act;
				Ar.UndoStack.pop();
			}
			/*若删除则插入*/
			else if (act->Op == Del) {
				Ar.Insert(act->Begin.second, act->Begin.first, act->str, R);
				crt.CaretPosY = act->Begin.second;
				crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);
				delete act;
				Ar.UndoStack.pop();
			}
			else {
				delete act;
				Ar.UndoStack.pop();
				SendMessage(hWnd, WM_COMMAND, ID_UNDO, 1L);
				SendMessage(hWnd, WM_COMMAND, ID_UNDO, 1L);
				undo a = new Undo();
				Ar.RedoStack.push(a);
			}
			tmpL = Ar.GetLine(crt.CaretPosY);
			ShowCaret(hWnd);
			
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);

			break;
		}

		/*恢复撤销*/
		/*只能在撤销后执行，若撤销后又发生改变，则 恢复撤销栈 清空*/

		case ID_REDO: {
			hdc = GetDC(hWnd);

			if (Ar.RedoStack.empty())
				break;
			if (fTextSelected)fTextSelected = false;

			undo act = Ar.RedoStack.top();

			/*若插入则删除*/
			if (act->Op == Ins)
			{
				selectPos p = Ar.Delete(act->Begin.second, act->Begin.first, act->End.second, act->End.first, U);
				crt.CaretPosY = p.second;
				crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);
				delete act;
				Ar.RedoStack.pop();
			}
			/*若删除则插入*/
			else if (act->Op == Del) {
				Ar.Insert(act->Begin.second, act->Begin.first, act->str, U);
				crt.CaretPosY = act->Begin.second;
				crt.CaretPosX = Ar.GetLine(crt.CaretPosY)->CharWidth(LF, hdc);
				delete act;
				Ar.RedoStack.pop();

			}
			else {
				delete act;
				Ar.RedoStack.pop();
				SendMessage(hWnd, WM_COMMAND, ID_REDO, 1L);
				SendMessage(hWnd, WM_COMMAND, ID_REDO, 1L);
				undo a = new Undo();
				Ar.UndoStack.push(a);
			}

			tmpL = Ar.GetLine(crt.CaretPosY);
			ShowCaret(hWnd);
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);

			break;

		}
		case ID_CHOME:
			hdc = GetDC(hWnd);
			HideCaret(hWnd);
			if (fTextSelected) {
				SendMessage(hWnd, WM_DEMASK, 0, 0);
				fTextSelected = false;

			}

			crt.MvCHome(tmpL, hdc);

			if (ScrCrtPosX > nWindowX || ScrCrtPosX <= 0)
				SetScrollRange(hWnd, SB_HORZ, 0, 2 + Ar.MaxWidth(hdc) / nCharX, 0);
			if (ScrCrtPosY * nCharY > nWindowY || ScrCrtPosY < 0)
				SetScrollRange(hWnd, SB_VERT, 0, Ar.LineNum(), 0);

			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(hWnd, SB_VERT, &si);
			yScrPos = si.nPos;

			GetScrollInfo(hWnd, SB_HORZ, &si);
			xScrPos = si.nPos;

			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			ShowCaret(hWnd);

			InvalidateRect(hWnd, NULL, TRUE);

			ReleaseDC(hWnd, hdc);

			break;

		case ID_CEND:
			//tmpL = Ar.GetLine(crt.CaretPosY);
			hdc = GetDC(hWnd);
			HideCaret(hWnd);
			if (fTextSelected) {
				SendMessage(hWnd, WM_DEMASK, 0, 0);
				fTextSelected = false;
			}

			crt.MvCEnd(tmpL, hdc);

			if (ScrCrtPosX > nWindowX || ScrCrtPosX <= 0)
				SetScrollRange(hWnd, SB_HORZ, 0, 2 + Ar.MaxWidth(hdc) / nCharX, 0);
			if (ScrCrtPosY * nCharY > nWindowY || ScrCrtPosY < 0)
				SetScrollRange(hWnd, SB_VERT, 0, Ar.LineNum(), 0);
			InvalidateRect(hWnd, NULL, TRUE);

			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(hWnd, SB_VERT, &si);
			yScrPos = si.nPos;

			GetScrollInfo(hWnd, SB_HORZ, &si);
			xScrPos = si.nPos;

			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);
			ShowCaret(hWnd);

			ReleaseDC(hWnd, hdc);
			break;

		case ID_TEST:

			MessageBox(hWnd, GetCommandLine(), L"提示", MB_OK);

			break;
		case IDM_DELETE: {

			SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 0);
			break;
		}

		case IDM_SELECTALL: {

			hdc = GetDC(hWnd);
			crt.MvCEnd(tmpL, hdc);
			selectBegin = { 0, 0 };
			selectEnd = { tmpL->len, Ar.LineNum() - 1 };
			fTextSelected = true;

			ShowCaret(hWnd);
			SetCaretPos(ScrCrtPosX, ScrCrtPosY * nCharY);

			SendMessage(hWnd, WM_MASK, 0, 1L);

			ReleaseDC(hWnd, hdc);

			break;
		}

		case IDM_CUT: {
			if (fTextSelected) {
				hdc = GetDC(hWnd);
				SendMessage(hWnd, WM_COMMAND, ID_COPY, 0);
				SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 1L);
				fTextSelected = FALSE;
				ReleaseDC(hWnd, hdc);
			}
			break;
		}

		case IDM_SEARCH: {

			/*初始化*/
			ZeroMemory(&fr, sizeof(fr));
			fr.lStructSize = sizeof(fr);
			fr.hwndOwner = hWnd;
			fr.lpstrFindWhat = szFindWhat;
			fr.wFindWhatLen = MAXL;
			fr.Flags = 0;

			hdlg = FindText(&fr);

			break;
		}

		case IDM_REPLACE: {
			ZeroMemory(&rp, sizeof(rp));
			rp.lStructSize = sizeof(rp);
			rp.hwndOwner = hWnd;
			rp.lpstrFindWhat = rpFindWhat;
			rp.lpstrReplaceWith = rpReplaceWith;
			rp.wFindWhatLen = MAXL;
			rp.wReplaceWithLen = MAXL;
			rp.Flags = 0;

			rdlg = ReplaceText(&rp);

			break;
		}
		case ID_STATISTICS: //Ctrl+M
		{

			LPTSTR sszBuffer = new TCHAR[1024];
			int li = Ar.LineNum();
			if (fTextSelected) {
				li=Ar.GetTotal(selectBegin, selectEnd);
			}
			else Ar.GetTotal();
			wsprintf(sszBuffer,L"行数:%d\n字符数（计空格）:%d\n字符数（不计空格）:%d\n中文字符数:%d", li,Ar.totalnum,Ar.totalnumwithoutspace,Ar.chinesenum);
			if (fTextSelected) MessageBox(hWnd, sszBuffer, L"选中统计", MB_OK);
			else MessageBox(hWnd, sszBuffer, L"全文统计", MB_OK);
		}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
	case WM_ERASEBKGND:
		return true;

	case WM_PAINT:
		paint(hWnd);
	break;

	case WM_CLOSE: {

		if (isModified == true) {
			//弹出消息框
			int X = MessageBox(NULL, _T("是否保存文件？"), _T("提示消息"), MB_YESNOCANCEL | MB_ICONQUESTION);
			if (X == IDYES) {
				SendMessage(hWnd, WM_COMMAND, IDM_SAVE, 1L);
				DestroyWindow(hWnd);
			}
			else if (X == IDNO) {
				DestroyWindow(hWnd);
			}
		}
		else {
			DestroyWindow(hWnd);
		}
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	return 0;
	}
	return 0;
}

// “关于”框的消息处理程序。
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

INT_PTR CALLBACK Welcome(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

void openNewFile(wchar_t * Address, HDC hdc) //打开新文件
{
	std::wifstream inFile;
	inFile.open(Address);
	if (!inFile.is_open()) return;
	else inFile.imbue(std::locale("chs"));

	wchar_t ch;

	if (!Ar.IsEmpty()) Ar.clearWord();
	line curL = Ar.GetLine(0); //当前行
	line nextL; //下一行

	while (inFile.get(ch)) {

		if (ch == L'\n') {
			nextL = curL->NewLine();//在当前行后面插入一行
			curL = nextL;
			Ar.IncLineN(); //行数加一;
		}
		//else if (ch == L'\n') continue;
		else if (ch == L'\t')
			for (int i = 0; i < 4; i++)
				curL->Push(L' ', LF);

		else curL->Push(ch, LF);
	}

	crt.CaretPosY = Ar.LineNum() - 1;
	crt.CaretPosX = curL->CharWidth(hdc);
	//crt.CtrCaretMv(Ar, , , hdc);
	//line lastL = Ar.GetLine(Ar.LineNum() - 1);
	//Ar.Remove(lastL); //移除文章的最后一行
	inFile.close();

	tmpL = curL;
}

void saveFile(wchar_t * Address) { //保存文件

	std::wofstream outFile(Address);
	if (!outFile) return;
	else outFile.imbue(std::locale("chs"));

	line curL = Ar.GetLine(0);

	wchar_t * str;
	while(!Ar.IsLastL(curL)){
		str = curL->GetStr();
		outFile << str << L"\n";
		curL = curL->next;
		delete[] str;
	}
	/*修正最后一行多一个回车*/
	str = curL->GetStr();
	outFile << str ;
	curL = curL->next;
	delete[] str;
}

void createNewFile(HDC hdc) { //新建文件

	if (!Ar.IsEmpty()){
		tmpL = Ar.GetLine(0);
		crt.MvHome(tmpL, hdc);
		SetCaretPos(0, 0);
		Ar.clearWord();
	}
}

void wstrcpy(wchar_t * desStr, const wchar_t * srcStr) {

	size_t len = wcslen(srcStr);
	int i;

	for (i = 0; i < len; i++) {
		desStr[i] = srcStr[i];
	}
	desStr[len] = L'\0';
}

void paint(HWND hWnd)
{
	HideCaret(hWnd);
	PAINTSTRUCT ps;
	hdc = BeginPaint(hWnd, &ps);

	Bkhdc = CreateCompatibleDC(hdc);
	HBITMAP memBmp = CreateCompatibleBitmap(Bkhdc, nWindowX, nWindowY);
	//hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BITMAP1));

	SelectObject(Bkhdc, memBmp);
	//SelectObject(Bkhdc, hBitmap);

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, SB_VERT, &si);
	yScrPos = si.nPos;

	GetScrollInfo(hWnd, SB_HORZ, &si);
	xScrPos = si.nPos;

	scrFirstLine = max(0, yScrPos + ps.rcPaint.top / nCharY);
	scrLastLine = min(Ar.LineNum() - 1, yScrPos + ps.rcPaint.bottom / nCharY);

	line ttmpL = Ar.GetLine(scrFirstLine);

	RECT fillRc;
	fillRc.left = 0;
	fillRc.top = 0;
	fillRc.bottom = nWindowY;
	fillRc.right = nWindowX;
	FillRect(Bkhdc, &fillRc, (HBRUSH)(COLOR_WINDOW + 1));
	x = nCharX * (-xScrPos);
	y = nCharY * (-yScrPos);

	for (int i = scrFirstLine; i <= scrLastLine; ttmpL = ttmpL->next, i++)
	{
		crPrevText = SetTextColor(Bkhdc, RGB(253, 185, 53));
		crPrevBk = SetBkColor(Bkhdc, RGB(255, 255, 255));
		std::wstring l_ws;
		l_ws = std::to_wstring(i+1);
		TextOut(Bkhdc, 0, (i)* nCharY + y, (LPCWSTR)l_ws.c_str(), (int)l_ws.size());
		SetTextColor(Bkhdc, crPrevText);
		SetBkColor(Bkhdc, crPrevBk);

		TextOut(Bkhdc, x + 4 * nCharX, (i)* nCharY + y, ttmpL->GetPos(LF),
			ttmpL->Getlen(LF));
		TextOut(Bkhdc, x + 4 * nCharX + ttmpL->CharWidth(LF, hdc),
			(i)* nCharY + y, ttmpL->GetPos(RG), ttmpL->Getlen(RG));
	}

	BitBlt(hdc, 0, 0, nWindowX, nWindowY, Bkhdc, 0, 0, SRCCOPY);
//	DeleteObject(hBitmap);
	DeleteObject(Bkhdc);
	DeleteObject(memBmp);
	EndPaint(hWnd, &ps);

	ReleaseDC(hWnd, hdc);
	ShowCaret(hWnd);
	return;
}