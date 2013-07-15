// stpalmdb - support for palm database (.PDB) files

#ifndef STGLIB_STPALMDB
#define STGLIB_STPALMDB

#pragma message("using stpalmdb.h")

#include "/stglib/stfile.h"
#include "/stglib/ststruct.h"
#include "/stglib/stbuffer.h"
#include "/stglib/stfield.h"

//////////////////////////////////////////////////////////////////////
//  PALM structure definitions (taken from palm kernel src)
//////////////////////////////////////////////////////////////////////

#define dmDBNameLength 32

typedef UInt32				LocalID;		// local (card relative) chunk ID

/************************************************************
 * Structure of a record list extension. This is used if all
 *  the database record/resource entries of a database can't fit into
 *  the database header.
 *************************************************************/
typedef struct {
	LocalID				nextRecordListID;	// local chunkID of next list
	UInt16				numRecords;			// number of records in this list
//	UInt16				firstEntry;			// array of Record/Rsrc entries 
											// starts here
	} RecordListType;


StField RecordListFields[]=
{
	FIELD_MAP(RecordListType,nextRecordListID),
	FIELD_MAP(RecordListType,numRecords),
//	FIELD_MAP(RecordListType,firstEntry),
	{0,0}
};

/************************************************************
 * Structure of a Record entry
 *************************************************************/
typedef struct {
	LocalID		localChunkID;				// local chunkID of a record
	UInt8			attributes;				// record attributes;
	UInt8			uniqueID[3];			// unique ID of record; should
											//	not be 0 for a legal record.
	} RecordEntryType;

StField RecordEntryFields[]=
{
	FIELD_MAP(RecordEntryType,localChunkID),
	FIELD_MAP(RecordEntryType,attributes),
//	FIELD_MAP(RecordEntryType,uniqueID),
	{0,0}
};


/************************************************************
 * Structure of a Database Header
 *************************************************************/
typedef struct {
	UInt8			name[dmDBNameLength];	// name of database
	UInt16		attributes;					// database attributes
	UInt16		version;					// version of database

	UInt32		creationDate;				// creation date of database
	UInt32		modificationDate;			// latest modification date
	UInt32		lastBackupDate;				// latest backup date
	UInt32		modificationNumber;			// modification number of database

	LocalID		appInfoID;					// application specific info
	LocalID		sortInfoID;					// app specific sorting info

	UInt32		type;						// database type
	UInt32		creator;					// database creator 
	
	UInt32		uniqueIDSeed;				// used to generate unique IDs.
											//	Note that only the low order
											//	3 bytes of this is used (in
											//	RecordEntryType.uniqueID).
											//	We are keeping 4 bytes for 
											//	alignment purposes.

	RecordListType	recordList;				// first record list
	} DatabaseHdrType;		



StField HeaderFields[]=
{
	FIELD_MAP(DatabaseHdrType,attributes),
	FIELD_MAP(DatabaseHdrType,version),
	FIELD_MAP(DatabaseHdrType,creationDate),
	FIELD_MAP(DatabaseHdrType,modificationDate),
	FIELD_MAP(DatabaseHdrType,lastBackupDate),
	FIELD_MAP(DatabaseHdrType,modificationNumber),
	FIELD_MAP(DatabaseHdrType,appInfoID),
	FIELD_MAP(DatabaseHdrType,sortInfoID),
//	FIELD_MAP(DatabaseHdrType,type),
//	FIELD_MAP(DatabaseHdrType,creator),
	FIELD_MAP(DatabaseHdrType,uniqueIDSeed),
	{0,0}
};




#define	dmHdrAttrResDB				0x0001	// Resource database
#define 	dmHdrAttrReadOnly		0x0002	// Read Only database
#define	dmHdrAttrAppInfoDirty		0x0004	// Set if Application Info block is dirty
											// Optionally supported by an App's conduit
#define	dmHdrAttrBackup				0x0008	//	Set if database should be backed up to PC if
											//	no app-specific synchronization conduit has
											//	been supplied.
