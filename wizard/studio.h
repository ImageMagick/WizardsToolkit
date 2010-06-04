/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit methods.
*/
#ifndef _WIZARDSTOOLKIT_STUDIO_H
#define _WIZARDSTOOLKIT_STUDIO_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(WIN32) || defined(WIN64)
#  define WIZARDSTOOLKIT_WINDOWS_SUPPORT
#if defined(_WIN64) 
 typedef __int64 ssize_t; 
#else 
 typedef int ssize_t;
#endif
#else
#  define WIZARDSTOOLKIT_POSIX_SUPPORT
#endif

#if !defined(_WIZARDSTOOLKIT_CONFIG_H)
# define _WIZARDSTOOLKIT_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "wizard/wizard-config.h"
# else
#  include "wizard-config.h"
# endif
#if defined(_WIZARDSTOOLKIT__FILE_OFFSET_BITS) && !defined(_FILE_OFFSET_BITS)
# define _FILE_OFFSET_BITS _WIZARDSTOOLKIT__FILE_OFFSET_BITS
#endif
#if defined(_wizardstoolkit_const) && !defined(const)
# define const  _wizardstoolkit_const
#endif
#if defined(_wizardstoolkit_inline) && !defined(inline)
# define inline  _wizardstoolkit_inline
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
# if defined(_magickcore_restrict) && !defined(restrict)
#  define restrict  _wizardstoolkit_restrict
# endif
#endif

#if !defined(const)
#  define STDC
#endif

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(__CYGWIN__) && !defined(__MINGW32__)
# if defined(_MT) && defined(_DLL) && !defined(_WIZARDDLL_) && !defined(_LIB)
#  define _WIZARDDLL_
# endif
# if defined(_WIZARDDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_WIZARDLIB_)
#   define WizardExport  __declspec(dllimport)
#   if defined(_VISUALC_)
#    pragma message( "Wizard lib DLL import interface" )
#   endif
#  else
#   define WizardExport  __declspec(dllexport)
#   if defined(_VISUALC_)
#    pragma message( "Wizard lib DLL export interface" )
#   endif
#  endif
# else
#  define WizardExport
#  if defined(_VISUALC_)
#   pragma message( "Wizard lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#  define ModuleExport  __declspec(dllexport)
#  if defined(_VISUALC_)
#   pragma message( "Wizard module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "Wizard module static interface" )
#  endif

# endif
# define WizardGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4068)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# define WizardExport
# define ModuleExport
# define WizardGlobal
#endif
#define WizardSignature  0xabacadabUL
#if !defined(MaxTextExtent)
# define MaxTextExtent  4096
#endif

#include <stdarg.h>
#include <stdio.h>
#if defined(WIZARDSTOOLKIT_HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif
#if defined(WIZARDSTOOLKIT_STDC_HEADERS)
# include <stdlib.h>
# include <stddef.h>
#else
# if defined(WIZARDSTOOLKIT_HAVE_STDLIB_H)
#  include <stdlib.h>
# endif
#endif
#if defined(WIZARDSTOOLKIT_HAVE_STRING_H)
# if !defined(STDC_HEADERS) && defined(WIZARDSTOOLKIT_HAVE_MEMORY_H)
#  include <memory.h>
# endif
# include <string.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_STRINGS_H)
# include <strings.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_STDINT_H)
# include <stdint.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
# include <direct.h>
# if !defined(WIZARDSTOOLKIT_HAVE_STRERROR)
#  define HAVE_STRERROR
# endif
#endif

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
# include <direct.h>
# if !defined(WIZARDSTOOLKIT_HAVE_STRERROR)
#  define HAVE_STRERROR
# endif
#endif

#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <assert.h>

#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
#  include <pthread.h>
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#  define WIZARDSTOOLKIT_HAVE_WINTHREADS  1
#include <windows.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_SYS_SYSLIMITS_H)
#  include <sys/syslimits.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_ARM_LIMITS_H)
#  include <arm/limits.h>
#endif
#if defined(_OPENMP) && (_OPENMP >= 200203)
#  include <omp.h>
#  define WIZARDSTOOLKIT_HAVE_OPENMP 1
#endif

