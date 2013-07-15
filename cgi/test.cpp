#include "/src/stglib/stdio.h"
#include "/src/stglib/sthtml.h"

int main(int argc,char **argv)
{
	StdOutput<<"Content-type: text/html\n\n";

	StHtmlTag Head("head");
	StHtmlTag Body("body");
	StHtmlTag Html("html");

	"Test Page">>StTag("title")>>Head;
	StTag("meta","http-equiv=\"refresh\" content=15>")>>Head;

//	Body<<StTag("H4")<<"Testing 123";
	"Testing 123">>StTag("H4")>>Body;
	"One">>StTag("a","href=/cgi/one")>>Body;

	Head>>Html;
	Body>>Html;
	Html>>StdOutput;
}
