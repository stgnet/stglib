// stg template library
// mysql interface
// NOTE: compiling this requires extra commmand line options under linux:
//	-I'/usr/include/mysql' -L'/usr/lib/mysql' -lmysqlclient
// ALSO: must have package mysql-devel installed

#ifndef STGLIB_mysql
#define STGLIB_mysql

#include "/src/stglib/stcore.h"
#include "/src/stglib/ststring.h"
#include "/src/stglib/stbox.h"

// helpful notification on win32 compilers
#pragma message("using stmysql.h")

// requires -I'/usr/include/mysql'
#include "mysql.h"
#include "mysqld_error.h"

class StMySqlRecord:public StBox<StString>
{
public:
	StMySqlRecord()
	{
	}

	virtual ~StMySqlRecord()
	{
	}
};

class StMySql:public StBox<StMySqlRecord>
{
	MYSQL *_mysql;
	MYSQL_RES *_result;
	StString _query;
public:
	StMySqlRecord _Columns;

	int _ColumnIndex(const char *name)
	{
		int i=0;
		while (i<~_Columns)
		{
			if (_Columns[i]==name)
				return(i);
			++i;
		}
		return(0);
	}

	StSize _MyErr(const char *where)
	{
		// this should instead map errors into StErr_*!
		int my_err=mysql_errno(_mysql);
		StError st_err=(StError)my_err;
		switch (my_err)
		{
		case ER_ACCESS_DENIED_ERROR:
		case ER_DBACCESS_DENIED_ERROR:
			st_err=StErr_PermissionDenied;
		}
		return(_Err(st_err,where,mysql_error(_mysql)));
	}
#ifdef BOGUS_CHANGED
	StMySql(const char *database,const char *UcPaH=NULL)
	{
		const char *user=NULL;
		const char *pass=NULL;
		const char *host=NULL;
		StString temp;

		if (UcPaH)
		{
			/* THIS DOESN"T WORK!
				Problem with using StParse is that it can't be converted back
				to a char * aftewards, without putting each value in a StString!

			StParseString Host;
			StParseBefore<'@'> User(Host);
			StParseAfter<':'> Pass(User);

			host=Host;
			if (~User) user=User;
			if (~Pass) pass=Pass;
			*/

			temp<<UcPaH;
			StByte *scan=temp;
			user=pass=host=(const char *)scan;
			while (*scan)
			{
				if (*scan=='@')
				{
					*scan=0;
					++scan;
					host=(const char *)scan;
					break;
				}
				if (*scan==':')
				{
					*scan=0;
					++scan;
					pass=(const char *)scan;
				}
				++scan;
			}
			if (user==pass)
				pass=NULL;
			if (user==host)
				user=NULL;
		}

//		StdOutput<<"Connecting to '"<<user<<"' : '"<<pass<<"' @ '"<<host<<"' ...\n";

		_mysql=mysql_init(NULL);		
		if (!mysql_real_connect(_mysql,host,user,pass,database,0,NULL,0))
			_MyErr("mysql_real_connect");
	}
#endif
	StMySql(const char *database,const char *host=NULL,const char *user=NULL,const char *pass=NULL)
	{
		_mysql=mysql_init(NULL);		
		if (!mysql_real_connect(_mysql,host,user,pass,database,0,NULL,0))
			_MyErr("mysql_real_connect");
	}
	virtual ~StMySql()
	{
		mysql_close(_mysql);
	}
	virtual StSize _Write(const StByte *data,StSize size)
	{

		// first things first - let's erase everything!!!
		!_query;
		_BoxEmpty();
		!_Columns;
		_ResetError();

		StSize wrote=_query._Write(data,size);

		// make a request
		if (mysql_query(_mysql,_query))
			return(_MyErr("mysql_query"));

		// get the data
		_result=mysql_store_result(_mysql);
		if (!_result)
			return(_MyErr("mysql_store_result"));


//MYSQL_FIELD *field;
//while((field = mysql_fetch_field(_result)))
//{
//    printf("field name %s\n", field->name);
//}

//		printf("Have %d fields, %d rows\n",mysql_num_fields(_result),mysql_num_rows(_result));



		// load the columns
		MYSQL_FIELD *field;
		while (field=mysql_fetch_field(_result))
		{
			+_Columns<<field->name;
		}

		// load the records;
		MYSQL_ROW row;
		unsigned int fields=mysql_num_fields(_result);
		while (row=mysql_fetch_row(_result))
		{
			StMySqlRecord &record=+(*this);
			unsigned long *lengths=mysql_fetch_lengths(_result);
			int i=0;
			while (i<fields)
			{
				StString &s=+record;
				char *ptr=row[i];
				if (!ptr) ptr="";
				s._Write((StByte*)ptr,lengths[i]);

				++i;
			}
		}

		return(wrote);
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
//		MYSQL_ROW row;
		return(0);
	}

/* THIS IS NOT THE CORRECT MEANING OF _Available()
	virtual StSize _Available(void)
	{
		return((StSize)mysql_num_rows(_result));
	}
*/

};

#endif

