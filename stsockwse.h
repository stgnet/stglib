// winsock errors used by StSock::_SockErrorDescription()


char *__WinsockErrorDesc[]=
{
	/* 000 */ "?",
	/* 001 */ "?",
	/* 002 */ "?",
	/* 003 */ "?",
	/* 004 */ "Interrupted System Call",
	/* 005 */ "?",
	/* 006 */ "?",
	/* 007 */ "?",
	/* 008 */ "?",
	/* 009 */ "Bad File Number",

	/* 010 */ "?",
	/* 011 */ "?",
	/* 012 */ "?",
	/* 013 */ "Permission Denied",
	/* 014 */ "Bad Address",
	/* 015 */ "?",
	/* 016 */ "?",
	/* 017 */ "?",
	/* 018 */ "?",
	/* 019 */ "?",

	/* 020 */ "?",
	/* 021 */ "?",
	/* 022 */ "Invalid Argument",
	/* 023 */ "?",
	/* 024 */ "Too many open files",
	/* 025 */ "?",
	/* 026 */ "?",
	/* 027 */ "?",
	/* 028 */ "?",
	/* 029 */ "?",

	/* 030 */ "?",
	/* 031 */ "?",
	/* 032 */ "?",
	/* 033 */ "?",
	/* 034 */ "?",
	/* 035 */ "Operation Would Block",
	/* 036 */ "Operation in progress",
	/* 037 */ "Operation already in progress",
	/* 038 */ "Not a socket",
	/* 039 */ "Destination address required",

	/* 040 */ "Message too long",
	/* 041 */ "Protocol wrong type for socket",
	/* 042 */ "Protocol not available",
	/* 043 */ "Protocol not supported",
	/* 044 */ "Socket type not supported",
	/* 045 */ "Operation not supported at endpoint",
	/* 046 */ "Protocol family not supported",
	/* 047 */ "Address family not supported",
	/* 048 */ "Address in use",
	/* 049 */ "Address not available",

	/* 050 */ "Network is down",
	/* 051 */ "Network is unreachable",
	/* 052 */ "Network reset dropped connection",
	/* 053 */ "Connection aborted",
	/* 054 */ "Connection reset by peer",
	/* 055 */ "No buffer space available",
	/* 056 */ "Already connected",
	/* 057 */ "Not connected",
	/* 058 */ "Cannot send after shutdown",
	/* 059 */ "Too many references",

	/* 060 */ "Connection timed out",
	/* 061 */ "Connection refused",
	/* 062 */ "Too many links",
	/* 063 */ "Name too long",
	/* 064 */ "Host is down",
	/* 065 */ "No route to host",
	/* 066 */ "Directory not empty",
	/* 067 */ "WSAEPROCLIM",
	/* 068 */ "Too many users",
	/* 069 */ "Quote exceeded",

	/* 070 */ "Stale file handle",
	/* 071 */ "Object is remote",
	/* 072 */ "?",
	/* 073 */ "?",
	/* 074 */ "?",
	/* 075 */ "?",
	/* 076 */ "?",
	/* 077 */ "?",
	/* 078 */ "?",
	/* 079 */ "?",

	/* 080 */ "?",
	/* 081 */ "?",
	/* 082 */ "?",
	/* 083 */ "?",
	/* 084 */ "?",
	/* 085 */ "?",
	/* 086 */ "?",
	/* 087 */ "?",
	/* 088 */ "?",
	/* 089 */ "?",

	/* 090 */ "?",
	/* 091 */ "WSASYSNOTREADY",
	/* 092 */ "WSAVERNOTSUPPORTED",
	/* 093 */ "WSANOTINITIALIZED",
	/* 094 */ "?",
	/* 095 */ "?",
	/* 096 */ "?",
	/* 097 */ "?",
	/* 098 */ "?",
	/* 099 */ "?",

	/* 100 */ "?",
	/* 101 */ "WSAEDISCON",
	/* 102 */ "WSAENOMORE",
	/* 103 */ "WSAECANCELLED",
	/* 104 */ "WSAEINVALIDPROCTABLE",
	/* 105 */ "WSAEINVALIDPROVIDER",
	/* 106 */ "WSAEPROVIDERFAILEDINIT",
	/* 107 */ "WSASYSCALLFAILURE",
	/* 108 */ "WSASERVICE_NOT_FOUND",
	/* 109 */ "WSATYPE_NOT_FOUND",

	/* 110 */ "WSA_E_NO_MORE",
	/* 111 */ "WSA_E_CANCELLED",
	/* 112 */ "WSAEREFUSED",
	/* 113 */ "?",
	/* 114 */ "?",
	/* 115 */ "?",
	/* 116 */ "?",
	/* 117 */ "?",
	/* 118 */ "?",
	/* 119 */ "?",
};

const char *_SockErrorDescription(StError error_code,const char *system_msg)
{
#ifdef WIN32
	// Winsock error numbers are in a separate numerical sequence, and
	// are not available directly from the OS.  Instead, they are looked
	// up in a table provided by stsockwse.h
	unsigned entries=sizeof(__WinsockErrorDesc)/sizeof(char*);

	if (((unsigned)error_code)>=WSABASEERR && (unsigned)error_code<WSABASEERR+entries)
		return(__WinsockErrorDesc[error_code-WSABASEERR]);
#endif
	return(system_msg);
}
