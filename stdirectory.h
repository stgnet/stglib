// STGLIB/stdirectory.h
// Copyright 1999,2002,2006 by Scott Griepentrog & StG Net

// directory class

// EXAMPLE CODE:
//
//	void ScanDirectory(const char *path)
//	{
//	        StDirectory<StDirectoryEntry> list(path);
//	
//	        StBoxRef<StDirectoryEntry> scan(list);
//	        while (++scan)
//	        {
//	                StDirectoryPath fullpath(path,scan->_Name);
//	
//	                StdOutput<<fullpath<<"\n";
//	
//	                if (scan->_IsDirectory)
//	                        ScanDirectory(fullpath);
//	        }
//	}


#ifndef STGLIB_STDIRECTORY
#define STGLIB_STDIRECTORY

#pragma message("using stdirectory.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/ststring.h"
#include "/src/stglib/stbox.h"


//////////////////////////////////////////////////////////////////////
// StDirectoryEntry - base class for directory entries (holds stamps)
//////////////////////////////////////////////////////////////////////
class StDirectoryEntry
{
public:
	StString	_Name;
//	StSize		_Size;		// linux doesn't give this
	StDateTime	_Update;
	StByte		_IsDirectory;
	StByte		_IsDriveLetter;

	StDirectoryEntry()
	{
//		_Size=0;
//		_Update=0;
		_IsDirectory=0;
		_IsDriveLetter=0;
	}

	bool operator>(StDirectoryEntry &other)
	{
		return(_Name>other._Name);
	}
	bool operator<(StDirectoryEntry &other)
	{
		return(_Name<other._Name);
	}

	bool operator==(const char *s)
	{
		return(_Name==s);
	}
	
};

#define is_drive_letter(s) (*(s)>='A' && *(s)<='Z' && (s)[1]==':' && ((s)[2]=='/' || !(s)[2]))

//////////////////////////////////////////////////////////////////////
// StDirectoryPath - overloaded StString with path concatenation
//////////////////////////////////////////////////////////////////////
class StDirectoryPath:public StString
{
public:
	StDirectoryPath(const char *path,const char *file)
	{
		// walk the path and check for trailing /
		char lc=0;
		const char *p=path;
		while (*p)
		{
			lc=*p;
			++p;
		}

		// build new string
		(*this)<<path;
		if (lc!='/')
			(*this)<<"/";
		(*this)<<file;
	}
};


#ifdef STGLIB_STP_WIN32
// ### Move this section to stp_win32 later ###
// emulate opendir/readdir for WIN32

#define DIR StDirectoryFromWIN32
#define DT_DIR 0x01
struct dirent
{
	char d_name[_MAX_PATH];
	char d_type;

};

class StDirectoryFromWIN32
{
public:
	long handle;
	struct _finddata_t entry;
	int state;
	struct dirent de;

	struct dirent *Entry(void)
	{
		strcpy(de.d_name,entry.name);
		de.d_type=0;
		if (entry.attrib&_A_SUBDIR)
			de.d_type=DT_DIR;

		return(&de);
	}
};


StDirectoryFromWIN32 *opendir(const char *path)
{
	StDirectoryFromWIN32 *d=new StDirectoryFromWIN32;

	StString path_star;
	path_star<<path;

	if (!(path_star._Contains("*")>=0))
		path_star<<"/*";

	d->state=0;
	d->handle=_findfirst(path_star,&(d->entry));
	if (d->handle!=-1)
		d->state=1;

	return(d);
}
struct dirent *readdir(StDirectoryFromWIN32 *d)
{
	if (d->state)
	{
		d->state=0;
		return(d->Entry());
	}
	if (d->handle==-1 || _findnext(d->handle,&(d->entry)))
	{
		return(0);
	}
	return(d->Entry());
}
void closedir(StDirectoryFromWIN32 *d)
{
	delete d;
}


#endif



//////////////////////////////////////////////////////////////////////
// StDirectory - obtain lists of files as an StBox of StString's
//////////////////////////////////////////////////////////////////////
// note: it is presumed that ITEM has a base of StString

template <class ITEM> class StDirectory:public StBox<ITEM>
{
public:
	StDirectory(const char *path)
	{
		if (!path || !*path)
		{
#ifdef WIN32
			// on WIN32 platform, presume a null path to be request for "root"
			// which can be reinterpreted as a request for present letter drives
			DWORD drives=GetLogicalDrives();
			StChar letter='A';
			while (drives)
			{
				if (drives&1)
				{
					ITEM *item=new ITEM;
					char dl[3];
					dl[0]=letter;
					dl[1]=':';
					dl[2]=0;
					item->_Name<<dl;
					item->_IsDirectory=1;
					item->_IsDriveLetter=1;

					_BoxAdd(item);
				}
				drives>>=1;
				letter++;
			}
			return;
#else
			path="/";
#endif
		}

		// change /X: to X:
		if (*path=='/' && is_drive_letter(path+1))
			++path;
		

		struct dirent *de;

		DIR *dp=opendir(path);
		if (!dp)
		{
			this->_Errno("StDirectory","opendir");
			return;
		}

		de=readdir(dp);
/*		if (!de)
		{
//			this->_Errno("StDirectory","readdir");
			closedir(dp);
			return;
		}
*/
		while (de)
		{
			if (de->d_name[0]!='.')
			{
				ITEM *item=new ITEM;
//StdOutput<<de->d_name<<"="<<de->d_type<<"\n";

				item->_Name<<de->d_name;
	//			item->_Size=de.d_size;
	//			item->_Update=de.time_write;
	//			if (de.attrib&_A_SUBDIR)
				if (de->d_type==DT_DIR)
					item->_IsDirectory=1;

				_BoxAdd(item);
			}
			de=readdir(dp);
		}

//		while (!_findnext(hff,&de))

		closedir(dp);
	}
};

#endif

