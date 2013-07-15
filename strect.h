// STGLIB/strect.h
// Copyright 2002 by StG Net

// Rectangle calculations 
// Must be used in conjunction with StWindow


#ifndef STGLIB_STRECT
#define STGLIB_STRECT

#pragma message("using strect.h")

#include "/src/stglib/stcore.h"

/* This is a Win32 RECT structure, which this class matches
	to gain convenience and speed when making Win32 calls

typedef struct _RECT { 
  LONG left; 
  LONG top; 
  LONG right; 
  LONG bottom; 
} RECT, *PRECT; 

  And this is the SIZE structure:

typedef struct tagSIZE { 
  LONG cx; 
  LONG cy; 
} SIZE, *PSIZE; 


*/

// class 'links' to many functions within StWindow class
class StWindow;
class StRect;

//////////////////////////////////////////////////////////////////////
// Edge detection flags (stored in a StByte)
//////////////////////////////////////////////////////////////////////
#define StRectEdge_X1 (0x01)
#define StRectEdge_Y1 (0x02)
#define StRectEdge_X2 (0x04)
#define StRectEdge_Y2 (0x08)

//////////////////////////////////////////////////////////////////////
// Window Point Handling (mouse actions, etc)
//////////////////////////////////////////////////////////////////////
class StPoint
{
public:
	long X;
	long Y;

	StPoint()
	{
		X=Y=0;
	}
	inline StPoint(long xi, long yi)
	{
		X=xi;
		Y=yi;
	}
	~StPoint()
	{
	}

	inline StPoint& operator=(StPoint &copyfrom)
	{
		X=copyfrom.X;
		Y=copyfrom.Y;
		return(*this);
	}

	// convenice function to preset position
	inline StPoint& operator()(long x,long y)
	{
		X=x;
		Y=y;
		return(*this);
	}

	inline StPoint& operator+=(StPoint &add)
	{
		X+=add.X;
		Y+=add.Y;
		return(*this);
	};
	inline StPoint& operator-=(StPoint &sub)
	{
		X-=sub.X;
		Y-=sub.Y;
		return(*this);
	};

	inline StPoint& operator-(StPoint &sub)
	{
		static StPoint Result;
		Result.X=X-sub.X;
		Result.Y=Y-sub.Y;
		return(Result);
	};

	inline operator struct tagSIZE *(void)
	{
		return((struct tagSIZE *)&X);
	}
};

//////////////////////////////////////////////////////////////////////
// Zoom ratio handling
//////////////////////////////////////////////////////////////////////

class StZoom
{
	StSize Mul;
	StSize Div;
public:
	StZoom()
	{
		Mul=1;
		Div=1;
	}
	~StZoom()
	{
	}

	inline StZoom& operator()(long m,long d)
	{
		Mul=m;
		Div=d;
		return(*this);
	}
};

//////////////////////////////////////////////////////////////////////
// Window Rect Handling (dimension, view)
//////////////////////////////////////////////////////////////////////

class StRect;

// These sub-classes are cumbersome, but allow provide high usability
// and self-documenting code

class StRectX
{
	friend class StRect;
	StRect *rect;
public:
	StRectX(){}
	~StRectX(){}
	inline operator long();
	inline void operator+=(long);
	inline void operator-=(long);
};
class StRectY
{
	friend class StRect;
	StRect *rect;
public:
	StRectY(){}
	~StRectY(){}
//	inline operator long();
	inline void operator+=(long);
	inline void operator-=(long);
};

class StRectW
{
	friend class StRect;
	StRect *rect;
public:
	StRectW(){}
	~StRectW(){}
	inline operator long();
	inline void operator+=(long);
	inline void operator-=(long);
	inline bool operator=(const long);
};

class StRectH
{
	friend class StRect;
	StRect *rect;
public:
	StRectH(){}
	~StRectH(){}
	inline operator long();
	inline void operator+=(long);
	inline void operator-=(long);
	inline bool operator=(const long);
};

class StRectSize
{
	friend class StRect;
	StRect *rect;
public:
	StRectSize(){}
	~StRectSize(){}
	inline bool operator==(StRectSize &);
	inline bool operator<(StRectSize &);
	inline bool operator=(StPoint &);
};

class StRectZoom
{
	int step;
	friend class StRect;
	StRect *rect;
public:
	StRectZoom();
	~StRectZoom();
	inline StRectZoom& operator++(void);
	inline StRectZoom& operator--(void);
};


class StRect:public StBaseData
{
public:
	long X1;
	long Y1;
	long X2;
	long Y2;

	StRectX		X;		// X positioning of entire rectangle (w/o changing size)
	StRectY		Y;		// Y positioning of entire rectangle (w/o changing size)
	StRectW		W;		// width handling
	StRectH		H;		// height
	StRectSize	Size;	// tests and changes of size in both directions
	StRectZoom	Zoom;	// applies change in size equally to both corners

	StRect():StBaseData((StByte*)&X1,sizeof(long)*4)
	{
		X.rect=this;
		Y.rect=this;
		W.rect=this;
		H.rect=this;
		Size.rect=this;
		Zoom.rect=this;
		X1=Y1=X2=Y2=0;
	}
	~StRect()
	{
	}

