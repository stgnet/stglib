
#ifndef STGLIB_STWINDOW
#define STGLIB_STWINDOW

#pragma message("using stwindow.h")

include "/stglib/stwin32.h"

#include "/stglib/stcore.h"
#include "/stglib/strect.h"
#include "/stglib/stbox.h"

class StWindow;
class StWindowObject;
class StWindowView;

class StWindow *_MainWindow=0;

//////////////////////////////////////////////////////////////////////
// StNotifyRect - overload of StRect to add change notify function 
//////////////////////////////////////////////////////////////////////
class StNotifyRect:public StRect
{
	friend class StWindowObject;
	StWindowObject *_WindowObject;
public:
	StNotifyRect()
	{
		_WindowObject=0;
	}

	// map assignment to lower class to prevent bug
	StNotifyRect& operator=(StRect &copyfrom)
	{
		StRect::operator=(copyfrom);
		return(*this);
	}
	StNotifyRect& operator=(StNotifyRect &copyfrom)
	{
		StRect::operator=(copyfrom);
		return(*this);
	}

	inline StRect& operator=(struct tagRECT &rect)
	{
		X1=rect.left;
		Y1=rect.top;
		X2=rect.right;
		Y2=rect.bottom;
		NotifyChange();
		return(*this);
	}

	inline virtual void NotifyChange(void);
};

//////////////////////////////////////////////////////////////////////
// StWindowCommand (base class for menus, buttons, etc) 
//////////////////////////////////////////////////////////////////////

#define StWindowCommandOffset 100

class StWindowCommand;

StBox<StWindowCommand> _StWindow_Commands;

class StWindowCommand
{
protected:
	StSize _index;

	// base class provides index box to command function
public:
	StWindowCommand()
	{
		// add to command list box and get index value
		_index=_StWindow_Commands._BoxAdd(this)+StWindowCommandOffset;
	}

	virtual ~StWindowCommand()
	{
		_StWindow_Commands-=this;
	}

	virtual void _WindowCommand(StWindow *window)
	{
		// some menus do not have a command window
		window;
	}
};

//////////////////////////////////////////////////////////////////////
//  StMenu 
//////////////////////////////////////////////////////////////////////


class StMenu:StWindowCommand
{
	StMenu *_Parent;
	StString _Name;
	MENUITEMINFO _menuinfo;
	HMENU _hMenu;

	friend class StWindow;

	void _InitInfo(void)
	{
		memset(&_menuinfo,0,sizeof(_menuinfo));
		_menuinfo.cbSize=sizeof(_menuinfo);

		_menuinfo.fMask=MIIM_ID|MIIM_SUBMENU|MIIM_TYPE;

		_menuinfo.wID=_index;

		_menuinfo.fType=MFT_STRING;
		_menuinfo.dwTypeData=(char*)((const char*)_Name);
		_menuinfo.cch=~_Name;

	}
	void _UpdateInfo(void)
	{
		SetMenuItemInfo(_Parent->_hMenu,_index,FALSE,&_menuinfo);
	}

	// allow child to use menu handle, but make sure we are one...
	HMENU _MenuGetHandle(void)
	{
		if (_hMenu)
			return(_hMenu);

		// create menu and attach it as a submenu
		_hMenu=CreateMenu();
		if (_Parent)
		{
			_menuinfo.hSubMenu=_hMenu;
			_UpdateInfo();
		}
		return(_hMenu);
	}

public:
	// !!! used only by StWindow to declare root menu placeholder
	StMenu()
	{
		_Parent=NULL;
		_hMenu=NULL;
	}
	// used to create a new un-attached menu
	StMenu(StChar *item_str)
	{
		_Parent=NULL;
		_Name<<item_str;
		_InitInfo();

		_hMenu=CreateMenu();
	}

	// used to create item in menu
	StMenu(StWindow &Parent,StChar *item_str);

