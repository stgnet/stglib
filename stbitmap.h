// stbitmap - bitmap handling for stwindow

#ifndef STGLIB_STBITMAP
#define STGLIB_STBITMAP

#pragma message("using stbitmap.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/ststruct.h"
#include "/src/stglib/stbuffer.h"
#include "/src/stglib/strect.h"

class StBitmap;

//////////////////////////////////////////////////////////////////////
//  StColor - structure to define color levels in bitmap
//////////////////////////////////////////////////////////////////////

class StColor
{
public:
	StByte Red;
	StByte Green;
	StByte Blue;
	StByte Alpha;

	StColor()
	{
		Alpha=0;
		Red=0;
		Green=0;
		Blue=0;
	}

	StColor(int r,int g,int b)
	{
		Alpha=0;
		Red=r;
		Green=g;
		Blue=b;
	}

	operator unsigned long(void)
	{
		return(*((unsigned long*)this));
	}

	inline StColor& operator=(unsigned long colorref)
	{
		*((unsigned long*)this)=colorref;
		return(*this);
	}

	void operator()(int r,int g,int b)
	{
		Alpha=0;
		Red=r;
		Green=g;
		Blue=b;
	}

	friend StColor operator|( StColor &left, StColor &right )
	{
		StColor r;

		r.Alpha	=(left.Alpha+right.Alpha)/2;
		r.Red	=(left.Red	+right.Red	)/2;
		r.Green	=(left.Green+right.Green)/2;
		r.Blue	=(left.Blue	+right.Blue	)/2;
		return(r);
	}

};

StColor StColor_White(255,255,255);
StColor StColor_Black(0,0,0);
//////////////////////////////////////////////////////////////////////
//  StPixel - direct pixel manipulation in the bitmap
//////////////////////////////////////////////////////////////////////

class StPixel
{
	friend class StBitmap;
protected:
	StBitmap *bm;
	long x;
	long y;
	long width;
	long height;
public:
	StPixel();
	virtual inline StPixel& operator=(StPixel &copy);			// force copy through color structure

	// these must be defined by instance matching color depth
	virtual inline operator StColor&()=0;						// get color from pixel
	virtual inline StPixel& operator=(StColor color)=0;			// set pixel to color
};

class StPixel1:public StPixel
{
public:
	inline operator StColor&();
	inline StPixel& operator=(StColor color);
};
class StPixel4:public StPixel
{
public:
	inline operator StColor&();
	inline StPixel& operator=(StColor color);
};
class StPixel8:public StPixel
{
public:
	inline operator StColor&();
	inline StPixel& operator=(StColor color);
};
class StPixel24:public StPixel
{
public:
	inline operator StColor&();
	inline StPixel& operator=(StColor color);
};


//////////////////////////////////////////////////////////////////////
//  StBitmapBlit - copy another bitmap into this rectangle
//////////////////////////////////////////////////////////////////////
/*
class StBitmapBlit
{
public:
	StBitmap *Destination;
//	StRect Target;
	long X;
	long Y;
	long W;
	long H;

	StBitmapBlit()
	{
	}

	StBitmapBlit(StSize x,StSize y,StSize w,StSize h)
	{
		X=x;
		Y=y;
		W=w;
		H=h;
	}

	operator<<(StBitmap &Source);
};
*/

//////////////////////////////////////////////////////////////////////
//  StBitmapInfo - handler for BITMAPINFOHEADER structure
//////////////////////////////////////////////////////////////////////
// take the stbuffer class and overlay the BITMAPINFO and color table
class StBitmapInfo:public StBuffer
{
#define pBMI ((BITMAPINFOHEADER*)((StByte*)(*this)))
#define pCOLOR ((StColor*)(((StByte*)(*this))+pBMI->biSize))
public:

	// convenience variables

	virtual void _Reset(void)
	{
		// erase and start over
		!_Storage;
		// initialize allocation for BITMAPINFOHEADER
		_Storage._ArraySetUsed(sizeof(BITMAPINFOHEADER));
	}

	StBitmapInfo()
	{
		_Reset();
	}

	// must declare assignment operator to prevent munged ptrs
	// (is this still necessary since _Storage is a member???)
	StBitmapInfo& operator= (StBitmapInfo& copyfrom)
	{
		_Storage=copyfrom._Storage;
		return(*this);
	}

