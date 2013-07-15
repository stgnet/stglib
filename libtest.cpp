//#define STBASE_DEBUG

#include "/src/stglib/stglib.h"
#include "/src/stglib/stdio.h"

//#include "/src/stglib/stsock.h"
//#include "/src/stglib/stsockraw.h"
//#include "/src/stglib/stsockif.h"

//#include "/src/stglib/stsockaddr.h"

#include "/src/stglib/stbox.h"
//#include "/src/stglib/stdirectory.h"
#include "/src/stglib/stfilter.h"
#include "/src/stglib/stparse.h"
//#include "/src/stglib/sthttp.h"
#include "/src/stglib/stmath.h"
#include "/src/stglib/sttime.h"

//#include "/src/stglib/stui.h"

#include "/src/stglib/sttag.h"

#include "/src/stglib/stconf.h"

//#include "/src/stglib/stspeech.h"

#include "/src/stglib/stcgi.h"

#define CHKERR(x,y) {if ((x)._Error) {StdOutput<<y<<": Error="<<(x)._Error; return(0);}}

//if ((x)._ErrorDescription) StdOutput<<"Description="<<*((x)._ErrorDescription); StdOutput<<"\n"; }

#define PASS (NULL)
#define FAIL ("FAILED")

void good(void)
{
}

/*int http_test(void)
{
	StdOutput<<"HTTP Test\n";

	StBuffer header;

//	header<<"METHOD path version\r\nOne: test1\r\nTwo: test2\r\n\r\n";

	header<<"METHOD this/is/the/url version 1.2.3\r\nOne: test1\r\nTwo: test2\r\n\r\n";

	StHttp http(header);

	StdOutput<<"Method: "<<http._Method<<"\n";
	StdOutput<<"Path: "<<http._Path<<"\n";
	StdOutput<<"Version: "<<http._Version<<"\n";

	StdOutput<<"Number of entires: "<<~http<<"\n";
//	StdOutput<<"one: "<<http("one")<<"\n";

	StSize index=0;
	while (index<~http)
	{
		StdOutput<<http[index]->_Param<<"="<<http[index]->_Value<<"\n";
		++index;
	}

	return(PASS);
}
*/

const char * buffer_ref_test(void)
{
	StBuffer buf1;
	StString str2;

	buf1<<"Test1 OK";
	buf1(4)='2';

	buf1>>str2;
	if (str2!="Test2 OK")
		return("string[] modify failed");

	return(PASS);
}

const char * pipe_test(void)
{
	StPipe pipe1;

	"Hello 123">>pipe1;

	pipe1>>StdOutput;

	return(PASS);
}



class dec_filter:public StFilterByte
{
public:
	StByte _Filter(StByte b)
	{
		--b;
		return(b);
	}

};
// this function tests the operator>> and operator<< returning that for a "pipe" class
const char * filter_test(void)
{
	StString str;
	StString tmp;
//	str._SetDebugName("str");
//	tmp._SetDebugName("tmp");

	// first test that the string conversion returns this
	tmp<<" three";
	str<<"One "<<2<<tmp<<" 4";
	if (str._Error)
		StdError<<"Failed: "<<str._GetErrorString()<<"\n";

	StdOutput<<str<<"\n";
	if (str!="One 2 three 4")
		return("string concat mixed forms");

	// test that the pipes flow correctly
	dec_filter f1; //:StBase("f1");
	dec_filter f2; //:StBase("f2");
	dec_filter f3; //:StBase("f3");
	!tmp<<"cookie";
	!str;
//	tmp>>f1;
//	f1>>StdOutput; //>>f2>>f3>>str;

	tmp>>f1>>f2>>f3>>str;
	if (f1._Error)
		StdError<<"F1: "<<f1._GetErrorString()<<"\n";
	if (f2._Error)
		StdError<<"F2: "<<f2._GetErrorString()<<"\n";
	if (f3._Error)
		StdError<<"F3: "<<f3._GetErrorString()<<"\n";
	if (str._Error)
		StdError<<"Str: "<<str._GetErrorString()<<"\n";

	StdOutput<<str<<"\n";


	if (str=="cookie")
		return("pipe failure, cookie mangled");

	if (str!="`llhfb")
		return("pipe failure");
	return(PASS);
}



const char * average_test(void)
{
	StAverage<int> avg(10);

	int loop=0;
	while (loop<50)
	{
		printf("%d ",loop);
		avg+=loop;
		printf("avg=%d\n",(int)avg);
		++loop;
	}
	return(PASS);
}

