// STGLIB/stpostfx.h
// Copyright 1999,2002 by StG Net
//
// additional functions that cross refrence multiple classes

#ifdef STGLIB_STBITMAP
#ifdef STGLIB_STWINDOW
//////////////////////////////////////////////////////////////////////
// StBitmap::_BitmapPaint()
//////////////////////////////////////////////////////////////////////
void StBitmap::_BitmapPaint(StWindow &window,StRect &Target,StRect &View)
{
	if (!(&View))
	{
		_BitmapPaint(window,Target,_BitmapSize());
		return;
	}

/*	if (View.Size==Target.Size)
	{
		// one to one - can display bitmap without stretching

		// paint the window client area with the bitmap
		SetDIBitsToDevice(
			window._WindowPaintStruct.hdc, 
			Target.X1, 
			Target.Y1,
			Target.W,
			Target.H,
			View.X1,				// X SRC
			View.Y1,				// Y SRC
			0,						// first scan line
			_bm_info->biHeight,		// number of scan lines in bitmap
			_bm_bits,				// ptr to bitmap data
			(const BITMAPINFO*)_bm_info,
			DIB_RGB_COLORS);
		return;
	}
*/
	// must perform a stretch operation
	StretchDIBits(
		window._WindowPaintStruct.hdc,		// handle to DC
		Target.X1,					// x-coord of destination upper-left corner
		Target.Y1,					// y-coord of destination upper-left corner
		Target.W,					// width of destination rectangle
		Target.H,					// height of destination rectangle
		View.X1,					// x-coord of source upper-left corner
		View.Y1,					// y-coord of source upper-left corner
		View.W,						// width of source rectangle
		View.H,						// height of source rectangle
		_Bits,						// bitmap bits
		_Info,						// bitmap data
		DIB_RGB_COLORS,
		SRCCOPY);						// raster operation code


}

#endif
#endif


//////////////////////////////////////////////////////////////////////
//  STBOX + STDATAFILE
//////////////////////////////////////////////////////////////////////

#ifdef STGLIB_STBOX
#ifdef STGLIB_STDATAFILE
/*
//	inline virtual StBase& operator<< (StBase &that)
template <class TYPE> StBase& StBox<TYPE>::operator<< (StBase &that)
{
	return(*this);
}

template <class TYPE> StBase& StBox<TYPE>::operator>> (StBase &that)
{
	// convert pointer to the destination class to a datafile
	StDataFile *parent=(StDataFile*)&that;

	// create a new datafile instance that references this object
	StDataFile df(*parent,*_DataDescriptor);

	// write my data
//	_Write(df);
	StBase::operator>>(df);

	// look through the items in the box and write them to the datafile
	StBoxRef<TYPE> item(*this);
	while (++item)
		(*item)>>df;

	return(*this);
}
*/

#endif
#endif