	inline operator BITMAPINFOHEADER* (void)
	{
		return((BITMAPINFOHEADER*)(StByte*)(*this));  //_Storage._ArrayPtr);
	}
	inline operator BITMAPINFO* (void)
	{
		return((BITMAPINFO*)(StByte*)(*this));  //_Storage._ArrayPtr);
	}

	inline BITMAPINFOHEADER* operator->(void)
	{
		return((BITMAPINFOHEADER*)(StByte*)(*this));  //_Storage._ArrayPtr);
	}
	
//	inline StColor& operator[](StSize index)
	inline StColor& Color(StSize index)
	{
		if (index > pBMI->biClrUsed)
			return(*((StColor*)0));		// nasty: causes caller to crash if index too large


		// insure array large enough for color table
//		_Storage._ArraySetUsed(sizeof(BITMAPINFOHEADER)+pBMI->biClrUsed*sizeof(StColor));

		StColor *pclr=pCOLOR;
		return(pclr[index]);
	}

	// the structure has been changed - revalidate
	void Changed(void)
	{
		if (!pBMI->biWidth || !pBMI->biHeight || !pBMI->biBitCount)
		{
			// zero size bitmap?  leave structure empty...
			return;
		}

		// try to figure out how many colors there are, or actually should be
		if (!pBMI->biClrUsed)
		{
			if (pBMI->biBitCount!=24)
			{
				// must have a number of colors for non-24 bit bmp's
				pBMI->biClrUsed=1<<pBMI->biBitCount;
			}
		}
		// make sure array has enough allocation for color table
		// NO!!! DO NOT DO THIS! instead have a color table reference that prechecks
		// doing this here mucks the array used size causing an empty color table after _Read()!
//		_Storage._ArraySetUsed(sizeof(BITMAPINFOHEADER)+pBMI->biClrUsed*sizeof(StColor));

		// check the fields and patch values that are wrong
		if (!pBMI->biSize)
			pBMI->biSize=sizeof(BITMAPINFOHEADER);

		// will presume that height and width are set correctly
		if (pBMI->biPlanes!=1)
			pBMI->biPlanes=1;

		if (!pBMI->biSizeImage)
		{
			pBMI->biSizeImage=((((pBMI->biWidth * (DWORD)pBMI->biBitCount) 
				+ 31) & ~31) >> 3) * pBMI->biHeight;
		}

		// X/Y PelsPerMeter are unused here
	}

	// rewrite the _Read() call so that we read *ONLY* the info header first,
	// then *ONLY* the size of the color table, then leave the rest for _Bits
	virtual StSize _Read(StBase &Source)
	{
		// before reading, empty the array (don't call ours, which re-allocates the struct!)
		StBuffer::_Reset();

		_ReadBytes(Source,sizeof(BITMAPINFOHEADER));
		if (_Error) return(StErr_RETURN);

		// validate the structure
		Changed();
		StSize color_table_size=pBMI->biClrUsed*sizeof(StColor);

		_ReadBytes(Source,color_table_size);
		return(sizeof(BITMAPINFOHEADER)+color_table_size);
	}

};
	// bitmap info header fields:
	//	DWORD      biSize;
	//	LONG       biWidth;
	//	LONG       biHeight;
	//	WORD       biPlanes;
	//	WORD       biBitCount;
	//	DWORD      biCompression;
	//	DWORD      biSizeImage;
	//	LONG       biXPelsPerMeter;
	//	LONG       biYPelsPerMeter;
	//	DWORD      biClrUsed;
	//	DWORD      biClrImportant;

//////////////////////////////////////////////////////////////////////
//  StBitmap
//////////////////////////////////////////////////////////////////////

// this class includes StBase so it can stream a bitmap file in/out
class StBitmap:public StBase
{
	friend class StPixel;
	friend class StPixel1;
	friend class StPixel4;
	friend class StPixel8;
	friend class StPixel24;

public:
//	StRect _BitmapSize;

private:
    StStruct<BITMAPFILEHEADER> _FileHeader;
	StBitmapInfo _Info;
	StBuffer _Bits;

	// for _BitmapGetDC() only
	HDC _hDC;
	HBITMAP _hBitmap;

	StPixel *_pixel;				// ptr to direct pixel manipulation class for color depth

public:

	void _InitializeImage(long width,long height,int bitcount=24)
	{
	//	DWORD      biSize;
	//	LONG       biWidth;
	//	LONG       biHeight;
	//	WORD       biPlanes;
	//	WORD       biBitCount;
	//	DWORD      biCompression;
	//	DWORD      biSizeImage;
	//	LONG       biXPelsPerMeter;
	//	LONG       biYPelsPerMeter;
	//	DWORD      biClrUsed;
	//	DWORD      biClrImportant;

		!_Info;
		_Info->biWidth=width;
		_Info->biHeight=height;
		_Info->biBitCount=bitcount;
		_Info.Changed();
		_BitmapInfoChanged();
		_Bits._ArrayAllocate(_Info->biSizeImage);
	}

	void _BitmapChangeSize(long width, long height)
	{
		_InitializeImage(width,height);
	}
	void _BitmapChangeSize(StPoint newsize)
	{
		_InitializeImage(newsize.X,newsize.Y);
	}
	void _Reset(void)
	{
		!_FileHeader;
		!_Info;
		!_Bits;
	}

	StBitmap()
	{
		_hDC=NULL;
		_pixel=NULL;
	}

	StBitmap& operator=(StBitmap &copyfrom)
	{
		// make copy of data elements from another bitmap
		_hDC=NULL;
		_pixel=NULL;

		_FileHeader=copyfrom._FileHeader;
		_Info=copyfrom._Info;
		_Bits=copyfrom._Bits;
		_Info.Changed();
				_BitmapInfoChanged();

		return(*this);
	}

	~StBitmap()
	{
		if (_pixel)
			delete _pixel;
	}

	inline StRect &_BitmapSize(void)
	{
		static StRect rect;
		rect(0,0,_Info->biWidth,_Info->biHeight);
		return(rect);
	}

	// paint bitmap on a window (actually defined in StWindow)
	void _BitmapPaint(StWindow &window,StRect &Target,StRect &View=*((StRect *)0));

	// generically paint bitmap on hDC
	void _BitmapPaintHdc(HDC &hdc,StRect &Target)
	{
		// paint the window client area with the bitmap
			SetDIBitsToDevice(
				hdc, 
				0, 
				0,
				_Info->biWidth,
				_Info->biHeight,
				0,				// X SRC
				0,				// Y SRC
				0,						// first scan line
				_Info->biHeight,		// number of scan lines in bitmap
				_Bits,				// ptr to bitmap data
				(const BITMAPINFO*)_Info,
				DIB_RGB_COLORS);
	}