#if defined(WIZARDSTOOLKIT_HAVE_PREAD) && defined(WIZARDSTOOLKIT_HAVE_DECL_PREAD) && !WIZARDSTOOLKIT_HAVE_DECL_PREAD
ssize_t pread(int,void *,size_t,off_t);
#endif

#if defined(WIZARDSTOOLKIT_HAVE_PWRITE) && defined(WIZARDSTOOLKIT_HAVE_DECL_PWRITE) && !WIZARDSTOOLKIT_HAVE_DECL_PWRITE
ssize_t pwrite(int,const void *,size_t,off_t);
#endif

#if defined(WIZARDSTOOLKIT_HAVE_STRLCPY) && defined(WIZARDSTOOLKIT_HAVE_DECL_STRLCPY) && !WIZARDSTOOLKIT_HAVE_DECL_STRLCPY
extern size_t strlcpy(char *,const char *,size_t);
#endif

#if defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF) && defined(WIZARDSTOOLKIT_HAVE_DECL_VSNPRINTF) && !WIZARDSTOOLKIT_HAVE_DECL_VSNPRINTF
extern int vsnprintf(char *,size_t,const char *,va_list);
#endif

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) || defined(WIZARDSTOOLKIT_POSIX_SUPPORT)
# include <sys/types.h>
# include <sys/stat.h>
# if defined(WIZARDSTOOLKIT_HAVE_FTIME)
# include <sys/timeb.h>
# endif
# if defined(WIZARDSTOOLKIT_POSIX_SUPPORT)
#  if defined(WIZARDSTOOLKIT_HAVE_SYS_NDIR_H) || defined(WIZARDSTOOLKIT_HAVE_SYS_DIR_H) || defined(WIZARDSTOOLKIT_HAVE_NDIR_H)
#   define dirent direct
#   define NAMLEN(dirent) (dirent)->d_namlen
#   if defined(WIZARDSTOOLKIT_HAVE_SYS_NDIR_H)
#    include <sys/ndir.h>
#   endif
#   if defined(WIZARDSTOOLKIT_HAVE_SYS_DIR_H)
#    include <sys/dir.h>
#   endif
#   if defined(WIZARDSTOOLKIT_HAVE_NDIR_H)
#    include <ndir.h>
#   endif
#  else
#   include <dirent.h>
#   define NAMLEN(dirent) strlen((dirent)->d_name)
#  endif
#  include <pwd.h>
# endif
# if !defined(S_ISDIR)
#  define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
# endif
# if !defined(S_ISREG)
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
# include "wizard/wizard-type.h"
# if !defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#  include <sys/time.h>
#if defined(WIZARDSTOOLKIT_HAVE_SYS_TIMES_H)
#  include <sys/times.h>
#endif
#if defined(WIZARDSTOOLKIT_HAVE_SYS_RESOURCE_H)
#  include <sys/resource.h>
#endif
# endif
#else
# include <types.h>
# include <stat.h>
# if defined(macintosh)
#  include <SIOUX.h>
#  include <console.h>
#  include <unix.h>
# endif
# include "wizard/wizard-type.h"
#endif

#if defined(WIZARDSTOOLKIT_HAVE___ATTRIBUTE__)
#  define wizard_aligned(x)  __attribute__((aligned(x)))
#  define wizard_attribute  __attribute__
#  define wizard_unused(x)  wizard_unused_ ## x __attribute__((unused))
#else
#  define wizard_attribute(x)  /* nothing */
#  define wizard_unused(x) x
#endif

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
# include "wizard/nt-base.h"
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#endif
#if defined(macintosh)
# include "mac.h"
#endif
#if defined(vms)
# include "vms.h"
#endif

#undef HAVE_CONFIG_H
#undef index
#undef pipe

#include <wizard/string_.h>

