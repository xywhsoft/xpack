#ifndef XPKGUI_COMPAT_UNISTD_H
#define XPKGUI_COMPAT_UNISTD_H

#include <windows.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef SSIZE_T ssize_t;
typedef unsigned int useconds_t;

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef F_OK
#define F_OK 0
#endif
#ifndef X_OK
#define X_OK 1
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef R_OK
#define R_OK 4
#endif

#define access _access
#define close _close
#define read _read
#define write _write
#define lseek _lseeki64
#define unlink _unlink
#define getcwd _getcwd
#define chdir _chdir
#define rmdir _rmdir
#define isatty _isatty
#define getpid _getpid

static __inline int usleep(useconds_t usec)
{
	Sleep((DWORD)((usec + 999u) / 1000u));
	return 0;
}

static __inline unsigned int sleep(unsigned int seconds)
{
	Sleep(seconds * 1000u);
	return 0;
}

static __inline int pipe(int handles[2])
{
	return _pipe(handles, 4096, _O_BINARY);
}

#endif