#define	dmHdrAttrOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
											//  for it to install a newer version of this database
											//  with a different name if the current database is
											//  open. This mechanism is used to update the 
											//  Graffiti Shortcuts database, for example. 
#define	dmHdrAttrResetAfterInstall	0x0020 	// Device requires a reset after this database is 
															// installed.
#define	dmHdrAttrCopyPrevention		0x0040	// This database should not be copied to 

#define	dmHdrAttrStream				0x0080	// This database is used for file stream implementation.
#define	dmHdrAttrHidden				0x0100	// This database should generally be hidden from view
											//  used to hide some apps from the main view of the
											//  launcher for example.
											// For data (non-resource) databases, this hides the record
											//	 count within the launcher info screen.
#define	dmHdrAttrLaunchableData		0x0200	// This data database (not applicable for executables)
											//  can be "launched" by passing it's name to it's owner
											//  app ('appl' database with same creator) using
											//  the sysAppLaunchCmdOpenNamedDB action code. 

#define	dmHdrAttrRecyclable			0x0400	// This database (resource or record) is recyclable:
											//  it will be deleted Real Soon Now, generally the next
											//  time the database is closed. 

#define	dmHdrAttrBundle				0x0800	// This database (resource or record) is associated with
											// the application with the same creator. It will be beamed
											// and copied along with the application. 

#define	dmHdrAttrOpen				0x8000	// Database not closed properly



#define	dmRecAttrDelete			0x80	// delete this record next sync
#define	dmRecAttrDirty			0x40	// archive this record next sync
#define	dmRecAttrBusy			0x20	// record currently in use
#define	dmRecAttrSecret			0x10	// "secret" record - password protected



//////////////////////////////////////////////////////////////////////
//  StPalmDB - read/write palm .PDB file format
//////////////////////////////////////////////////////////////////////
class StPalmDB:public StBase
{
	StFile _File;
	StStruct<DatabaseHdrType> _Header;
	StArrayObj<StStruct<RecordEntryType> > _Record;
	StArray<StSize> _RecordSize;

public:
	StPalmDB(char *path)
	{
		if (!_File._FileOpenExisting(path))
		{
			// read existing header
			_ReadStructure();

		}
		else
		{
			_File._FileCreateNew(path);
			// create new header
		}

	}
	virtual ~StPalmDB()
	{
	}

	StPalmDB& operator=(StPalmDB &copyfrom)
	{
	}


	void _Reset(void)
	{
//		!_FileHeader;
	}
	StByte *_GetName(void)
	{
		return(_Header->name);
	}
	StSize operator~()
	{
		return(_Header->recordList.numRecords);
	}

	// since this is a "virtual" structure, we cannot easily support the low
	// level io calls.


	virtual StSize _Read(StByte *data,StSize size)
	{
		return(_Err(StErr_NotImplemented,"StBitmap::_Read","low level calls unsupported"));
	}
	virtual StSize _Write(const StByte *data,StSize size)
	{
		return(_Err(StErr_NotImplemented,"StBitmap::_Write","low level calls unsupported"));
	}
	virtual StSize _Read(StBase &Source)
	{
		return(_Err(StErr_NotImplemented,"StBitmap::_Read","low level calls unsupported"));
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		return(_Err(StErr_NotImplemented,"StBitmap::_Write","low level calls unsupported"));
	}

	time_t _ConvertFromPalmDate(UInt32 palm_date)
	{
		if (!palm_date)
			return(0);
		tzset();
		return(palm_date+_timezone-3600*24);
	}
	UInt32 _ConvertToPalmDate(time_t std_date)
	{
		if (!std_date)
			return(0);
		tzset();
		return(std_date-_timezone+3600*24);
	}

