#include <stdio.h> // XXX
#include <stdlib.h>
#include <list>

#include <base/system.h>
#include <game/version.h>
#include "debug.h"

#define CALLSTACK_SIZE 20

IStorageTW * CDebugger::m_pStorage = 0;

CCallstack gDebugInfo;

CDebugger::CDebugger()
{
	for(int i = 0; i < CALLSTACK_SIZE; i++)
		gDebugInfo.m_CallStack.push_front("..");

	RegisterSignals();
}

void CDebugger::RegisterSignals()
{
	//signal(SIGABRT, signalhandler);
	signal(SIGFPE, signalhandler);
	signal(SIGILL, signalhandler);
	/*signal(SIGINT, signalhandler);*/ // DON'T ever catch this!!
	signal(SIGSEGV, signalhandler);
	/*signal(SIGTERM, signalhandler);*/ // not sure, but I think this shall not be caught aswell
#if defined(CONF_FAMILY_UNIX)
	signal(SIGSTKFLT, signalhandler);
	signal(SIGPIPE, signalhandler);
	signal(SIGHUP, signalhandler);
#endif
}

void CCallstack::CallstackAdd(const char *file, int line, const char *function)
{
	static char aStamp[128]; // static so that we only need to allocate once -> faster
	str_format(aStamp, sizeof(aStamp), "at %s:%i in %s", file, line, function);
	gDebugInfo.m_CallStack.pop_back();
	gDebugInfo.m_CallStack.push_front(std::string(aStamp));
}

void signalhandler(int sig) { CDebugger::signalhandler_ex(sig); }
void CDebugger::signalhandler_ex(int sig)
{
	const char *paSigNames[] = {
		"_INTERNAL_ERROR" // there is no 0
		"SIGHUP",	//	1	/* Hangup (POSIX).  */
		"SIGINT",	//	2	/* Interrupt (ANSI).  */
		"SIGQUIT",	//	3	/* Quit (POSIX).  */
		"SIGILL",	//	4	/* Illegal instruction (ANSI).  */
		"SIGTRAP",	//	5	/* Trace trap (POSIX).  */
		"SIGABRT",	//	6	/* Abort (ANSI).  */
		"SIGIOT",	//	6	/* IOT trap (4.2 BSD).  */
		"SIGBUS",	//	7	/* BUS error (4.2 BSD).  */
		"SIGFPE",	//	8	/* Floating-point exception (ANSI).  */
		"SIGKILL",	//	9	/* Kill, unblockable (POSIX).  */
		"SIGUSR1",	//	10	/* User-defined signal 1 (POSIX).  */
		"SIGSEGV",	//	11	/* Segmentation violation (ANSI).  */
		"SIGUSR2",	//	12	/* User-defined signal 2 (POSIX).  */
		"SIGPIPE",	//	13	/* Broken pipe (POSIX).  */
		"SIGALRM",	//	14	/* Alarm clock (POSIX).  */
		"SIGTERM",	//	15	/* Termination (ANSI).  */
		"SIGSTKFLT"	//	16	/* Stack fault.  */
	};
	const std::list <std::string> m_FinalCS = gDebugInfo.m_CallStack;
	printf("\n\n\n\n\n\n\n\nBUG BUG! sig=%s (%i)\n", paSigNames[sig>16||sig<1?0:sig], sig);

	for(std::list<std::string>::const_iterator it = m_FinalCS.begin(); it != m_FinalCS.end(); it++)
	{
		printf("%s\n", it->c_str());
	}

	fs_makedir("./crashlogs");
	char aFile[512], aBuf[768];
	time_t rawtime;
	time(&rawtime);
	str_timestamp_ex(rawtime, aFile, sizeof(aFile), "crashlogs/report_%Y%m%d%H%M%S.log");
	IOHANDLE f = io_open(aFile, IOFLAG_WRITE);
	bool ReportExists = true;

	if(!f)
	{
		ReportExists = false;
		printf("<<<< FAILED TO OPEN '%s' FOR WRITING >>>>\n", aFile);
		printf("<<<<  no crash report will be saved!  >>>>\n\n\n");
	}
	else
	{
		#define WRITE_LINE() \
				io_write(f, aBuf, str_length(aBuf)); \
				io_write_newline(f)

		str_timestamp_ex(rawtime, aBuf, sizeof(aBuf), "### io.github.allthehaxx::ath.crashreport.autogenerated %d.%m.%Y %H:%M:%S");
		WRITE_LINE();

		str_format(aBuf, sizeof(aBuf), "### %s", aFile);
		WRITE_LINE();
		io_write_newline(f);

		str_format(aBuf, sizeof(aBuf), "OS: " CONF_FAMILY_STRING "/" CONF_PLATFORM_STRING);
		WRITE_LINE();

		str_format(aBuf, sizeof(aBuf), "VERSION: " ALLTHEHAXX_VERSION ".%i.%i (" GAME_VERSION "/" ATH_VERSION ")", GAME_ATH_VERSION_NUMERIC, CLIENT_VERSIONNR);
		WRITE_LINE();

		str_format(aBuf, sizeof(aBuf), "ERRSIG: %s (%i)", paSigNames[sig>16||sig<1?0:sig], sig);
		WRITE_LINE();

		io_write_newline(f);
		io_write_newline(f);

		str_format(aBuf, sizeof(aBuf), "******* BEGIN CALLSTACK *******");
		WRITE_LINE();

		for(std::list<std::string>::const_iterator it = m_FinalCS.begin(); it != m_FinalCS.end(); it++)
		{
			io_write(f, it->c_str(), (unsigned int)str_length(it->c_str()));
			io_write_newline(f);
		}

		str_format(aBuf, sizeof(aBuf), "*******  END CALLSTACK  *******");
		WRITE_LINE();

		io_flush(f);
		io_close(f);
	}

	char aMsg[512];
	str_format(aMsg, sizeof(aMsg), "Whoups, the client just crashed. I'm sorry... ");

	if(ReportExists)
	{
		str_format(aBuf, sizeof(aBuf), "A crash report has been saved to '%s' and will be sent on next client start."
						" It doesn't contain any sensitive info and will be very helpful for us to fix the bug, thanks! :)"
						"   [If your mouse is still grabbed, you can press either ENTER or ALt+F4 to close this]", aFile);

	}
	else
		str_format(aBuf, sizeof(aBuf), "Unfortunately, we couldn't write to the file '%s' in order to save a report :/", aFile);

	str_append(aMsg, aBuf, sizeof(aMsg));

	const char *paFunnyMessages[] = { // these should - please - fit to a crash. Thanks in advance.
			"I know what you think...",
			"This was not supposed to happen!",
			"Who is responsible for that mess?",
			"That's not my fault!",
			"My cat ate the alien intelligence",
			"What are we going to do now?",
			"YOU DIDN'T SAW THAT! IT'S CONFIDENTIAL!",
			"Please act as if nothing had happened",
			"Panda??",
			"Paper > All ",
			"Grab a pencil and a pigeon",
			"asm()",
	};

	size_t n = rand()%(sizeof(paFunnyMessages)/sizeof(paFunnyMessages[0]));
	str_format(aBuf, sizeof(aBuf), "AllTheHaxx Crash  --  %s", paFunnyMessages[n]);
	gui_messagebox(aBuf, aMsg);
	printf("\nexiting\n");
	exit(0xEE00|sig);
}