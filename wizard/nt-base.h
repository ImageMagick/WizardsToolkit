/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Windows NT utility methods for the Wizard's Toolkit.
*/
#ifndef _WIZARDSTOOLKIT_NTBASE_H
#define _WIZARDSTOOLKIT_NTBASE_H

#include "wizard/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <winuser.h>
#include <wingdi.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#if defined(_DEBUG)
#include <crtdbg.h>
#endif

#include <stdarg.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define PROT_READ  0x01
#define PROT_WRITE  0x02
#define MAP_SHARED  0x01
#define MAP_PRIVATE  0x02
#define MAP_ANONYMOUS  0x20
#define F_OK 0
#define R_OK 4
#define W_OK 2
#define RW_OK 6
#define _SC_PAGESIZE 1
#define _SC_PHYS_PAGES 2
#define _SC_OPEN_MAX 3
#if !defined(SSIZE_MAX)
#define SSIZE_MAX  0x7fffffffL
#endif

/*
  _MSC_VER values:
    1100 MSVC 5.0
    1200 MSVC 6.0
    1300 MSVC 7.0 Visual C++ .NET 2002
    1310 Visual c++ .NET 2003
    1400 Visual C++ 2005
    1500 Visual C++ 2008
*/

#if !defined(chsize)
# if defined(__BORLANDC__)
#   define chsize(file,length)  chsize(file,length)
# else
#   define chsize(file,length)  _chsize(file,length)
# endif
#endif

#if !defined(access)
#if defined(_VISUALC_) && (_MSC_VER >= 1400)
#  define access(path,mode)  _access_s(path,mode)
#endif
#endif
#if !defined(chdir)
#  define chdir  _chdir
#endif
#if !defined(close)
#  define close  _close
#endif
#if !defined(closedir)
#  define closedir(directory)  NTCloseDirectory(directory)
#endif
#if !defined(fdopen)
#  define fdopen  _fdopen
#endif
#if !defined(fileno)
#  define fileno  _fileno
#endif
#if !defined(freelocale)
#  define freelocale  _free_locale
#endif
#if !defined(fseek) && !defined(__MINGW32__) && !defined(__MINGW64__)
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#  define fseek  _fseeki64
#endif
#endif
#if !defined(fstat) && !defined(__BORLANDC__)
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#  define fstat  _fstati64
#else
#  define fstat  _fstat
#endif
#endif
#if !defined(fsync)
#  define fsync  _commit
#endif
#if !defined(ftell) && !defined(__MINGW32__) && !defined(__MINGW64__)
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#  define ftell  _ftelli64
#endif
#endif
#if !defined(ftruncate)
#  define ftruncate(file,length)  NTTruncateFile(file,length)
#endif
#if !defined(getcwd)
#  define getcwd  _getcwd
#endif
#if !defined(getpid)
#  define getpid  _getpid
#endif
#if !defined(hypot)
#  define hypot  _hypot
#endif
#if !defined(inline)
#  define inline __inline
#endif
#if !defined(isatty)
#  define isatty _isatty
#endif
#if !defined(locale_t)
#define locale_t _locale_t
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#if !defined(lseek)
#  define lseek  _lseeki64
#endif
#else
#if !defined(lseek)
#  define lseek  _lseek
#endif
#endif
#if !defined(WIZARDSTOOLKIT_LTDL_DELEGATE)
#if !defined(lt_dlclose)
#  define lt_dlclose(handle)  NTCloseLibrary(handle)
#endif
#if !defined(lt_dlerror)
#  define lt_dlerror()  NTGetLibraryError()
#endif
#if !defined(lt_dlexit)
#  define lt_dlexit()  NTExitLibrary()
#endif
#if !defined(lt_dlinit)
#  define lt_dlinit()  NTInitializeLibrary()
#endif
#if !defined(lt_dlopen)
#  define lt_dlopen(filename)  NTOpenLibrary(filename)
#endif
#if !defined(lt_dlsetsearchpath)
#  define lt_dlsetsearchpath(path)  NTSetSearchPath(path)
#endif
#if !defined(lt_dlsym)
#  define lt_dlsym(handle,name)  NTGetLibrarySymbol(handle,name)
#endif
#endif
#if !defined(mkdir)
#  define mkdir  _mkdir
#endif
#if !defined(mmap)
#  define mmap(address,length,protection,access,file,offset) \
  NTMapMemory(address,length,protection,access,file,offset)