	StRect& operator=(StRect &copyfrom)
	{
		// make copy of another rect
		// this is required to prevent corruption of rect ptrs

		if (X1==copyfrom.X1 &&
			Y1==copyfrom.Y1 &&
			X2==copyfrom.X2 &&
			Y2==copyfrom.Y2)
		{
			return(*this);
		}

		X1=copyfrom.X1;
		Y1=copyfrom.Y1;
		X2=copyfrom.X2;
		Y2=copyfrom.Y2;
		NotifyChange();
		return(*this);
	}

	inline StRect& operator()(long newx1, long newy1,long newx2,long newy2)
	{
		X1=newx1;
		Y1=newy1;
		X2=newx2;
		Y2=newy2;
		NotifyChange();
		return(*this);
	}

	// can be declared by inheriting class to obtain notification if change of rect
	inline virtual void NotifyChange(void)
	{
	}

	inline bool operator==(StRect &compare)
	{
		if (X1!=compare.X1) return(false);
		if (Y1!=compare.Y1) return(false);
		if (X1!=compare.X1) return(false);
		if (Y2!=compare.Y2) return(false);
		return(true);
	}

	// convert from one coordinate system to another
	inline StRect& Transform(StRect &From, StRect &To)
	{
		if (From==To)
			return(*this);

		X1=To.X1+(X1-From.X1)*To.W/From.W;
		Y1=To.Y1+(Y1-From.Y1)*To.H/From.H;
		X2=To.X2+(X2-From.X2)*To.W/From.W;
		Y2=To.Y2+(Y2-From.Y2)*To.H/From.H;
		return(*this);
	}


	bool Hit(StPoint point)
	{
		if (point.X<=X1) return(false);
		if (point.Y<=Y1) return(false);
		if (point.X>X2) return(false);
		if (point.Y>Y2) return(false);
		return(true);
	}

	StByte Edge(StPoint point,int width=5)
	{
		StByte edges=0;
		if (abs(point.X-X1)<width) edges|=StRectEdge_X1;
		if (abs(point.Y-Y1)<width) edges|=StRectEdge_Y1;
		if (abs(point.X-X2)<width) edges|=StRectEdge_X2;
		if (abs(point.Y-Y2)<width) edges|=StRectEdge_Y2;
		return(edges);
	}

	inline StRect& AddEdge(StByte edge,StPoint &add)
	{
		if (edge&StRectEdge_X1) 
			X1+=add.X;
		if (edge&StRectEdge_Y1) 
			Y1+=add.Y;
		if (edge&StRectEdge_X2) 
			X2+=add.X;
		if (edge&StRectEdge_Y2) 
			Y2+=add.Y;
		NotifyChange();
		return(*this);
	}

	inline StRect& operator+=(StPoint &add)
	{
		X1+=add.X;
		X2+=add.X;
		Y1+=add.Y;
		Y2+=add.Y;
		NotifyChange();
		return(*this);
	}

	inline operator struct tagRECT *(void)
	{
		return((struct tagRECT *)&X1);
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

};

StRectZoom::StRectZoom()
{
	step=0;
};

StRectZoom::~StRectZoom()
{
};

inline StRectZoom& StRectZoom::operator++(void)
{
	// zoom in
	step++;
	rect->NotifyChange();
	return(*this);
};

inline StRectZoom& StRectZoom::operator--(void)
{
	// zoom out
	step--;
	rect->NotifyChange();
	return(*this);
};

inline StRectW::operator long(void)
{
	return(rect->X2-rect->X1);
};

inline StRectH::operator long(void)
{
	return(rect->Y2-rect->Y1);
};

inline bool StRectW::operator=(const long value)
{
	rect->X2=rect->X1+value;
	rect->NotifyChange();
}
inline bool StRectH::operator=(const long value)
{
	rect->Y2=rect->Y1+value;
	rect->NotifyChange();
}

inline void StRectX::operator+=(long add)
{
	rect->X1+=add;
	rect->X2+=add;
	rect->NotifyChange();
};
inline void StRectY::operator+=(long add)
{
	rect->Y1+=add;
	rect->Y2+=add;
	rect->NotifyChange();
};

inline void StRectX::operator-=(long sub)
{
	rect->X1-=sub;
	rect->X2-=sub;
	rect->NotifyChange();
};
inline void StRectY::operator-=(long sub)
{
	rect->Y1-=sub;
	rect->Y2-=sub;
	rect->NotifyChange();
};

inline bool StRectSize::operator==(StRectSize &that)
{
	// if the width and height are the same, the 'size' of the rect is equal
	if (rect->W!=that.rect->W)
		return(false);
	if (rect->H!=that.rect->H)
		return(false);
	return(true);
}
inline bool StRectSize::operator=(StPoint &size)
{
	rect->X2=rect->X1+size.X;
	rect->Y2=rect->Y1+size.Y;
	rect->NotifyChange();
}

inline bool StRectSize::operator<(StRectSize &that)
{
	return(rect->W+rect->H < that.rect->W+that.rect->H);
}


#endif