	// used to create item in menu
	StMenu(StMenu &Parent,StChar *item_str)
	{
		_Parent=&Parent;
		_Name<<item_str;
		_InitInfo();

		_hMenu=NULL;

		// parent needs to have a menu handle...
		// then add this item to the menu
		InsertMenuItem(_Parent->_MenuGetHandle(),999,TRUE,&_menuinfo);
	}
	virtual ~StMenu()
	{
		if (_hMenu)
			DestroyMenu(_hMenu);
	}
};

//////////////////////////////////////////////////////////////////////
// StMouse class to convey movement data
//////////////////////////////////////////////////////////////////////
enum StMouseAction
{
	Move,
	Down,
	Up,
	Click,
	DoubleClick,
};

class StMouse
{
public:
	StPoint Position;
	StMouseAction Action;
	StByte Left;
	StByte Right;

	// convenience variables for moving objects
	// !!! this presumes ONE instance of StMouse
	StWindowObject *SelectedObject;
	StPoint SelectedPosition;
	StByte SelectedEdge;	// for resizing

	void operator!(void)
	{
		Left=Right=0;
		Action=Move;
		// don't reset SelectedObject here!
	}

	StMouse()
	{
		operator!();
		SelectedObject=NULL;
		SelectedEdge=0;

	}

};

//////////////////////////////////////////////////////////////////////
// StCursor class to hold cursor definitions
//////////////////////////////////////////////////////////////////////
class StCursor
{
	friend class StWindow;
	friend class StWindowObject;
	HCURSOR hCursor;
public:
	StCursor(LPCTSTR windows_idc_code)
	{
		hCursor=LoadCursor(NULL,windows_idc_code);
	}
	// expand this later with another constructor to allow
	// creation of custom cursors
};	

// create some global cursors
StCursor _WindowCursorArrow(IDC_ARROW);
StCursor _WindowCursorMove(IDC_SIZEALL);
StCursor _WindowCursorNWSE(IDC_SIZENWSE);
StCursor _WindowCursorNESW(IDC_SIZENESW);
StCursor _WindowCursorVert(IDC_SIZENS);
StCursor _WindowCursorHorz(IDC_SIZEWE);
StCursor _WindowCursorNo(IDC_NO);

// and a table to map edge selection to correct cursor
StCursor *_WindowCursorForEdge[16]=
{
	&_WindowCursorArrow,	// 0000 no edge
	&_WindowCursorHorz,	// 0001 left edge
	&_WindowCursorVert,	// 0010 top edge
	&_WindowCursorNWSE,	// 0011 top + left
	&_WindowCursorHorz,	// 0100 right
	&_WindowCursorNo,	// 0101 right + left (no can do)
	&_WindowCursorNESW,	// 0110 right + top
	&_WindowCursorNo,	// 0111 right + top + left (no)
	&_WindowCursorVert,	// 1000 bottom
	&_WindowCursorNESW,	// 1001 bottom + left
	&_WindowCursorNo,	// 1010 bottom + top (no)
	&_WindowCursorNo,	// 1011 bottom + top + left (no)
	&_WindowCursorNWSE,	// 1100 bottom + right
	&_WindowCursorNo,	// 1101 no
	&_WindowCursorNo,	// 1110 no
	&_WindowCursorNo,	// 1111 and no
};

//////////////////////////////////////////////////////////////////////
// WindowObject base class
//////////////////////////////////////////////////////////////////////


class StWindowObject:public StBox<StWindowObject>
{
	friend class StWindow;
	friend class StWindowView;

	// ptr to main window - set when object joins window
	StWindow *_Window;
	StWindowObject *_Parent;

public:
	StNotifyRect _WindowObjectView;		// size of object in view (normally entire object)
	StNotifyRect _WindowObjectDisp;		// where on parent window this object is displayed
	StRect _WindowObjectTarget;		// target display point in main window coordinates

	StByte _WindowObjectMoveable;
	StByte _WindowObjectSizeable;
	StByte _WindowObjectDropable;