const char * stdio_test(void)
{
	// kinda silly to test stdout
	StdOutput<<"Test1: StdOutput<<string\n";
	CHKERR(StdOutput,"StdOutput");

	"Test2: string>>StdOutput\n">>StdOutput;
	CHKERR(StdOutput,"StdOutput");

	StdError<<"Test3: StdError<<string\n";
	CHKERR(StdError,"StdError");
	"Test4: string>>StdError\n">>StdError;
	CHKERR(StdError,"StdError");

	StdOutput<<"Test5: Input copied to output until EOF\n";
 	StdOutput<<StdInput;
	CHKERR(StdInput,"StdInput");
	CHKERR(StdOutput,"StdOutput");


	StdOutput<<"Test6: Input copied to output again until EOF\n";
	StdInput>>StdOutput;
	CHKERR(StdInput,"StdInput");
	CHKERR(StdOutput,"StdOutput");

	StdOutput<<"Test7: IO copied wrong direction, no result\n";
	StdInput<<StdOutput;
	if (StdInput._Error!=StErr_BadFileHandle)
		CHKERR(StdInput,"StdInput");
	if (StdOutput._Error!=StErr_BadFileHandle)
		CHKERR(StdOutput,"StdOutput");

	StdOutput>>StdInput;
	if (StdInput._Error!=StErr_BadFileHandle)
		CHKERR(StdInput,"StdInput");
	if (StdOutput._Error!=StErr_BadFileHandle)
		CHKERR(StdOutput,"StdOutput");

	return(PASS);
}

const char * string_test_sub(StBase &out)
{
	// test initializing string from various sources
	StString string;
	string<<"This is a string";

	char *char_p="This is a char_p";
	const char *const_char_p="This is a const_char_p";

	{
		StString test1(string);
		StString test2(char_p);
		StString test3(const_char_p);
		StString test4("char array");

		out<<test1<<"\n";
		out<<test2<<"\n";
		out<<test3<<"\n";
		out<<test4<<"\n";

	}
	
	{
		StString test1;
		StString test2;
		StString test3;
		StString test4;

		test1=string;
		test2=char_p;
		test3=const_char_p;
		test4="char array";

		out<<test1<<"\n";
		out<<test2<<"\n";
		out<<test3<<"\n";
		out<<test4<<"\n";

	}
	// test writing various types to "string" using base reference
	out<<"String test\n";

	char v_char=123;
	out<<"v_char="<<v_char<<"\n";

	unsigned char v_uchar=234;
	out<<"v_uchar="<<v_uchar<<"\n";


	int v_int=345;
	out<<"v_int="<<v_int<<"\n";

	unsigned int v_uint=456;
	out<<"v_uint="<<v_uint<<"\n";

	short v_short=567;
	out<<"v_short="<<v_short<<"\n";

	unsigned short v_ushort=678;
	out<<"v_ushort="<<v_ushort<<"\n";

	long v_long=789;
	out<<"v_long="<<v_long<<"\n";

	unsigned long v_ulong=890;
	out<<"v_ulong="<<v_ulong<<"\n";

	return(PASS);
}

const char * string_test(void)
{
	StString temp;

	if (string_test_sub(temp))
		return("failed string test to string");

	if (string_test_sub(StdOutput))
		return("failed string test to output");

	return(PASS);
}

const char * stringbuf_test(void)
{
	StString one;
	StString two;

	// load up a couple of strings
	one<<"one";
	two<<"two";

	// check ability to transmogrify into a char*
	const char *test1=one;
	const char *test2=two;

	// the following is broken out so that the cause
	// of any compile failure can be clearly identified
	StdOutput
		<<"One="
		<<one
		<<" test1="
		<<test1
		<<"\n";
	StdOutput<<"Two="<<two<<" test2="<<test2<<"\n";

	if (one==two) return("stringbuf1");

	if (one=="two") return("stringbuf2");
	if (two=="one") return("stringbuf3");

	if (one!="one") return("stringbuf4");
	if (two!="two") return("stringbuf5");

	if (one>two) return("stringbuf6");
	if (two<one) return("stringbuf7");

	return(PASS);
}

/*
class item
{
public:
	int number;
};

const char * stbox_test(void)
{

	StBox<item> list;

	int loop=0;
	while (loop<10)
	{
		item *i=+list;
		i->number=++loop;
	}

	// dump the list
	StBoxRef<item> scan(list);
	while (++scan)
	{
		printf("In box: %d\n",scan->number);
	}
	return(PASS);
}
*/

