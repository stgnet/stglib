// STGLIB/StDevGdi.h
// Copyright 2005 by StG Net

#ifndef STGLIB_StDevGdi
#define STGLIB_StDevGdi

#pragma message("using stdevgdi.h")


#include "/src/stglib/stcore.h"

// use StBitmap and StRect originally for StWindow
#include "/src/stglib/stbitmap.h"
#include "/src/stglib/strect.h"

#include "/src/stglib/stbox.h"

// WIN32 implementation

#ifdef WIN32

#include "/src/stglib/stwin32.h"

LRESULT CALLBACK StDevGdiWndProc(HWND hWnd, UINT mesg, WPARAM wParam, LPARAM lParam);

WNDCLASS StDevGdi_WindowsClass;
#define StDevGdi_CLASS_NAME "StDevGdi"

class StDevGdi;
StDevGdi *_StDevGdi_MainWindow=NULL;


class StDevGdiElement
{
public:
	StRect location;

};


class StDevGdi:public StBox<StDevGdiElement>
{
	HWND hWnd;

public:
	StBitmap Image;
	StDevGdi(const char *title)
	{
		_StDevGdi_MainWindow=this;

		if (!StDevGdi_WindowsClass.lpszClassName)
		{
			StDevGdi_WindowsClass.style = 0; //0x00005000|WS_EX_TRANSPARENT; //CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS; //CS_SAVEBITS;
			StDevGdi_WindowsClass.lpszClassName=StDevGdi_CLASS_NAME;
			StDevGdi_WindowsClass.hInstance = 0;
			StDevGdi_WindowsClass.lpfnWndProc = StDevGdiWndProc;
			StDevGdi_WindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			StDevGdi_WindowsClass.hIcon = 0;
			StDevGdi_WindowsClass.lpszMenuName = 0;
			StDevGdi_WindowsClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			StDevGdi_WindowsClass.cbClsExtra = 0;
			StDevGdi_WindowsClass.cbWndExtra = 0;





			if (!RegisterClass(&StDevGdi_WindowsClass))
			{
				// how to handle this in constructor!?
				//error= GetLastError();
				return;
			}
		}

//		hWnd = CreateWindow(StDevGdi_CLASS_NAME, title, WS_OVERLAPPEDWINDOW,
//			CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
//			(HWND)NULL,(HMENU)NULL,0,NULL);

		hWnd=CreateWindowEx(
			0,				// extended window style
			StDevGdi_CLASS_NAME,	// registered class name
			title,				// window name
			WS_BORDER|WS_CAPTION|WS_SYSMENU,		//WS_OVERLAPPEDWINDOW,					// window style
			CW_USEDEFAULT,								// horizontal position of window
			CW_USEDEFAULT,								// vertical position of window
			CW_USEDEFAULT,					 // window width
			CW_USEDEFAULT,					// window height
			(HWND)NULL,					// handle to parent or owner window
			(HMENU)NULL,					// menu handle or child identifier
			0,					// handle to application instance
			NULL				// window-creation data
		);

		if (!hWnd)
		{
			MessageBox(NULL,"CreateWindow failed",title,MB_ICONERROR);
		}
		if (hWnd)
		{

			ShowWindow(hWnd, SW_SHOWNORMAL);
			UpdateWindow(hWnd);
		}
	}

	~StDevGdi()
	{
		if (hWnd) 
		{ 
			DestroyWindow(hWnd);
			Update();
			hWnd=NULL;
		}
	}

	int Update(void)
	{
		HACCEL hAccelTable=NULL;
		MSG msg;
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) 
		{
			if (msg.message == WM_QUIT)
				return(0);

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return(1);
	}

	void _Paint(void)
	{
		HDC hdc;
		PAINTSTRUCT ps;
		StRect rect;

				hdc = BeginPaint (hWnd, &ps);
				GetClientRect (hWnd, rect);

				SetBkMode(hdc, TRANSPARENT);

/*						DrawText (hdc, "Hello Windows!", -1, &rect,
						DT_SINGLELINE|DT_CENTER|DT_VCENTER);
				SetTextColor(hdc, RGB(255, 0, 0));
				TextOut(hdc, rect.right/2, 200, "Hello World", 11);
*/

		Image._BitmapPaintHdc(hdc,rect);
		
		EndPaint(hWnd,&ps);
	}

	virtual void _Press(int x,int y)=0;
	virtual void _Key(int ch)=0;

	void SetPixel(int x,int y)
	{
		StColor white(255,255,255);
		Image(x,y)=white;
		// invalidate image here to force repaint
//		StRect repaint;
//		repaint(x,y,x,y);
		InvalidateRect(hWnd,NULL,TRUE);
	}
};

LRESULT CALLBACK StDevGdiWndProc(HWND hWnd, UINT mesg, 
			 WPARAM wParam, LPARAM lParam)
{
	switch (mesg)
	{ 
		case WM_PAINT:
			_StDevGdi_MainWindow->_Paint();
			return(0);

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			_StDevGdi_MainWindow->_Press(LOWORD(lParam),HIWORD(lParam));
			return(0);

		case WM_DESTROY:
			PostQuitMessage(0);
			return(0);



		case WM_KEYDOWN:

//			if ((wParam==VK_SPACE)||(wParam==VK_RETURN)||(wParam>=0x2f ) &&(wParam<=0x100)) 

			BYTE ks[256];
			GetKeyboardState(ks);

            WORD w;
            UINT scan=0;
            ToAscii(wParam,scan,ks,&w,0);

			_StDevGdi_MainWindow->_Key(w);

			return(0);

	}

		return DefWindowProc (hWnd, mesg, wParam, lParam);
}

#endif



#endif
