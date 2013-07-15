// StGLIB/stfield.h
// field map handling


#ifndef STGLIB_STFIELD
#define STGLIB_STFIELD

#pragma message("using stfield.h")

#include "/src/stglib/stcore.h"

#define FIELD_MAP(c,f) { ((size_t)(char*)&((c*)0)->f) , sizeof(((c*)0)->f) }

typedef struct
{
	StSize Offset;
	StSize Size;

} StField;


	void inline _swap_bo(StByte *ptr,StSize len)
	{
		StByte *end=ptr+len;
		
		--end;
		while (ptr<end)
		{
			StByte temp=*ptr;
			*ptr=*end;
			*end=temp;

			++ptr;
			--end;
		}
	}

	void _FieldSwapBO(void *pStruct,StField *table)
	{
		static unsigned short swapbo_test=1;

		if (!(*((StByte*)(&swapbo_test))))
			return;

		while (table->Size)
		{
			_swap_bo((StByte*)pStruct+table->Offset,table->Size);
			++table;
		}

	}

#endif
