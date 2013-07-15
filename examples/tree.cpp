// example of StDirectory usage

#include "/src/stglib/stdio.h"
#include "/src/stglib/stdirectory.h"

void Directory(const char *path)
{
	StDirectory<StDirectoryEntry> list(path);

	StBoxRef<StDirectoryEntry> scan(list);
	while (++scan)
	{
		StDirectoryPath fullpath(path,scan->_Name);

		StdOutput<<fullpath<<"\n";

		if (scan->_IsDirectory)
			Directory(fullpath);
	}
}

int main(int argc,char **argv)
{
	char *path;
	if (!*++argv)
		path=".";
	else
		path=*argv;

	Directory(path);

	return(0);
}