/*
const char * stdir_test(void)
{
	StDirectory<StDirectoryEntry> list(".");

	StBoxRef<StDirectoryEntry> scan(list);
	while (++scan)
	{
		printf("Found: %s IsDirectory=%d\n",(const char*)scan->_Name,scan->_IsDirectory);
	}
	return(PASS);
}
*/

const char * StStringSection_test(void)
{
	StString foobar("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	StStringSection foosect(foobar);

	StdOutput<<"foobar="<<foobar<<"\n";
	StdOutput<<"section="<<foosect<<"\n";
	if (foosect!="") return("StringSection1");
	foosect(2,3);
	StdOutput<<"section="<<foosect<<"\n";
	if (foosect!="CDE") return("StringSection2");
	return(PASS);
}

/*
const char * StParse_test(void)
{
	StParseString FirstLine;
	StParseTerm<' '> Method(FirstLine);		// usually GET or POST
	StParseTerm<' '> Path(FirstLine);

	FirstLine<<"GET / HTTP/1.1";

	StdOutput<<"Method="<<Method<<"\n";
	StdOutput<<"Path="<<Path<<"\n";
	StdOutput<<"Remainder="<<FirstLine<<"\n";

	if (Method!="GET") return("No get");
	if (Path!="/") return("bad path");
	if (FirstLine!="HTTP/1.1") return("bad firstline");

	return(PASS);
}
*/

const char * stfilter_test(void)
{
	return(PASS);
}

/*
class StMacAddr:public StBaseData
{
public:
	StByte MAC[6];

	StMacAddr():StBaseData(MAC,6)
	{
		memset(MAC,0,6);
	}
	StMacAddr(StByte *buf,StSize len):StBaseData(MAC,6)
	{
		if (len>6)
			len=6;
		if (len<6)
			memset(MAC,0,6);
		memcpy(MAC,buf,len);
	}
};
*/

/*
const char * comparison_sub(StBase &match)
{
	StMacAddr addr4((StByte*)"ABCDEF",6);
	StMacAddr addr5((StByte*)"ABCEFD",6);

	if (addr5==match) 
		return(FAIL);

	if (addr4==match)
		return(PASS);
	return(FAIL);
}

const char * comparison_test(void)
{
	StMacAddr addr1((StByte*)"ABCDEF",6);
	StMacAddr addr2((StByte*)"ABCEFD",6);
	StMacAddr addr3((StByte*)"ABCDEF",6);

	StBase &ref=addr3;

	if (addr1==addr2)
		return(FAIL);
	if (addr1!=addr3)
		return(FAIL);
	if (addr1==addr3)
	{
		if (addr1>addr3)
			return(FAIL);
		if (addr1<addr3)
			return(FAIL);

		if (addr1>addr2)
			return(FAIL);
		if (addr2<addr1)
			return(FAIL);


		if (addr1==ref)
		{
			if (addr2==ref)
				return(FAIL);

			return(comparison_sub(addr1));
		}
	}
	return(FAIL);
}
*/

/*
const char * sttime_test(void)
{
	StTime mark;

	Sleep(2100);

	StSize elapsed=~mark;
	StdOutput<<"Elapsed="<<elapsed<<"\n";
	if (elapsed<2 || elapsed>3)
		return(FAIL);
	return(PASS);
}
*/

/*
const char * copy_test(void)
{
	StIpAddr orig("1.2.3.4");
	StIpAddr same("1.2.3.4");

	StIpAddr copy1(orig);
	
	copy1("5.6.7.8");
	StdOutput<<orig<<" - "<<copy1<<"\n";

	if (orig!=same)
		return(FAIL);

	if (orig==copy1)
		return(FAIL);

	StIpAddr copy2=orig;
	copy2("5.6.7.8");
	StdOutput<<orig<<" - "<<copy2<<"\n";

	if (orig!=same)
		return(FAIL);

	if (orig==copy2)
		return(FAIL);

	StIpAddr copy3;
	copy3=orig;
	copy3("5.6.7.8");
	StdOutput<<orig<<" - "<<copy3<<"\n";

	if (orig!=same)
		return(FAIL);

	if (orig==copy3)
		return(FAIL);

	return(PASS);
}

*/

/*
const char * ui_test(void)
{
	StUi splash("Splash Screen");
	(+splash)->Message("This is a test of the user interface class.");
	(+splash)->Button("OK");
	splash.Dialog();

	return(PASS);
}
*/

const char * error_test(void)
{
	// Check to make sure that an error generated is displayed
//	StBase test;
	StString test;

	test._Err(StErr_NotPossible,"Arbitrary Error");

	StString temp;
	test._ShowError(temp);
	if (!~temp)
		return(FAIL);

	return(PASS);
}

/*
const char * case2_test(void)
{
	StFile temp("this_file_should_not_exist.txt");
	temp<<"Cant write to it";
	if (!temp._Error)
		return("writing to ro file didn't produce error");
	if (temp._Error!=StErr_ReadOnly)
		return("Writing to ro file produced wrong error code");
	StString msg;
	temp._ShowError(msg);
	if (!~msg)
		return("no text error message produced");
	msg>>StdOutput;
	return(PASS);
}
*/

const char * html_test(void)
{
	StString out;

	// basic tag operation
	{
		StTag tag("TAG");
		tag<<"text";
		!out;
		tag>>out;
		out>>StdOutput;
		if (out!="<TAG>text</TAG>\n")
			return("basic tag test failed");
	}
	// with opt
	{
		StTag tag("TAG","Option");
		tag<<"text2";
		!out;
		tag>>out;
		out>>StdOutput;
		if (out!="<TAG Option>text2</TAG>\n")
			return("tag with opt failed");
	}
	// no text
	{
		StTag tag("TAGit","Option");
		!out;
		tag>>out;
		out>>StdOutput;
		if (out!="<TAGit Option/>")
			return("tag with no text failed");
	}
	// reverse inline
	{
		StTag tag("TAG");
		!out;
		"text3">>tag>>out;
		out>>StdOutput;
		if (out!="<TAG>text3</TAG>\n")
			return("reverse inline failed");
	}
		
	// multiple text
	{
		StTag tag("TAG");
		!out;
		"textA">>tag;
		"textB">>tag>>out;
		out>>StdOutput;
		if (out!="<TAG>textAtextB</TAG>\n")
			return("multiple text failed");
	}
		
	// short inline format
	{
		!out;
//		"Text">>StTag("TaG")>>out;
//		out<<StTag("TaG")<<"Text";
		StTag tag("TaG");
		"Text">>tag>>out;
		out>>StdOutput;
		if (out!="<TaG>Text</TaG>\n")
			return("short inline format failed");
	}
	return(PASS);
}

class NullFilter:public StFilterByte
{
public:
	StByte _Filter(StByte b)
	{
		return(b);
	}

};

const char * BasicIO_test(void)
{
	// StString basic check
	StString str1;
	str1<<"hello world";
	if (str1!="hello world")
		return("string comparison failed");

	// Reverse operation
	!str1;
	"Hello World">>str1;
	if (str1!="Hello World")
		return("string reverse failed");

	// StBuffer handling
	// Note: this is testing StBuffer, even though we're
	// using the StStringFunctions wrapper for convenience
//	StBuffer temp1;
	StString temp1;

	temp1<<"one";
	if (temp1!="one")
		return("char * >> buffer");

	// check second write operation accumulation
	temp1<<" two";
	if (temp1!="one two")
		return("second write to buffer");

	// check size return value
	if (~temp1!=7)
		return("size wrong after second write");

	// check reset
	!temp1;
	if (~temp1)
		return("size nonzero after reset");

	if (temp1!="")
		return("value wrong after reset");

	// low level write
	StByte abcd[4]={'A','B','C','D'};

	temp1._Write(abcd,3);
	if (temp1!="ABC")
		return("low level write produced wrong value");

	if (~temp1!=3)
		return("low level write produced wrong size");


	// low level read
	StByte buf[8];
	StSize got=temp1._Read(buf,7);
	if (got!=3)
		return("low level read wrong size");
	if (memcmp(buf,abcd,3))
		return("low level read returned wrong value");

	// test multiple operations together
	!temp1<<"one"<<" "<<"two";
	if (temp1!="one two")
		return("multiple operations");

	// reverse operation
	!temp1;
	"reverse">>temp1;
	if (temp1!="reverse")
		return("reverse operation");

	// reverse multiple
	"reverse multiple">>!temp1;
	if (temp1!="reverse multiple")
		return("reverse multiple");

	"forward">>!temp1<<" and reverse";
	if (temp1!="forward and reverse")
		return("forward and reverse");

//	StdOutput<<temp1<<"\n";

	// pass from string to buffer to string
	StBuffer temp2;
	StString temp3;

	!temp1<<"today is a good day to code";
	temp1>>temp2>>temp3;
	if (temp3!="today is a good day to code")
		return("buffer passing 1");

	// try it again with a filter in the middle
	NullFilter filter1;
	!temp1<<"water";
	!temp3;
	temp1>>filter1>>temp3;
	if (temp3!="water")
		return("buffer through filter 1");

	// try it again with two filters in the middle
	NullFilter filter2;
	!temp1<<"oil";
	!temp3;
	temp1>>filter1>>filter2>>temp3;
	if (temp3!="oil")
		return("buffer through filter 2");

	// try it again with steps separated
	!temp1<<"H2o";
	!temp3;
	temp1>>filter1;
	filter1>>filter2;
	filter2>>temp3;
	if (temp3!="H2o")
		return("buffer with individual steps");


	// check comparsion of two strings
	StString a("o");
	StString b("o");

	if (a!=b)
		return("string compare != invalid");

	if (a==b)
		good();
	else
		return("string compare not == inavlid");

	a<<"ne";
	b<<"ther";

	if (a==b)
		return("string compare == invalid");

	if (a!=b)
		good();
	else
		return("string compare not != invalid");

	// check comparison to empty strings
	StString e1;
	StString e2;

	if (a==e1)
		return("empty string fail 1");

	if (a!=e1)
		good();
	else
		return("empty string fail not 1");

	if (e1==a)
		return("emtpy string fail 2");

	if (e1!=a)
		good();
	else
		return("empty string fail not 2");

	if (e1==e2)
		good();
	else
		return("empty string fail 3");

	if (e1!=e2)
		return("empty string fail 4");

	return(PASS);
}

const char * array_test(void)
{
	StSize original=_StArray_Bytes;
	{
		StArray<int> at;

		if (~at)
			return("empty array non zero used?");

		// add one element
		at._ArrayAdd(1);

		if (~at!=1)
			return("array not sized 1");
		if (at[0]!=1)
			return("element 1 not correct");

		// try forcing it out of order
		at[2]=3;

		if (~at!=3)
			return("array not sized 3");
		if (at[1]!=0)
			return("element 2 not correct");
		if (at[2]!=3)
			return("element 2 not correct");


		StdOutput<<"Memory usage: "<<(const unsigned int)(_StArray_Bytes-original)<<" bytes\n";
	}
	if (_StArray_Bytes!=original)
		return("memory leak detected");

	return(PASS);
}
const char * stack_test(void)
{
	StSize original=_StArray_Bytes;
	{
		StStack<int> st;

		if (~st)
			return("empty stack non zero used?");

		++st=1;		

		if (~st!=1)
			return("stack not sized 1");
		if (st[0]!=1)
			return("stack element 1 not correct");

		++st=2;

		if (~st!=2)
			return("stack not sized 2");
		if (st[0]!=2)
			return("element 1 not correct");
		if (st[1]!=1)
			return("element 2 not correct");

		if (st--!=2)
			return("unstacked value not 2");

		if (~st!=1)
			return("post unstack not sized 1");
		if (st[0]!=1)
			return("post unstack element 1 not correct");


		StdOutput<<"Memory usage: "<<(const unsigned int)(_StArray_Bytes-original)<<" bytes\n";
	}
	if (_StArray_Bytes!=original)
		return("memory leak detected");

	return(PASS);
}

const char * conf_test(void)
{
	{
		StFile temp("temp.conf",StFileOpenExistingOrCreate);
		"#this is a comment\n">>temp;
		"one=1\n">>temp;
		"two=2\n">>temp;
	}

	StFile conf_file("temp.conf");
	StConf conf(conf_file);

	if (conf("one")!="1")
		return("first value wrong");
	if (conf("two")!="2")
		return("second value wrong");

	return(PASS);
}

/*
class case3:public StBox<StString>
{
public:
	StBox<StString> Pint;
	case3()
	{
		+(*this)="inh one";		
		+(*this)="inh two";
		+Pint="pub one";
		+Pint="pub two";
	}
};

const char * case3_test(void)
{
	// check inherited and public box[] reference 
	case3 check;

	StString temp;

	StdOutput<<check[0]<<"\n";
	StdOutput<<check[1]<<"\n";
	StdOutput<<check.Pint[0]<<"\n";
	StdOutput<<check.Pint[1]<<"\n";


	temp<<check[0];
	if (temp!="inh one")
		return("inh one wrong");
	if (check[1]!="inh two")
		return("inh one wrong");
	if (check.Pint[0]!="pub one")
		return("inh one wrong");
	if (check.Pint[1]!="pub two")
		return("inh one wrong");

	// make sure that referencing invalid array length throws up
	const char *fail_msg=0;
	try
	{
		const char *test=check[2];
	}
	catch(const char *except)
	{
		fail_msg=except;
	}
	if (!fail_msg)
		return("invalid array didn't produce exception");

	return(PASS);
}
*/

const char *pair_test(void)
{
	StPair pairs;

	pairs("one")<<"a";

	StString temp;
	temp<<"bb";

	pairs("two")<<temp;

	!temp<<"three=ccc";
	pairs._AddEquals(temp);

	if (pairs("one")!="a")
		return("pair 1 failed");

	if (pairs("two")!="bb")
		return("pair 2 failed");

	if (pairs("three")!="ccc")
		return("pair 2 failed");


	return(PASS);
}

const char *atoi_test(void)
{
	StString test("1234");
	int value;

	test>>value;
	if (value!=1234)
		return("wrong value");

	return(PASS);
}

const char * base64_test(void)
{
	// check base 64 decode
	StString result;

	StDecodeB64 b64;
	"dGhpcyBpcyBhIHRlc3Q6aGVsbG8gdGhlcmU=">>b64>>result;
	StdOutput<<result<<"\n";

	return(PASS);
}

const char *endian_test(void)
{
	// check to make sure that StNet# endian conversions work

	StNet2 value1;
	StNet4 value2;

	StByte *ptr1=(StByte*)&value1;
	StByte *ptr2=(StByte*)&value2;

	ptr1[0]=0x12; ptr1[1]=0x34;
	ptr2[0]=0xFE; ptr2[1]=0xED; ptr2[2]=0xC0; ptr2[3]=0xDE;

	StByt2 result1=value1;
	StByt4 result2=value2;

	StdOutput<<"Result1="<<StHex(result1,4)<<"\n";
	StdOutput<<"Result2="<<StHex(result2,8)<<"\n";

	if (result1!=0x1234) return("endian test1 failed");
	if (result2!=0xFEEDC0DE) return("endian test2 failed");


	// now change the value and check the bytes
	value1=0x7890;
	value2=0x12345678;

	StdOutput<<"Result3="<<StHex(ptr1[0],2)<<StHex(ptr1[1],2)<<"\n";
	StdOutput<<"Result4="<<StHex(ptr2[0],2)<<StHex(ptr2[1],2)<<StHex(ptr2[2],2)<<StHex(ptr2[3],2)<<"\n";

	if (ptr1[0]!=0x78 || ptr1[1]!=0x90) return("endian test3 failed");
	if (ptr2[0]!=0x12 || ptr2[1]!=0x34 || ptr2[2]!=0x56 || ptr2[3]!=0x78) return("endian test4 failed");

	return(PASS);
}


typedef const char * (*test_func)(void);

void DoTest(const char *name,test_func test)
{
	StdOutput<<"----------------------- TEST: "<<name<<"\n";
	const char *failure;
	try
	{
		failure=(*test)();
	}
	catch(const char *except)
	{
		StdOutput<<"*** EXCEPTION: "<<name<<"\nREASON: "<<except<<"\n";
		exit(1);
	}
	if (failure)
	{
		StdOutput<<"*** FAILED: "<<name<<"\nREASON: "<<failure<<"\n";
		exit(1);
	}
}
// TESTS TO ADD
// 1) check stpair operation, esp. separate entries on copied pair box

int main(void)
{
	DoTest("array",array_test);
	DoTest("stack",stack_test);
	DoTest("error",error_test);
	DoTest("atoi",atoi_test);
	DoTest("BasicIO",BasicIO_test);
	DoTest("conf",conf_test);
	DoTest("html",html_test);
	DoTest("pair",pair_test);
//	DoTest("case2",case2_test);
//	DoTest("case3",case3_test);
	DoTest("base64",base64_test);
	DoTest("filter",filter_test);
	DoTest("string",string_test);
	DoTest("stringbuf",stringbuf_test);
//	DoTest("box",stbox_test);
	DoTest("StringSection",StStringSection_test);
	DoTest("filter",stfilter_test);
	DoTest("endian",endian_test);

	StdOutput<<">>>>>>>>> Tests passed <<<<<<<<<\n";
	exit(0);

	return(0);
}
