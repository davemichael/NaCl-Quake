#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifndef __WIN32__
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
//#include <sys/mman.h>
#endif

#include "quakedef.h"

//#define HAVE_SDL_DELAY
//#define HAVE_MKDIR

qboolean			isDedicated;

int noconinput = 0;

char *basedir = ".";
char *cachedir = "";

cvar_t  sys_linerefresh = {"sys_linerefresh","0"};// set for entity display
cvar_t  sys_nostdout = {"sys_nostdout","0"};

extern int nacl_file_length(int fd);
extern int nacl_file_write(int, void*, size_t);

// =======================================================================
// General routines
// =======================================================================

void Sys_DebugNumber(int y, int val)
{
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];
	
	va_start (argptr,fmt);
	vsprintf (text,fmt,argptr);
	va_end (argptr);
	fprintf(stderr, "%s", text);
	
	//Con_Print (text);
}

void Sys_Quit (void)
{
	Host_Shutdown();
	exit(0);
}

void Sys_Init(void)
{
#if id386
	Sys_SetFPCW();
#endif
}

#if !id386

/*
================
Sys_LowFPPrecision
================
*/
void Sys_LowFPPrecision (void)
{
// causes weird problems on Nextstep
}


/*
================
Sys_HighFPPrecision
================
*/
void Sys_HighFPPrecision (void)
{
// causes weird problems on Nextstep
}

#endif	// !id386


void Sys_Error (char *error, ...)
{ 
    va_list     argptr;
    char        string[1024];

    va_start (argptr,error);
    vsprintf (string,error,argptr);
    va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);

	Host_Shutdown ();
	exit (1);

} 

void Sys_Warn (char *warning, ...)
{ 
    va_list     argptr;
    char        string[1024];
    
    va_start (argptr,warning);
    vsprintf (string,warning,argptr);
    va_end (argptr);
	fprintf(stderr, "Warning: %s", string);
} 

/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_FileOpenRead (char *path, int *hndl)
{
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		*hndl = -1;
		return -1;
	}
	*hndl = fd; 
	return nacl_file_length(fd);
}

int Sys_FileOpenWrite (char *path)
{
	return open(path, O_WRONLY|O_APPEND|O_CREAT);
}

void Sys_FileClose (int handle)
{
	close(handle);
}

void Sys_FileSeek (int handle, int position)
{
	if ( handle >= 0 ) {
		lseek (handle, position, SEEK_SET);
	}
}

int Sys_FileRead (int handle, void *dst, int count)
{
	return read(handle, dst, count);
}

int Sys_FileWrite (int handle, void *src, int count)
{
	// UNTESTED, probably doesn't work.  At best, saves in a pretend file
	// in memory.
	return nacl_file_write(handle, src, count);
}

int	Sys_FileTime (char *path)
{
	FILE	*f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
#if defined (HAVE_MKDIR)

#ifdef __WIN32__
    mkdir (path);
#else
    mkdir (path, 0777);
#endif

#endif
}

void Sys_DebugLog(char *file, char *fmt, ...)
{
    va_list argptr; 
    static char data[1024];
    FILE *fp;
    
    va_start(argptr, fmt);
    vsprintf(data, fmt, argptr);
    va_end(argptr);
    fp = fopen(file, "a");
    fwrite(data, strlen(data), 1, fp);
    fclose(fp);
}

double Sys_FloatTime (void)
{
#ifdef __WIN32__

	static int starttime = 0;

	if ( ! starttime )
		starttime = clock();

	return (clock()-starttime)*1.0/1024;

#else

    struct timeval tp;
    struct timezone tzp; 
    static int      secbase; 
    
    gettimeofday(&tp, &tzp);  

    if (!secbase)
    {
        secbase = tp.tv_sec;
        return tp.tv_usec/1000000.0;
    }

    return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;

#endif
}

// =======================================================================
// Sleeps for microseconds
// =======================================================================

static volatile int oktogo;

void alarm_handler(int x)
{
	oktogo=1;
}

byte *Sys_ZoneBase (int *size)
{

	char *QUAKEOPT = getenv("QUAKEOPT");

	*size = 0xc00000;
	if (QUAKEOPT)
	{
		while (*QUAKEOPT)
			if (tolower(*QUAKEOPT++) == 'm')
			{
				*size = atof(QUAKEOPT) * 1024*1024;
				break;
			}
	}
	return malloc (*size);

}

void Sys_LineRefresh(void)
{
}

void Sys_Sleep(void)
{
#if defined(HAVE_SDL_DELAY)
	SDL_Delay(1);
#else
	extern int sched_yield(void);
	sched_yield();
#if 0
        {
          int i;
	  for (i = 0; i < 10000; i++) ;
        }
#endif
#endif
}

void floating_point_exception_handler(int whatever)
{
//	Sys_Warn("floating point exception\n");
	signal(SIGFPE, floating_point_exception_handler);
}

void moncontrol(int x)
{
}

extern int quake_main (int c, char **v)
{

	double		time, oldtime, newtime;
	quakeparms_t parms;
	extern int vcrFile;
	extern int recording;
	static int frame;

	moncontrol(0);

//	signal(SIGFPE, floating_point_exception_handler);
	signal(SIGFPE, SIG_IGN);

	parms.memsize = 32*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = basedir;
	parms.cachedir = cachedir;

	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;

	Sys_Init();

    Host_Init(&parms);

	Cvar_RegisterVariable (&sys_nostdout);

    oldtime = Sys_FloatTime () - 0.1;
    while (1)
    {
// find time spent rendering last frame
        newtime = Sys_FloatTime ();
        time = newtime - oldtime;

        if (cls.state == ca_dedicated)
        {   // play vcrfiles at max speed
            if (time < sys_ticrate.value && (vcrFile == -1 || recording) )
            {
                Sys_Sleep ();
                continue;       // not time to run a server only tic yet
            }
            time = sys_ticrate.value;
        }

        if (time > sys_ticrate.value*2)
            oldtime = newtime;
        else
            oldtime += time;

        if (++frame > 10)
            moncontrol(1);      // profile only while we do each Quake frame
        Host_Frame (time);
        moncontrol(0);

// graphic debugging aids
        if (sys_linerefresh.value)
            Sys_LineRefresh ();
    }

}


#if 0

/*
 * we certainly can't have this in NaCl(!)  Fortunately it doesn't appear to be used anymore.
 */

/*
================
Sys_MakeCodeWriteable
================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{

	int r;
	unsigned long addr;
	int psize = getpagesize();

	fprintf(stderr, "writable code %lx-%lx\n", startaddr, startaddr+length);

	addr = startaddr & ~(psize-1);

	r = mprotect((char*)addr, length + startaddr - addr, 7);

	if (r < 0)
    		Sys_Error("Protection change failed\n");

}

#endif