	void _Reset(void)
	{
		_Window=NULL;
		_Parent=NULL;

		_WindowObjectMoveable=0;
		_WindowObjectSizeable=0;
		_WindowObjectDropable=0;

		_WindowObjectView._WindowObject=this;
		_WindowObjectDisp._WindowObject=this;
	}
	StWindowObject()
	{
		_Reset();
	}
	StWindowObject(StWindowObject& parent)
	{
		_Reset();
		parent+=this;
	}
	virtual ~StWindowObject()
	{
		if (_Parent)
			(*_Parent)-=this;
	}

	inline virtual void _BoxNotifyParentOfChild(StWindowObject *child)
	{
		// copy the ptr to the main window to our new child
		child->_Window=_Window;
		child->_Parent=this;

		// redraw this object, since something got added to us...
		_WindowObjectUpdate();
	}

	// specify how to compare two objects for sorting smaller objects to the top
	inline bool operator< (StWindowObject& rh)
	{
		return(_WindowObjectDisp.Size<rh._WindowObjectDisp.Size);
	}

	virtual void _WindowCursor(StCursor &cursor)
	{
		// oddly enough, this call doesn't require a _hWnd...
		SetCursor(cursor.hCursor);
	}

	virtual void _WindowObjectUpdate(void);

	void _WindowObjectUpdateTarget(void)
	{
		// update the target coordinates
		_WindowObjectTarget=_WindowObjectDisp;
		if (_Parent)
			_WindowObjectTarget.Transform(_Parent->_WindowObjectView,_Parent->_WindowObjectTarget); //_WindowObjectDisp);
	}


	virtual void _WindowObjectParentRectChanged(void)
	{
	}

	virtual void _WindowObjectRectChanged(StNotifyRect *which)
	{
		if (_BoxCount())
		{
			StBoxRef<StWindowObject> obj(*this);
			while (++obj)
				obj->_WindowObjectParentRectChanged();
		}
	}

	virtual void _WindowObjectDropped(StWindowObject *dropped)
	{
	}
	virtual void _WindowObjectDoubleClicked(void)
	{
	}
	virtual void _WindowObjectMouseDown(void)
	{
	}


	virtual void _WindowObjectDropCheck(StMouse &mouse)
	{
		StWindowObject *found=0;
		StBoxRef<StWindowObject> obj(*this);
		while (++obj)
		{
			if (obj==mouse.SelectedObject)
				continue;

			if (obj->_WindowObjectDisp.Hit(mouse.Position))
				found=(StWindowObject*)obj;
		}
		if (found)
		{
			found->_WindowObjectDropCheck(mouse);
			return;
		}

		if (_WindowObjectDropable)
			_WindowObjectDropped(mouse.SelectedObject);
	}