#endif
#if !defined(msync)
#  define msync(address,length,flags)  NTSyncMemory(address,length,flags)
#endif
#if !defined(munmap)
#  define munmap(address,length)  NTUnmapMemory(address,length)
#endif
#if !defined(opendir)
#  define opendir(directory)  NTOpenDirectory(directory)
#endif
#if !defined(open)
#  define open  _open
#endif
#if !defined(pclose)
#  define pclose  _pclose
#endif
#if !defined(popen)
#  define popen  _popen
#endif
#if !defined(fprintf_l)
#define fprintf_l  _fprintf_s_l
#endif
#if !defined(read)
#  define read  _read
#endif
#if !defined(readdir)
#  define readdir(directory)  NTReadDirectory(directory)
#endif
#if !defined(seekdir)
#  define seekdir(directory,offset)  NTSeekDirectory(directory,offset)
#endif
#if !defined(setmode)
#  define setmode  _setmode
#endif
#if !defined(spawnvp)
#  define spawnvp  _spawnvp
#endif
#if !defined(stat) && !defined(__BORLANDC__)
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#  define stat  _stati64
#else
#  define stat  _stat
#endif
#endif
#if !defined(strcasecmp)
#  define strcasecmp  _stricmp
#endif
#if !defined(strncasecmp)
#  define strncasecmp  _strnicmp
#endif
#if !defined(strtod_l)
#define strtod_l  _strtod_l
#endif
#if !defined(sysconf)
#  define sysconf(name)  NTSystemConfiguration(name)
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#  define tell  _telli64
#else
#  define tell  _tell
#endif
#if !defined(telldir)
#  define telldir(directory)  NTTellDirectory(directory)
#endif
#if !defined(tempnam)
#  define tempnam  _tempnam_s
#endif
#if !defined(vfprintf_l)
#define vfprintf_l  _vfprintf_l
#endif
#if !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1500)
#define vsnprintf _vsnprintf 
#endif
#if !defined(vsnprintf_l)
#define vsnprintf_l  _vsnprintf_l
#endif
#if !defined(write)
#  define write  _write
#endif
#if !defined(wstat) && !defined(__BORLANDC__)
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && \
  !(defined(_MSC_VER) && (_MSC_VER < 1400)) && \
  !(defined(__MSVCRT_VERSION__) && (__MSVCRT_VERSION__ < 0x800))
#  define wstat  _wstati64
#else
#  define wstat  _wstat
#endif
#endif

#if defined(_MT) && defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#  define SAFE_GLOBAL  __declspec(thread)
#else
#  define SAFE_GLOBAL
#endif

#if defined(__BORLANDC__)
#undef _O_RANDOM
#define _O_RANDOM 0
#undef _O_SEQUENTIAL
#define _O_SEQUENTIAL 0
#undef _O_SHORT_LIVED
#define _O_SHORT_LIVED 0
#undef _O_TEMPORARY
#define _O_TEMPORARY 0
#endif

#if !defined(XS_VERSION)
struct dirent
{
  char
    d_name[2048];

  int
    d_namlen;
};

typedef struct _DIR
{
  HANDLE
    hSearch;

  WIN32_FIND_DATA
    Win32FindData;

  BOOL
    firsttime;

  struct dirent
    file_info;
} DIR;

typedef struct _NTMEMORYSTATUSEX
{
  DWORD
    dwLength,
    dwMemoryLoad;

  DWORDLONG
    ullTotalPhys,
    ullAvailPhys,
    ullTotalPageFile,
    ullAvailPageFile,
    ullTotalVirtual,
    ullAvailVirtual,
    ullAvailExtendedVirtual;
} NTMEMORYSTATUSEX;

#if !defined(__MINGW32__) && !defined(__MINGW64__)
struct timezone
{
  int
    tz_minuteswest,
    tz_dsttime;
};

typedef _locale_t locale_t;
#endif

typedef UINT
  (CALLBACK *LPFNDLLFUNC1)(DWORD,UINT);

typedef UINT
  (CALLBACK *LPFNDLLFUNC2)(NTMEMORYSTATUSEX *);

#endif

#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
#  if defined(_WIN32)
#    define BZ_IMPORT 1
#  endif
#endif

extern WizardExport char
  *NTGetLastError(void);

#if !defined(WIZARDSTOOLKIT_LTDL_DELEGATE)
extern WizardExport const char
  *NTGetLibraryError(void);
#endif

extern WizardExport char
  **NTArgvToUTF8(const int argc,wchar_t **),
  *NTRegistryKeyLookup(const char *);

extern WizardExport DIR
  *NTOpenDirectory(const char *);

extern WizardExport double
  NTElapsedTime(void),
  NTUserTime(void);

extern WizardExport int
  Exit(int),
#if !defined(__MINGW32__) && !defined(__MINGW64__)
  gettimeofday(struct timeval *,struct timezone *),
#endif
  IsWindows95(),
  NTCloseDirectory(DIR *),
  NTCloseLibrary(void *),
  NTControlHandler(void),
  NTExitLibrary(void),
  NTInitializeLibrary(void),
  NTSetSearchPath(const char *),
  NTSyncMemory(void *,size_t,int),
  NTUnmapMemory(void *,size_t),
  NTSystemCommand(const char *);

extern WizardExport ssize_t
  NTSystemConfiguration(int),
  NTTellDirectory(DIR *);

extern WizardExport WizardBooleanType
  NTGetExecutionPath(char *,const size_t),
  NTGetModulePath(const char *,char *),
  NTGatherRandomData(const size_t,unsigned char *),
  NTReportEvent(const char *,const WizardBooleanType),
  NTReportException(const char *,const WizardBooleanType);

extern WizardExport struct dirent
  *NTReadDirectory(DIR *);

extern WizardExport unsigned char
  *NTResourceToBlob(const char *);

extern WizardExport void
  NTErrorHandler(const ExceptionType,const char *,const char *),
  *NTGetLibrarySymbol(void *,const char *),
  *NTMapMemory(char *,size_t,int,int,int,WizardOffsetType),
  *NTOpenLibrary(const char *),
  NTSeekDirectory(DIR *,ssize_t),
  NTWarningHandler(const ExceptionType,const char *,const char *);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* !C++ */

#endif /* !_WIZARDSTOOLKIT_NTBASE_H */