	StSize _ReadStructure(void)
	{
		// first read the bitmap header
		_File._FileSeek(0);
		_Header._Read(_File);
		if (_Header._Error)
			return(_Err(_Header));

		_FieldSwapBO((StByte*)&_Header,HeaderFields);
		_FieldSwapBO((StByte*)&(_Header->recordList),RecordListFields);

//#ifdef STGLIB_PALMDB_DEBUG

		StdOutput<<"Attributes: ";
		if (_Header->attributes&dmHdrAttrResDB) StdOutput<<"ResDB ";
		if (_Header->attributes&dmHdrAttrReadOnly) StdOutput<<"ReadOnly ";
		if (_Header->attributes&dmHdrAttrAppInfoDirty) StdOutput<<"AppInfoDirty ";
		if (_Header->attributes&dmHdrAttrBackup) StdOutput<<"Backup ";
		if (_Header->attributes&dmHdrAttrOKToInstallNewer) StdOutput<<"OKToInstallNewer ";
		if (_Header->attributes&dmHdrAttrResetAfterInstall) StdOutput<<"ResetAfterInstall ";
		if (_Header->attributes&dmHdrAttrCopyPrevention) StdOutput<<"CopyPrevention ";
		if (_Header->attributes&dmHdrAttrStream) StdOutput<<"Stream";
		if (_Header->attributes&dmHdrAttrHidden) StdOutput<<"Hidden ";
		if (_Header->attributes&dmHdrAttrLaunchableData) StdOutput<<"LaunchableData ";
		if (_Header->attributes&dmHdrAttrRecyclable) StdOutput<<"Recyclable ";
		if (_Header->attributes&dmHdrAttrBundle) StdOutput<<"Bundle ";
		if (_Header->attributes&dmHdrAttrOpen) StdOutput<<"Open ";
		StdOutput<<"\n";
		StdOutput<<"Version   : "<<_Header->version<<"\n";
		StdOutput<<"CreationDT: "<<_ConvertFromPalmDate(_Header->creationDate)<<"\n";
		StdOutput<<"ModifiedDT: "<<_ConvertFromPalmDate(_Header->modificationDate)<<"\n";
		StdOutput<<"LastBckpDT: "<<_ConvertFromPalmDate(_Header->lastBackupDate)<<"\n";
		StdOutput<<"Modifictn#: "<<_Header->modificationNumber<<"\n";
		StdOutput<<"AppInfoID : "<<_Header->appInfoID<<"\n";
		StdOutput<<"SortInfoID: "<<_Header->sortInfoID<<"\n";
		StdOutput<<"Type      : "; StdOutput._Write((StByte*)&_Header->type,4); StdOutput<<"\n"; //<<_Header->type<<"\n";
		StdOutput<<"Creator   : "; StdOutput._Write((StByte*)&_Header->creator,4); StdOutput<<"\n"; //<<_Header->creator<<"\n";
		StdOutput<<"UniqueIDSd: "<<_Header->uniqueIDSeed<<"\n";

		StdOutput<<"NextRecord: "<<_Header->recordList.nextRecordListID<<"\n";
		StdOutput<<"NumRecords: "<<_Header->recordList.numRecords<<"\n";
//#endif


		if (_Header->recordList.nextRecordListID)
		{
			// this is a problem - haven't implemented handling of 2nd set of record ptrs
			return(_Err(StErr_NotImplemented,"StPalmDB::_ReadStructure","next record list handler unimplemented"));
		}

		int index=0;
		while (index<_Header->recordList.numRecords)
		{
			_Record[index]._Read(_File);
			if (_Record[index]._Error)
				return(_Err(_Record[index]));

			_FieldSwapBO(&(_Record[index]),RecordEntryFields);

			++index;
		}

		// go back through and set sizes
		index=0;
		while (index<_Header->recordList.numRecords-1)
		{
			_RecordSize[index]=_Record[index+1]->localChunkID-_Record[index]->localChunkID;
			++index;
		}
		// for the last one, use the current file size
		_RecordSize[index]=_File._FileSize()-_Record[index]->localChunkID;

		// debug
		index=0;
		while (index<_Header->recordList.numRecords)
		{
			StdOutput<<"Record # "<<index+1<<" offset="<<_Record[index]->localChunkID<<" size="<<_RecordSize[index]<<"\n";
			++index;
		}


		return(~_Header);
	}

};



#endif