	virtual void _WindowObjectMouse(StMouse &mouse)
	{
		if (_BoxCount())
		{
			// try to pass the mouse down to an object
			StWindowObject *found=NULL;
			{
				StBoxRef<StWindowObject> obj(*this);
				while (++obj)
				{
					if (obj->_Parent!=this)
					{
						char *blow=0;
						*blow=0;
					}
					if (obj->_WindowObjectDisp.Hit(mouse.Position))
						found=(StWindowObject*)obj;
				}
			}
			// use last found, rather than first found (will be on 'top' of display)
			if (found)
			{
				found->_WindowObjectMouse(mouse);
				return;
			}

		}
		switch (mouse.Action)
		{

		case DoubleClick:
			_WindowObjectDoubleClicked();
			break;

		case Down:
			if (_WindowObjectSizeable)
			{
				// check if mouse point on/adjacent to window edge
				mouse.SelectedEdge=_WindowObjectDisp.Edge(mouse.Position);
				if (mouse.SelectedEdge)
				{
					// on the edge - mark selected object
					mouse.SelectedObject=this;
					mouse.SelectedPosition=mouse.Position;
					break;
				}
			}				
			if (_WindowObjectMoveable)
			{
				mouse.SelectedObject=this;
				mouse.SelectedPosition=mouse.Position;
				break;
			}
			_WindowObjectMouseDown();
//			MessageBeep(MB_ICONASTERISK);
			break;

		case Move:
			if (mouse.SelectedObject)
			{
				if (mouse.SelectedEdge)
				{
					// user is dragging an edge - resize
					mouse.SelectedObject->_WindowObjectUpdate();
					mouse.SelectedObject->_WindowObjectDisp.AddEdge(mouse.SelectedEdge,
						mouse.Position-mouse.SelectedPosition);
					mouse.SelectedPosition=mouse.Position;
					mouse.SelectedObject->_WindowObjectUpdate();
					break;
				}

				// we're about to move this object, so ask for it to be udpated...
				mouse.SelectedObject->_WindowObjectUpdate();
				// then move it
				mouse.SelectedObject->_WindowObjectDisp+=mouse.Position-mouse.SelectedPosition;
				mouse.SelectedPosition=mouse.Position;
				// and ask for the new position to be updated again...
				mouse.SelectedObject->_WindowObjectUpdate();
				break;
			}

			// we're just moving the mouse - nothing being dragged
			if (_WindowObjectSizeable)
			{
				mouse.SelectedEdge=_WindowObjectDisp.Edge(mouse.Position);
				if (mouse.SelectedEdge)
				{
					_WindowCursor(*_WindowCursorForEdge[mouse.SelectedEdge]);
					break;
				}
			}

			if (_WindowObjectMoveable)
			{
				_WindowCursor(_WindowCursorMove);
				break;
			}
			_WindowCursor(_WindowCursorArrow);
			break;

		case Up:
			if (mouse.SelectedObject)
			{
				if (mouse.SelectedEdge)
				{
					// user is dragging an edge - resize
					mouse.SelectedObject->_WindowObjectUpdate();
					mouse.SelectedObject->_WindowObjectDisp.AddEdge(mouse.SelectedEdge,
						mouse.Position-mouse.SelectedPosition);
					mouse.SelectedObject->_WindowObjectUpdate();
					mouse.SelectedObject=0;
					break;
				}
				mouse.SelectedObject->_WindowObjectUpdate();
				mouse.SelectedObject->_WindowObjectDisp+=mouse.Position-mouse.SelectedPosition;
				mouse.SelectedObject->_WindowObjectUpdate();

				((StWindowObject*)_Window)->_WindowObjectDropCheck(mouse);

				mouse.SelectedObject=0;
			}
			break;
		}
	}

	virtual void _WindowObjectPaint(StWindow &window)=0;
	virtual void _WindowObjectPrint(StWindow &window)
	{
		// use the same method as painting on the display unless redefined
		_WindowObjectPaint(window);
	}
};

inline void StNotifyRect::NotifyChange(void)
{
	if (_WindowObject)
		_WindowObject->_WindowObjectRectChanged(this);
}

//////////////////////////////////////////////////////////////////////
// Window base class
//////////////////////////////////////////////////////////////////////

class StWindow:public StWindowObject
{
	friend int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int);
	friend LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
	friend class StMenu;
	friend class StRect;
	friend class StBitmap;

public:
	PAINTSTRUCT _WindowPaintStruct;
	PRINTDLG _WindowsPrintDialog;
	StRect _WindowPaintRect;


	// windows specific:
	HWND _hWnd;

	StPoint _DPI;

protected:
	StString _WindowTitle;
	StMenu _MenuBar;

	// called after window is created to allow 
	// further initialization before display
	void _WindowCreated(void)
	{
		if (_MenuBar._hMenu)
		   SetMenu(_hWnd,_MenuBar._hMenu);

	}