/*
  Review these platform specific definitions.
*/
#if defined(WIZARDSTOOLKIT_POSIX_SUPPORT)
# define DirectorySeparator  "/"
# define DirectoryListSeparator  ':'
# define Exit  _exit
# define IsBasenameSeparator(c)  ((c) == '/' ? WizardTrue : WizardFalse)
# define PreferencesDefaults  "~/."
# define ProcessPendingEvents(text)
# define ReadCommandlLine(argc,argv)
# define SetNotifyHandlers
#else

# if defined(vms)
#  define ApplicationDefaults  "decw$system_defaults:"
#  define DirectorySeparator  ""
#  define DirectoryListSeparator  ';'
#  define Exit  exit
#  define IsBasenameSeparator(c) \
  (((c) == ']') || ((c) == ':') || ((c) == '/') ? WizardTrue : WizardFalse)
#  define WizardLibPath  "sys$login:"
#  define WizardModulesPath  "sys$login:"
#  define WizardSignaturerePath  "sys$login:"
#  define PreferencesDefaults  "decw$user_defaults:"
#  define ProcessPendingEvents(text)
#  define ReadCommandlLine(argc,argv)
#  define SetNotifyHandlers
# endif

# if defined(macintosh)
#  define ApplicationDefaults  "/usr/lib/X11/app-defaults/"
#  define DirectorySeparator  ":"
#  define DirectoryListSeparator  ';'
#  define IsBasenameSeparator(c)  ((c) == ':' ? WizardTrue : WizardFalse)
#  define WizardLibPath  ""
#  define WizardModulesPath  ""
#  define WizardSignaturerePath  ""
#  define PreferencesDefaults  "~/."
#  define ReadCommandlLine(argc,argv)  argc=ccommand(argv); puts(WizardVersion);
#  define SetNotifyHandlers \
    SetErrorHandler(MACErrorHandler); \
    SetWarningHandler(MACWarningHandler)
# endif

# if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#  define DirectorySeparator  "\\"
#  define DirectoryListSeparator  ';'
#  define IsBasenameSeparator(c) \
  (((c) == '/') || ((c) == '\\') ? WizardTrue : WizardFalse)
#  define ProcessPendingEvents(text)
#if !defined(PreferencesDefaults)
#  define PreferencesDefaults  "~\\."
#endif /* PreferencesDefaults */
#  define ReadCommandlLine(argc,argv)
#  define SetNotifyHandlers \
    SetErrorHandler(NTErrorHandler); \
    SetWarningHandler(NTWarningHandler)
#  undef sleep
#  define sleep(seconds)  Sleep(seconds*1000)
#  if !defined(WIZARDSTOOLKIT_HAVE_TIFFCONF_H)
#    define HAVE_TIFFCONF_H
#  endif
# endif

#endif

/*
  Define declarations.
*/
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))
#define WizardEpoch  1000000000L
#define Max(x,y)  (((x) > (y)) ? (x) : (y))
#define Min(x,y)  (((x) < (y)) ? (x) : (y))
#define Swap(x,y) ((x)^=(y), (y)^=(x), (x)^=(y))

/*
  Define system symbols if not already defined.
*/
#if !defined(STDIN_FILENO)
#define STDIN_FILENO  0x00
#endif

#if !defined(STDOUT_FILENO)
#define STDOUT_FILENO  0x01
#endif

#if !defined(STDERR_FILENO)
#define STDERR_FILENO  0x02
#endif

#if !defined(O_BINARY)
#define O_BINARY  0x00
#endif

#if defined(S_IRUSR) && defined(S_IWUSR)
# define S_MODE (S_IRUSR | S_IWUSR)
#elif defined (WIZARDSTOOLKIT_WINDOWS_SUPPORT)
# define S_MODE (_S_IREAD | _S_IWRITE)
#else
# define S_MODE  0600
#endif

#if !defined(PATH_MAX)
#define PATH_MAX  4096
#endif

/*
  I/O defines.
*/
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(Windows95) && defined(_VISUALC_)
#define WizardSeek(file,offset,whence)  _lseeki64(file,offset,whence)
#else
#define WizardSeek(file,offset,whence)  lseek(file,offset,whence)
#endif

extern unsigned char
  _wizard_info[1];

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