	// allow writing on the bitmap using gdi calls
	HDC _BitmapBeginPaint(void)
	{
		if (_hDC)
			return(_hDC);

		// create a new dc
		HDC hWinDC=GetDC(NULL);
		_hDC=CreateCompatibleDC(hWinDC);

		_hBitmap=CreateCompatibleBitmap(
			hWinDC,
			_BitmapSize().W,
			_BitmapSize().H);

		ReleaseDC(NULL,hWinDC);
		SelectBitmap(_hDC,_hBitmap);
		PatBlt(_hDC,
			_BitmapSize().X1,_BitmapSize().Y1,
			_BitmapSize().X2,_BitmapSize().Y2,
			WHITENESS);
		return(_hDC);
	}
	void _BitmapEndPaint(void)
	{
		// copy the result to the bitmap
		GdiFlush();

		GetDIBits(
			_hDC,				// handle to DC
			_hBitmap,			// handle to bitmap
			0,					// first scan line to set
			_BitmapSize().H,		// number of scan lines to copy
			_Bits,				// array for bitmap bits
			_Info,				// bitmap data buffer
			DIB_RGB_COLORS);	// RGB or palette index
	
		DeleteDC(_hDC);
		_hDC=0;

	
	}




#define _STBITMAP_COLOR_ABS(x,y) (x>y?x-y:y-x)
#define _STBITMAP_COLOR_DIFF(a,b) (_STBITMAP_COLOR_ABS(a.Red,b.Red)+\
	_STBITMAP_COLOR_ABS(a.Green,b.Green)+_STBITMAP_COLOR_ABS(a.Blue,b.Blue))

	StByte _GetBestColorMatch(StColor color)
	{
		StSize index=0;
		StSize best=0;
		StSize best_diff=256*256*256;
		while (index<_Info->biClrUsed)
		{
			StSize this_diff=_STBITMAP_COLOR_DIFF(_Info.Color(index),color);
			if (this_diff<best_diff)
			{
				best=index;
				best_diff=this_diff;
			}
			index++;
		}
		return((StByte)best);
	}

	StSize _Write(const StByte *data,StSize size)
	{
		return(_Err(StErr_NotImplemented,"StBitmap::_Write","low level calls unsupported"));
	}

	StSize _Read(StByte *data,StSize size)
	{
		return(_Err(StErr_NotImplemented,"StBitmap::_Read","low level calls unsupported"));
	}

	StSize _Read(StBase &Source)
	{
		// first read the bitmap header
		_FileHeader._Read(Source);
		if (_FileHeader._Error)
			return(_Err(_FileHeader));
		
	    if (_FileHeader->bfType != 0x4d42)
		{
			// maybe the incoming data doesn't have a _Header on it?
			// return(_Err(StErr_InvalidDataFormat,"StBitmap::_Read","Not a bitmap file"));
			!_Info<<_FileHeader;
		}

		// this read call internally resets buffer first:
		_Info._Read(Source);
		if (_Info._Error)
			return(_Err(_Info));

		_BitmapInfoChanged();

		// this library is not currently supporting compressed bitmap...
		if (_Info->biCompression!=BI_RGB)
			return(_Err(StErr_InvalidDataFormat,"StBitmap::_Read","Compressed bitmap unsupported"));

		!_Bits;
		_Bits._Read(Source);
		if (_Bits._Error)
			return(_Err(_Bits));

		// this is redundant, now done by _Info._Read()
//		_Info.Changed();

		return(~_FileHeader + ~_Info + ~_Bits);
	}

	StSize _WriteTo(StBase &Destination)
	{
		_FileHeader._WriteTo(Destination);
		if (_FileHeader._Error)
			return(_Err(_FileHeader));
		
		_Info._WriteTo(Destination);
		if (_Info._Error)
			return(_Err(_Info));

		_Bits._WriteTo(Destination);
		if (_Bits._Error)
			return(_Err(_Bits));

		return(~_FileHeader + +~_Info + ~_Bits);
	}

	// direct pixel image manipulation
	inline StPixel& operator()(long x,long y)
	{
		if (!_pixel) _InitializeImage(1000,1000);
		_pixel->x=x;
		_pixel->y=_pixel->height-y-1;
		return((*_pixel));
	}
/*
	StBitmapBlit operator()(StSize x,StSize y,StSize w,StSize h)
	{
		StBitmapBlit blit(x,y,w,h);
		blit.Destination=this;

		return(blit);
	}
*/	
	
/*	StBitmap& operator+=(StBitmap &copyfrom)
	{
		long Xfrom=0;
		long Yfrom=0;
		long Xto=copyfrom._BitmapTarget.X1;
		long Yto=copyfrom._BitmapTarget.Y1;

		while (Yfrom<copyfrom._BitmapSize.H)
		{
			Xfrom=0;
			Xto=copyfrom._BitmapTarget.X1;
			while (Xfrom<copyfrom._BitmapSize.W)
			{
				(*this)(Xto,Yto)=copyfrom(Xfrom,Yfrom);
				Xfrom++;
				Xto++;
			}
			Yfrom++;
			Yto++;
		}
		return(*this);
	}
*/
#define ALIGN_TO(x,y) (x+=((x%y)?(y-(x%y)):0))
	void _BitmapInfoChanged(void)
	{

		// preset the pixel manipulator class to use the one matching color depth
		if (_pixel)
			delete _pixel;

		switch (_Info->biBitCount)
		{
		case 24: _pixel=new StPixel24; break;
		case 8:  _pixel=new StPixel8; break;
		case 4:  _pixel=new StPixel4; break;
		case 1:  _pixel=new StPixel1; break;
		default: _pixel=NULL; break; // will cause crash
		}

		// give the pixel class a link back here
		_pixel->bm=this;

		// copy width of bitmap to pixel class for speed...
		_pixel->width=_Info->biWidth;


		// and align to oddities of bitmap
		switch (_Info->biBitCount)
		{
		case 24: _pixel->width+=((_pixel->width&1)?1:0); break;
		case 8:  ALIGN_TO(_pixel->width,4); break;
			//_pixel->width+=2; _pixel->width+=((_pixel->width&1)?1:0); break;
		case 4:  _pixel->width+=6-(_pixel->width&1); break;
		case 1:  _pixel->width+=16-(_pixel->width&7); break;
		}

		// copy height of bitmap to pixel class for inversion
		_pixel->height=_Info->biHeight;
	}

};

