// STGLIB/stgdi.h
// Copyright 2005 by StG Net

#ifndef STGLIB_STGDI
#define STGLIB_STGDI

#pragma message("using stgdi.h")
#ifdef STGLIB_STP_WIN32
#pragma comment(linker, "/defaultlib:user32.lib")
#pragma comment(linker, "/defaultlib:gdi32.lib")
#endif

#include "/src/stglib/stcore.h"

// use StBitmap and StRect originally for StWindow
#include "/src/stglib/stbitmap.h"
#include "/src/stglib/strect.h"

// WIN32 implementation

#ifdef WIN32







LRESULT CALLBACK StGdiWndProc(HWND hWnd, UINT mesg, WPARAM wParam, LPARAM lParam);

WNDCLASS StGdi_WindowsClass;
#define STGDI_CLASS_NAME "StGdi"

class StGdi;
StGdi *_StGdi_MainWindow=NULL;


class StGdi
{
	HWND hWnd;
	StBitmap Image;
	int Changed;

public:
	StGdi(const char *title)
	{
		_StGdi_MainWindow=this;
		Changed=1;

		if (!StGdi_WindowsClass.lpszClassName)
		{
			StGdi_WindowsClass.lpszClassName=STGDI_CLASS_NAME;
			StGdi_WindowsClass.hInstance = 0;
			StGdi_WindowsClass.lpfnWndProc = StGdiWndProc;
			StGdi_WindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			StGdi_WindowsClass.hIcon = 0;
			StGdi_WindowsClass.lpszMenuName = 0;
			StGdi_WindowsClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			StGdi_WindowsClass.style = CS_SAVEBITS;
			StGdi_WindowsClass.cbClsExtra = 0;
			StGdi_WindowsClass.cbWndExtra = 0;

			if (!RegisterClass(&StGdi_WindowsClass))
			{
				// how to handle this in constructor!?
				//error= GetLastError();
				return;
			}
		}

		hWnd = CreateWindow(STGDI_CLASS_NAME, title, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
			(HWND)NULL,(HMENU)NULL,0,NULL);

		if (hWnd)
		{

			ShowWindow(hWnd, SW_SHOWNORMAL);
			UpdateWindow(hWnd);
		}
	}

	~StGdi()
	{
		if (hWnd) 
		{ 
			DestroyWindow(hWnd);
			hWnd=NULL;
		}
	}

	void Update(void)
	{
		if (Changed)
		{
			InvalidateRect(hWnd,NULL,TRUE);
			Changed=0;
		}
		HACCEL hAccelTable=NULL;
		MSG msg;
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) 
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	void _Paint(void)
	{
	    HDC         hdc;
		PAINTSTRUCT ps;
		StRect rect;

        hdc = BeginPaint (hWnd, &ps);
        GetClientRect (hWnd, rect);

        SetBkMode(hdc, TRANSPARENT);
/*            DrawText (hdc, "Hello Windows!", -1, &rect,
            DT_SINGLELINE|DT_CENTER|DT_VCENTER);
        SetTextColor(hdc, RGB(255, 0, 0));
        TextOut(hdc, rect.right/2, 200, "Hello World", 11);
*/

		Image._BitmapPaintHdc(hdc,rect);
		
		EndPaint(hWnd,&ps);
	}

	void SetPixel(int x,int y)
	{
		StColor white(255,255,255);
		Image(x,y)=white;
		Changed=1;
	}

	inline StPixel& operator()(long x,long y)
	{
		Changed=1;
		return Image(x,y);
	}


};

LRESULT CALLBACK StGdiWndProc(HWND hWnd, UINT mesg, 
			 WPARAM wParam, LPARAM lParam)
{
    switch (mesg)
	{ 
    case WM_PAINT:
		_StGdi_MainWindow->_Paint();
        return 0;
	}

    return DefWindowProc (hWnd, mesg, wParam, lParam);
}

#endif



#endif