public:
	StWindow(StChar *title=0)
	{
		if (title)
			_WindowTitle<<title;

		// the first window defined is the main window
		if (!_MainWindow)
			_MainWindow=this;


		memset(&_WindowsPrintDialog,0,sizeof(PRINTDLG));
		_WindowsPrintDialog.lStructSize=sizeof(PRINTDLG);
	}

	virtual ~StWindow()
	{
	}

	inline virtual void _BoxNotifyParentOfChild(StWindowObject *child)
	{
		// copy the ptr to the main window to our new child
		child->_Window=this;
		child->_Parent=this;
	}

	// dummy _WindowPaint(win) function to satisfy WindowObject's pure virtual
	virtual void _WindowObjectPaint(StWindow &win)
	{
		// make sure this isn't run accidentally
		StByte *blow=0;
		*blow=0;
	}

	virtual void _WindowPrint(void)
	{
		// load default printer (changes will be retained)
		if (!_WindowsPrintDialog.hDevNames &&
			!_WindowsPrintDialog.hDevMode)
		{
			// initialize defaults
			memset(&_WindowsPrintDialog,0,sizeof(PRINTDLG));
			_WindowsPrintDialog.lStructSize=sizeof(PRINTDLG);
			_WindowsPrintDialog.Flags=PD_RETURNDEFAULT;
			PrintDlg(&_WindowsPrintDialog);
			_WindowsPrintDialog.Flags&=~PD_RETURNDEFAULT;
		}

		// allow user to change printer
		_WindowsPrintDialog.Flags|=PD_RETURNDC|PD_NOPAGENUMS;
		_WindowsPrintDialog.hwndOwner=_hWnd;
		if (!PrintDlg(&_WindowsPrintDialog))
		{
			DWORD err=CommDlgExtendedError();
			if (err)
			{
				StString msg;
				msg<<"PrintDlg failed: "<<(int)err;
				MessageBox(_hWnd,msg,"Print",MB_OK);
			}
			return;
		}

		// copy the dc to the paint struct?
		// Is this a good idea???
		_WindowPaintStruct.hdc=_WindowsPrintDialog.hDC;

		// set the DPI structure
		_DPI(GetDeviceCaps(_WindowPaintStruct.hdc,LOGPIXELSX),GetDeviceCaps(_WindowPaintStruct.hdc,LOGPIXELSY));

		// tell printer we're starting to print
		DOCINFO docinfo;
		docinfo.cbSize=sizeof(docinfo);
		docinfo.lpszDocName="HMPCFG Phone Label";
		docinfo.lpszOutput=NULL;
		StartDoc(_WindowsPrintDialog.hDC,&docinfo);

		// print everything
		StBoxRef<StWindowObject> obj(*this);
		while (++obj)
		{
			obj->_WindowObjectUpdateTarget();
			obj->_WindowObjectPrint(*this);
		}

		// tell printer we're done
		EndDoc(_WindowsPrintDialog.hDC);
	}
	virtual void _WindowPaint(void)
	{
		BeginPaint(_hWnd,&_WindowPaintStruct);
		_WindowPaintRect=_WindowPaintStruct.rcPaint;

		StBoxRef<StWindowObject> obj(*this);
		while (++obj)
		{
			obj->_WindowObjectUpdateTarget();
			obj->_WindowObjectPaint(*this);
		}
		EndPaint(_hWnd,&_WindowPaintStruct);
	}

	virtual void _WindowUpdate(StRect &Repaint)
	{
		// repaint just a portion of the window...
		InvalidateRect(_hWnd,Repaint,TRUE);
	}

	virtual void _WindowUpdate()
	{
		// assume repaint of entire client area
		InvalidateRect(_hWnd,_WindowObjectView,TRUE);
	}

	virtual void _WindowSizeOrPositionChanged(void)
	{
		RECT rcRect;

		// update the 'client' window size
		GetClientRect(_hWnd,&rcRect);
		_WindowObjectView=rcRect;

		// the target coordinates match the client area
		_WindowObjectTarget=_WindowObjectView;

		// update the window position & size
//		GetWindowRect(_hWnd,&rcRect);
//		_WindowObjectTarget=rcRect;
 
		ClientToScreen(_hWnd,(POINT*)&(rcRect.left));
		ClientToScreen(_hWnd,(POINT*)&(rcRect.right));
		_WindowObjectDisp=rcRect;

 
	}

	virtual void _WindowClose(void)
	{
		PostQuitMessage(0);
	}
};