//////////////////////////////////////////////////////////////////////
//  StPixel function
//////////////////////////////////////////////////////////////////////
StPixel::StPixel()
{
};

StPixel& StPixel::operator=(StPixel &copy)
{
	// force copy through color
	(*this)=(StColor)copy;
	return(*this);
}

// get color from pixel
inline StPixel1::operator StColor&()
{
	static StColor color;
	StByte *ptr=bm->_Bits;

	ptr+=((y*width+x))/8;
	StByte index=*ptr;

	index>>=7-(x&7);
	index&=0x01;

	return(bm->_Info.Color(index));
};
inline StPixel4::operator StColor&()
{
	static StColor color;
	StByte *ptr=bm->_Bits;

	ptr+=((y*width+x))/2;
	StByte index=*ptr;
	if (!(x&1))
		index>>=4;
	index&=0x0F;
	return(bm->_Info.Color(index));
};
inline StPixel8::operator StColor&()
{
	static StColor color;
	StByte *ptr=bm->_Bits;

	ptr+=(y*width+x);
	return(bm->_Info.Color(*ptr));
};
inline StPixel24::operator StColor&()
{
	static StColor color;
	StByte *ptr=bm->_Bits;

	ptr+=3*(y*width+x);

	color.Blue=ptr[0];
	color.Green=ptr[1];
	color.Red=ptr[2];
	color.Alpha=0;
	return(color);
};

// set pixel from color
StPixel& StPixel1::operator=(StColor color)
{
	StByte index=bm->_GetBestColorMatch(color);
	StByte *ptr=bm->_Bits;
	ptr+=((y*width+x))/8;

	StByte mask=1;
	mask<<=x&7;
	index&=1;
	index<<=x&7;

	*ptr&=~mask;
	*ptr|=index;

	return(*this);
};
StPixel& StPixel4::operator=(StColor color)
{
	StByte index=bm->_GetBestColorMatch(color);
	StByte *ptr=bm->_Bits;
	ptr+=((y*width+x))/2;

	if (x&1)
	{
		*ptr&=0xF0;
		*ptr|=index&0x0F;
	}
	else
	{
		*ptr&=0x0F;
		*ptr|=(index<<4)&0xF0;
	}

	return(*this);
};
StPixel& StPixel8::operator=(StColor color)
{
	StByte index=bm->_GetBestColorMatch(color);
	StByte *ptr=bm->_Bits;
	ptr+=(y*width+x);
	*ptr=index;
	return(*this);
};
StPixel& StPixel24::operator=(StColor color)
{
	StByte *ptr=bm->_Bits;
	ptr+=3*(y*width+x);

	ptr[0]=color.Blue;
	ptr[1]=color.Green;
	ptr[2]=color.Red;
	return(*this);
};


/*
StBitmapBlit::operator <<(StBitmap &Source)
{
	long yt=Y;
	while (yt<Y+H)
	{
		long ys=yt-Y;
		ys*=Source._BitmapSize().H;
		long yp=ys*4;
		ys/=H;
		yp/=H;
		yp&=3;
		if (yp==3)
		{
			++ys;
			yp=0;
		}
		
		long xt=X;
		while (xt<X+W)
		{
			long xs=xt-X;
			xs*=Source._BitmapSize().W;
			long xp=xs*4;
			xs/=W;
			xp/=W;
			xp&=3;
			if (xp==3)
			{
				++xs;
				xp=0;
			}

			StColor c;

			if (yp && xp)
			{
				// mix all 4
				c=(Source(xs,ys)|Source(xs+1,ys))|(Source(xs,ys+1)|Source(xs+1,ys+1));
				StColor red(255,0,0);
				c=red;
			}
			else if (xp)
			{
				// mix horizontal
				c=Source(xs,ys)|Source(xs+1,ys);
//				StColor green(0,255,0);
//				c=green;
			}
			else if (yp)
			{
				// mix vertical
				c=Source(xs,ys)|Source(xs,ys+1);
//				StColor blue(0,0,255);
//				c=blue;
			}
			else
			{
				c=Source(xs,ys);
			}

			(*Destination)(xt,yt)=c;

			++xt;
		}
		++yt;
	}

}
*/

//#include "/src/stglib/stpostfx.h"

#endif