void StWindowObject::_WindowObjectUpdate(void)
{
	if (_Window)
		_Window->_WindowUpdate(_WindowObjectDisp);
}

//////////////////////////////////////////////////////////////////////
// StWindowView - view/zoom
//////////////////////////////////////////////////////////////////////
class StWindowView:public StWindowObject
{
	StZoom factor;
public:
	StWindowView(StWindowObject &Parent)
	{
		// add ourselves to the parent window
		Parent+=this;
	}
	virtual void _WindowObjectPaint(StWindow &window)
	{
		StBoxRef<StWindowObject> obj(*this);
		while (++obj)
		{
			obj->_WindowObjectUpdateTarget();
			obj->_WindowObjectPaint(window);
		}
	}
	virtual void _WindowObjectParentRectChanged(void)
	{
		if (!_Parent)	// crash protection
			return;


/*		if (Zoom)
		{
			_WindowObjectView(350,46,660,374);
			_WindowObjectDisp=_Parent->_WindowObjectView;
			return;
		}
*/		// update this 'sub' window's rects
		_WindowObjectView=_Parent->_WindowObjectView;
		_WindowObjectDisp=_WindowObjectView;

	}
};

//////////////////////////////////////////////////////////////////////
// StWindowClipboard - implementation of clipboard as base io
//////////////////////////////////////////////////////////////////////
enum StWindowClipboardFormat
{
	StCbFormat_Text=CF_TEXT,
	StCbFormat_Bitmap=CF_DIB,
};

class StWindowClipboard:public StFileMap
{
	HANDLE hData;
public:
	StWindowClipboard(StWindowClipboardFormat format):StFileMap(0,0)
	{
	    if (!OpenClipboard(NULL))
		{
			_Err(StErr_Unknown,"StWindowClipboard","OpenClipboard failed");
		    return; 
		}
	    if (!IsClipboardFormatAvailable(format))
		{
			_Err(StErr_FileNotFound,"StWindowClipboard","Format not available");
			return; 
		}
		hData=GetClipboardData(format);
		if (!hData)
		{
			_Err(StErr_Unknown,"StWindowClipboard","GetClipboardData failed");
			return;
		}
		_Data=(StByte*)GlobalLock(hData);
		if (!_Data)
		{
			_Err(StErr_Unknown,"StWindowClipboard","GlobalLock failed");
			return;
		}
		_Size=HeapSize(hData,0,_Data);
		if (_Size==(-1))
		{
			_Err(StErr_Unknown,"StWindowClipboard","HeapSize failed");
			_Data=0;
			_Size=0;
			return;
		}
	}
	virtual ~StWindowClipboard()
	{
		GlobalUnlock(hData);
	    CloseClipboard(); 
	}
};


//////////////////////////////////////////////////////////////////////
// StMenu constructor linked to window
//////////////////////////////////////////////////////////////////////

StMenu::StMenu(StWindow &window,StChar *item_str)
{
	_Parent=&(window._MenuBar);
	_Name<<item_str;
	_InitInfo();

	_hMenu=NULL;

	// parent needs to have a menu handle...
	// then add this item to the menu
	InsertMenuItem(window._MenuBar._MenuGetHandle(),999,TRUE,&_menuinfo);
}

//////////////////////////////////////////////////////////////////////
// MS-Windows WndProc 
//////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	static StPoint position;
	static StMouse mouse;
	static RECT rect;

	switch (message) 
	{
	case WM_COMMAND:
		{
			int wMsgId=LOWORD(wParam);
			StWindowCommand *wc=_StWindow_Commands[wMsgId-StWindowCommandOffset];
			if (!wc)
				   return DefWindowProc(hWnd, message, wParam, lParam);

			wc->_WindowCommand(_MainWindow);
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_SIZE:
	case WM_MOVE:
		_MainWindow->_WindowSizeOrPositionChanged();
		break;

// enable this when we know how to erase the non-painted portion of the client rect
//	case WM_ERASEBKGND:
//        return ((LRESULT)1); // Say we handled it.

	case WM_PAINT:
		_MainWindow->_WindowPaint();
		break;

	case WM_LBUTTONDBLCLK:
		!mouse;
		mouse.Position(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		mouse.Left=1;
		mouse.Action=DoubleClick;
		_MainWindow->_WindowObjectMouse(mouse);
		break;

	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		ClipCursor(_MainWindow->_WindowObjectDisp);

		!mouse;
		mouse.Position(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		mouse.Left=1;
		mouse.Action=Down;
		_MainWindow->_WindowObjectMouse(mouse);
		break;

	case WM_MOUSEMOVE: 
		!mouse;
		mouse.Position(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		mouse.Action=Move;
		if (wParam & MK_LBUTTON)
			mouse.Left=1;

		_MainWindow->_WindowObjectMouse(mouse);
		break;

	case WM_LBUTTONUP:
		ClipCursor((LPRECT) NULL); 
		ReleaseCapture(); 

		!mouse;
		mouse.Position(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)); 
		mouse.Left=1;
		mouse.Action=Up;
		_MainWindow->_WindowObjectMouse(mouse); 
		break; 


	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


//////////////////////////////////////////////////////////////////////
//  MS-Windows WinMain
//////////////////////////////////////////////////////////////////////

TCHAR *_StWindow_Class="StWindowClass";
HINSTANCE _StWindow_Instance;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable=NULL;

	_StWindow_Instance=hInstance;

	// register a class defining the window
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;

#ifdef STWINDOW_ICON
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)STWINDOW_ICON);
#else
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
#endif
	wcex.hCursor		= 0; //LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; //(LPCSTR)IDC_HMPCFG;
	wcex.lpszClassName	= _StWindow_Class;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
	RegisterClassEx(&wcex);

	// create the window
	_MainWindow->_hWnd=CreateWindow(_StWindow_Class,
		(const char*)(_MainWindow->_WindowTitle),
		WS_OVERLAPPEDWINDOW ,//| WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		NULL,
		NULL,
		hInstance,
		NULL);

   if (!_MainWindow->_hWnd)
      return FALSE;

   // allow window class to configure things between creation and display
   _MainWindow->_WindowCreated();

   ShowWindow(_MainWindow->_hWnd, SW_MAXIMIZE); //nCmdShow);
   UpdateWindow(_MainWindow->_hWnd);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

#include "/stglib/stpostfx.h"

#endif



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


#ifdef SAMPLE_BOGUS
HELLO.C


/*++

Copyright (c) 1998  Microsoft Corporation
All rights reserved.

Module Name:Hello

--*/

/******************************************************************
This is a simple Hello program with an application-defined function,
CreateAToolBar2, that creates a toolbar. CreateAToolBar2 uses 
CreateToolbarEx to create a toolbar with seven buttons and a separator.
CreateAToolbar2 adds three standard file bitmaps and four view bitmaps.
The identifiers for the buttons are defined in hello.h. By changing the
parameters passed to CreateToolbarEx you can created different types of toolbar. At the end of this file some examples are given.  
*******************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "hello.h"
 
HWND CreateAToolBar2 (HWND);
long PASCAL WndProc (HWND, UINT, WPARAM, LPARAM);

HINSTANCE hInst;


int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdParam, int nCmdShow)
{
    static char         szAppName[ ] = "HelloWin";
    HWND                hwnd;
    MSG                 msg;
    WNDCLASS            wndclass;

    if (!hPrevInstance)
    {
        wndclass.style          = CS_HREDRAW|CS_VREDRAW;
        wndclass.lpfnWndProc    = WndProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = 0;
        wndclass.hInstance      = hInstance;
        wndclass.hIcon          = LoadIcon (NULL, IDI_APPLICATION);
        wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground  = GetStockObject (WHITE_BRUSH);
        wndclass.lpszMenuName   = NULL;
        wndclass.lpszClassName  = szAppName;

        RegisterClass (&wndclass);
    }


    hwnd = CreateWindow(szAppName,
                        "The Hello Program",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    ShowWindow (hwnd, nCmdShow);
    UpdateWindow (hwnd);
    hInst = hInstance;

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    return msg.wParam;
}

long APIENTRY WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
    {
        HDC         hdc;
        PAINTSTRUCT ps;
        RECT        rect;
        

        switch (message)
        {
        case WM_CREATE:
            CreateAToolBar2(hwnd);
            return 0;
        case WM_PAINT:
            hdc = BeginPaint (hwnd, &ps);
            GetClientRect (hwnd, &rect);
            SetBkMode(hdc, TRANSPARENT);
            DrawText (hdc, "Hello Windows!", -1, &rect,
                DT_SINGLELINE|DT_CENTER|DT_VCENTER);
            SetTextColor(hdc, RGB(255, 0, 0));
            TextOut(hdc, rect.right/2, 200, "Hello World", 11);
            return 0;
        
        case WM_NOTIFY:
            return 0;
        
        case WM_DESTROY:
            PostQuitMessage (0);
            return 0;
        }

    return DefWindowProc (hwnd, message, wParam, lParam);
    }

HWND CreateAToolBar2(HWND hWndParent)
{
HWND hWndToolbar;
TBADDBITMAP tb;
int index, stdidx;
INITCOMMONCONTROLSEX icex;

// Toolbar buttons
TBBUTTON tbButtons [ ] = 
{
{STD_FILENEW, IDM_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
{STD_FILEOPEN, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
{STD_FILESAVE, IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
{VIEW_LARGEICONS, IDM_LARGEICON, TBSTATE_ENABLED, TBSTYLE_BUTTON, 
0L, 0},
{VIEW_SMALLICONS, IDM_SMALLICON, TBSTATE_ENABLED, TBSTYLE_BUTTON, 
0L, 0},
{VIEW_LIST, IDM_LISTVIEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 
0L, 0},
{VIEW_DETAILS, IDM_REPORTVIEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 
0L, 0},
}; 

// Ensure that the common control DLL is loaded. 
icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
icex.dwICC  = ICC_BAR_CLASSES;
InitCommonControlsEx(&icex);

// Create the toolbar and add the first three buttons and a separator.
hWndToolbar = CreateToolbarEx (hWndParent, 
WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS, 
ID_TOOLBAR, 11, (HINSTANCE)HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 
(LPCTBBUTTON)&tbButtons, 4, 0, 0, 100, 30, sizeof (TBBUTTON));

// Add the next four buttons
tb.hInst = HINST_COMMCTRL;
tb.nID = IDB_VIEW_SMALL_COLOR;
stdidx = SendMessage (hWndToolbar, TB_ADDBITMAP, 12, (LPARAM)&tb);

for (index = 4; index < NUM_BUTTONS; index++)
tbButtons[index].iBitmap += stdidx;

SendMessage (hWndToolbar, TB_ADDBUTTONS, 4, (LONG) &tbButtons[4]);
                     
return hWndToolbar;

} 

/****************************************************************
Flat Toolbar - To create a flat toolbar add TBSTYLE_FLAT.
 
hWndToolbar = CreateToolbarEx (hWndParent, 
WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 
ID_TOOLBAR, 11, (HINSTANCE)HINST_COMMCTRL, IDB_STD_SMALL_COLOR, (LPCTBBUTTON)&tbButtons, 4, 0, 0, 100, 30, sizeof (TBBUTTON));
****************************************************************/
/****************************************************************
Transparent Toolbar - To create a transparent toolbar add 
TBSTYLE_TRANSPARENT.
 
hWndToolbar = CreateToolbarEx (hWndParent, 
WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 
ID_TOOLBAR, 11, (HINSTANCE)HINST_COMMCTRL, IDB_STD_SMALL_COLOR, (LPCTBBUTTON)&tbButtons, 4, 0, 0, 100, 30, sizeof (TBBUTTON));
****************************************************************/
/*
</BODY>
</HTML>
*/




Built on: Thursday, October 21, 1999

#endif
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

