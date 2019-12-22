#ifndef _WIZARD_WIZARD_CONFIG_H
#define _WIZARD_WIZARD_CONFIG_H 1
 
/* wizard/wizard-config.h. Generated automatically at end of configure. */
/* config/config.h.  Generated from config.h.in by configure.  */
/* config/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define if you have the bzip2 library */
#ifndef WIZARDSTOOLKIT_BZLIB_DELEGATE
#define WIZARDSTOOLKIT_BZLIB_DELEGATE 1
#endif

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* Directory where architecture-dependent configuration files live. */
#ifndef WIZARDSTOOLKIT_CONFIGURE_PATH
#define WIZARDSTOOLKIT_CONFIGURE_PATH "/usr/local/etc/WizardsToolkit-1/"
#endif

/* Subdirectory of lib where architecture-dependent configuration files live.
   */
#ifndef WIZARDSTOOLKIT_CONFIGURE_RELATIVE_PATH
#define WIZARDSTOOLKIT_CONFIGURE_RELATIVE_PATH "WizardsToolkit-1"
#endif

/* "Directory where the Wizard's Toolkit documents live." */
#ifndef WIZARDSTOOLKIT_DOCUMENTATION_PATH
#define WIZARDSTOOLKIT_DOCUMENTATION_PATH "/usr/local/share/doc/WizardsToolkit-1/"
#endif

/* Enable Wizard Executable Environment debugging */
#ifndef WIZARDSTOOLKIT_Debug
#define WIZARDSTOOLKIT_Debug 1
#endif

/* Build self-contained, embeddable, zero-configuration Wizards Toolkit
   (experimental) */
/* #undef EMBEDDABLE_SUPPORT */

/* Directory where executables are installed. */
#ifndef WIZARDSTOOLKIT_EXECUTABLE_PATH
#define WIZARDSTOOLKIT_EXECUTABLE_PATH "/usr/local/bin/"
#endif

/* Define to 1 if you have the `alarm' function. */
#ifndef WIZARDSTOOLKIT_HAVE_ALARM
#define WIZARDSTOOLKIT_HAVE_ALARM 1
#endif

/* Define to 1 if you have the <argz.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_ARGZ_H
#define WIZARDSTOOLKIT_HAVE_ARGZ_H 1
#endif

/* Define to 1 if you have the <arm/limits.h> header file. */
/* #undef HAVE_ARM_LIMITS_H */

/* Define to 1 if you have the `atexit' function. */
#ifndef WIZARDSTOOLKIT_HAVE_ATEXIT
#define WIZARDSTOOLKIT_HAVE_ATEXIT 1
#endif

/* define if bool is a built-in type */
/* #undef HAVE_BOOL */

/* Define to 1 if you have the `clock' function. */
#ifndef WIZARDSTOOLKIT_HAVE_CLOCK
#define WIZARDSTOOLKIT_HAVE_CLOCK 1
#endif

/* Define to 1 if you have clock_gettime. */
#ifndef WIZARDSTOOLKIT_HAVE_CLOCK_GETTIME
#define WIZARDSTOOLKIT_HAVE_CLOCK_GETTIME 1
#endif

/* Define to 1 if clock_gettime supports CLOCK_REALTIME. */
/* #undef HAVE_CLOCK_REALTIME */

/* Define to 1 if you have the declaration of `pread', and to 0 if you don't.
   */
#ifndef WIZARDSTOOLKIT_HAVE_DECL_PREAD
#define WIZARDSTOOLKIT_HAVE_DECL_PREAD 1
#endif

/* Define to 1 if you have the declaration of `pwrite', and to 0 if you don't.
   */
#ifndef WIZARDSTOOLKIT_HAVE_DECL_PWRITE
#define WIZARDSTOOLKIT_HAVE_DECL_PWRITE 1
#endif

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#ifndef WIZARDSTOOLKIT_HAVE_DECL_STRERROR_R
#define WIZARDSTOOLKIT_HAVE_DECL_STRERROR_R 1
#endif

/* Define to 1 if you have the declaration of `strlcpy', and to 0 if you
   don't. */
#ifndef WIZARDSTOOLKIT_HAVE_DECL_STRLCPY
#define WIZARDSTOOLKIT_HAVE_DECL_STRLCPY 0
#endif

/* Define to 1 if you have the declaration of `vsnprintf', and to 0 if you
   don't. */
#ifndef WIZARDSTOOLKIT_HAVE_DECL_VSNPRINTF
#define WIZARDSTOOLKIT_HAVE_DECL_VSNPRINTF 1
#endif

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#ifndef WIZARDSTOOLKIT_HAVE_DIRENT_H
#define WIZARDSTOOLKIT_HAVE_DIRENT_H 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_DLFCN_H
#define WIZARDSTOOLKIT_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the `fchmod' function. */
#ifndef WIZARDSTOOLKIT_HAVE_FCHMOD
#define WIZARDSTOOLKIT_HAVE_FCHMOD 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_FCNTL_H
#define WIZARDSTOOLKIT_HAVE_FCNTL_H 1
#endif

/* Define to 1 if you have the `fprintf_l' function. */
/* #undef HAVE_FPRINTF_L */

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#ifndef WIZARDSTOOLKIT_HAVE_FSEEKO
#define WIZARDSTOOLKIT_HAVE_FSEEKO 1
#endif

/* Define to 1 if you have the `ftime' function. */
#ifndef WIZARDSTOOLKIT_HAVE_FTIME
#define WIZARDSTOOLKIT_HAVE_FTIME 1
#endif

/* Define to 1 if you have the `getcwd' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GETCWD
#define WIZARDSTOOLKIT_HAVE_GETCWD 1
#endif

/* Define to 1 if you have the `getdtablesize' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GETDTABLESIZE
#define WIZARDSTOOLKIT_HAVE_GETDTABLESIZE 1
#endif

/* Define to 1 if you have the `getexecname' function. */
/* #undef HAVE_GETEXECNAME */

/* Define to 1 if you have the `getpagesize' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GETPAGESIZE
#define WIZARDSTOOLKIT_HAVE_GETPAGESIZE 1
#endif

/* Define to 1 if you have the `getpid' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GETPID
#define WIZARDSTOOLKIT_HAVE_GETPID 1
#endif

/* Define to 1 if you have the `getrusage' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GETRUSAGE
#define WIZARDSTOOLKIT_HAVE_GETRUSAGE 1
#endif

/* Define to 1 if you have the `gettimeofday' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GETTIMEOFDAY
#define WIZARDSTOOLKIT_HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the `gmtime_r' function. */
#ifndef WIZARDSTOOLKIT_HAVE_GMTIME_R
#define WIZARDSTOOLKIT_HAVE_GMTIME_R 1
#endif

/* Compile with hugepage support */
/* #undef HAVE_HUGEPAGES */

/* Define to 1 if the system has the type `intmax_t'. */
#ifndef WIZARDSTOOLKIT_HAVE_INTMAX_T
#define WIZARDSTOOLKIT_HAVE_INTMAX_T 1
#endif

/* Define to 1 if the system has the type `intptr_t'. */
#ifndef WIZARDSTOOLKIT_HAVE_INTPTR_T
#define WIZARDSTOOLKIT_HAVE_INTPTR_T 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_INTTYPES_H
#define WIZARDSTOOLKIT_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `isascii' function. */
#ifndef WIZARDSTOOLKIT_HAVE_ISASCII
#define WIZARDSTOOLKIT_HAVE_ISASCII 1
#endif

/* Define to 1 if you have the `isnan' function. */
#ifndef WIZARDSTOOLKIT_HAVE_ISNAN
#define WIZARDSTOOLKIT_HAVE_ISNAN 1
#endif

/* Define to 1 if you have the <limits.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_LIMITS_H
#define WIZARDSTOOLKIT_HAVE_LIMITS_H 1
#endif

/* Define to 1 if you have the <linux/unistd.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_LINUX_UNISTD_H
#define WIZARDSTOOLKIT_HAVE_LINUX_UNISTD_H 1
#endif

/* Define to 1 if you have the <locale.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_LOCALE_H
#define WIZARDSTOOLKIT_HAVE_LOCALE_H 1
#endif

/* Define to 1 if the system has the type `locale_t'. */
/* #undef HAVE_LOCALE_T */

/* Define to 1 if you have the `localtime_r' function. */
#ifndef WIZARDSTOOLKIT_HAVE_LOCALTIME_R
#define WIZARDSTOOLKIT_HAVE_LOCALTIME_R 1
#endif

/* Define to 1 if the system has the type `long double'. */
#ifndef WIZARDSTOOLKIT_HAVE_LONG_DOUBLE
#define WIZARDSTOOLKIT_HAVE_LONG_DOUBLE 1
#endif

/* Define to 1 if the type `long double' works and has more range or precision
   than `double'. */
#ifndef WIZARDSTOOLKIT_HAVE_LONG_DOUBLE_WIDER
#define WIZARDSTOOLKIT_HAVE_LONG_DOUBLE_WIDER 1
#endif

/* Define to 1 if the system has the type `long long int'. */
#ifndef WIZARDSTOOLKIT_HAVE_LONG_LONG_INT
#define WIZARDSTOOLKIT_HAVE_LONG_LONG_INT 1
#endif

/* Define to 1 if you have the `lstat' function. */
#ifndef WIZARDSTOOLKIT_HAVE_LSTAT
#define WIZARDSTOOLKIT_HAVE_LSTAT 1
#endif

/* Define to 1 if you have the <machine/param.h> header file. */
/* #undef HAVE_MACHINE_PARAM_H */

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
/* #undef HAVE_MACH_O_DYLD_H */

/* Define to 1 if you have the `madvise' function. */
#ifndef WIZARDSTOOLKIT_HAVE_MADVISE
#define WIZARDSTOOLKIT_HAVE_MADVISE 1
#endif

/* Define to 1 if you have the <malloc.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_MALLOC_H
#define WIZARDSTOOLKIT_HAVE_MALLOC_H 1
#endif

/* Define to 1 if <wchar.h> declares mbstate_t. */
#ifndef WIZARDSTOOLKIT_HAVE_MBSTATE_T
#define WIZARDSTOOLKIT_HAVE_MBSTATE_T 1
#endif

/* Define to 1 if you have the `memmove' function. */
#ifndef WIZARDSTOOLKIT_HAVE_MEMMOVE
#define WIZARDSTOOLKIT_HAVE_MEMMOVE 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_MEMORY_H
#define WIZARDSTOOLKIT_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
#ifndef WIZARDSTOOLKIT_HAVE_MEMSET
#define WIZARDSTOOLKIT_HAVE_MEMSET 1
#endif

/* Define to 1 if you have the `mkdir' function. */
#ifndef WIZARDSTOOLKIT_HAVE_MKDIR
#define WIZARDSTOOLKIT_HAVE_MKDIR 1
#endif

/* Define to 1 if you have the `mkstemp' function. */
#ifndef WIZARDSTOOLKIT_HAVE_MKSTEMP
#define WIZARDSTOOLKIT_HAVE_MKSTEMP 1
#endif

/* Define to 1 if you have a working `mmap' system call. */
#ifndef WIZARDSTOOLKIT_HAVE_MMAP
#define WIZARDSTOOLKIT_HAVE_MMAP 1
#endif

/* Define to 1 if you have a working `mmap' system call. */
#ifndef WIZARDSTOOLKIT_HAVE_MMAP_FILEIO
#define WIZARDSTOOLKIT_HAVE_MMAP_FILEIO 1
#endif

/* Define to 1 if you have the `munmap' function. */
#ifndef WIZARDSTOOLKIT_HAVE_MUNMAP
#define WIZARDSTOOLKIT_HAVE_MUNMAP 1
#endif

/* define if the compiler implements namespaces */
/* #undef HAVE_NAMESPACES */

/* Define if g++ supports namespace std. */
/* #undef HAVE_NAMESPACE_STD */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the `newloacle' function. */
/* #undef HAVE_NEWLOACLE */

/* Define to 1 if you have the `pclose' function. */
#ifndef WIZARDSTOOLKIT_HAVE_PCLOSE
#define WIZARDSTOOLKIT_HAVE_PCLOSE 1
#endif

/* Define to 1 if you have the `poll' function. */
#ifndef WIZARDSTOOLKIT_HAVE_POLL
#define WIZARDSTOOLKIT_HAVE_POLL 1
#endif

/* Define to 1 if you have the `popen' function. */
#ifndef WIZARDSTOOLKIT_HAVE_POPEN
#define WIZARDSTOOLKIT_HAVE_POPEN 1
#endif

/* Define to 1 if you have the `posix_fallocate' function. */
#ifndef WIZARDSTOOLKIT_HAVE_POSIX_FALLOCATE
#define WIZARDSTOOLKIT_HAVE_POSIX_FALLOCATE 1
#endif

/* Define to 1 if you have the `posix_memalign' function. */
#ifndef WIZARDSTOOLKIT_HAVE_POSIX_MEMALIGN
#define WIZARDSTOOLKIT_HAVE_POSIX_MEMALIGN 1
#endif

/* Define to 1 if you have the `pow' function. */
#ifndef WIZARDSTOOLKIT_HAVE_POW
#define WIZARDSTOOLKIT_HAVE_POW 1
#endif

/* Define to 1 if you have the `pread' function. */
#ifndef WIZARDSTOOLKIT_HAVE_PREAD
#define WIZARDSTOOLKIT_HAVE_PREAD 1
#endif

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* Define if you have POSIX threads libraries and header files. */
#ifndef WIZARDSTOOLKIT_HAVE_PTHREAD
#define WIZARDSTOOLKIT_HAVE_PTHREAD 1
#endif

/* Have PTHREAD_PRIO_INHERIT. */
#ifndef WIZARDSTOOLKIT_HAVE_PTHREAD_PRIO_INHERIT
#define WIZARDSTOOLKIT_HAVE_PTHREAD_PRIO_INHERIT 1
#endif

/* Define to 1 if you have the `pwrite' function. */
#ifndef WIZARDSTOOLKIT_HAVE_PWRITE
#define WIZARDSTOOLKIT_HAVE_PWRITE 1
#endif

/* Define to 1 if you have the `raise' function. */
#ifndef WIZARDSTOOLKIT_HAVE_RAISE
#define WIZARDSTOOLKIT_HAVE_RAISE 1
#endif

/* Define to 1 if you have the `readlink' function. */
#ifndef WIZARDSTOOLKIT_HAVE_READLINK
#define WIZARDSTOOLKIT_HAVE_READLINK 1
#endif

/* Define to 1 if you have the `realpath' function. */
#ifndef WIZARDSTOOLKIT_HAVE_REALPATH
#define WIZARDSTOOLKIT_HAVE_REALPATH 1
#endif

/* Define to 1 if you have the `sbrk' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SBRK
#define WIZARDSTOOLKIT_HAVE_SBRK 1
#endif

/* Define to 1 if you have the `select' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SELECT
#define WIZARDSTOOLKIT_HAVE_SELECT 1
#endif

/* Define to 1 if you have the `setlocale' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SETLOCALE
#define WIZARDSTOOLKIT_HAVE_SETLOCALE 1
#endif

/* Define to 1 if you have the `setvbuf' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SETVBUF
#define WIZARDSTOOLKIT_HAVE_SETVBUF 1
#endif

/* Define to 1 if you have the `sigaction' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SIGACTION
#define WIZARDSTOOLKIT_HAVE_SIGACTION 1
#endif

/* Define to 1 if you have the `sigemptyset' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SIGEMPTYSET
#define WIZARDSTOOLKIT_HAVE_SIGEMPTYSET 1
#endif

/* Define to 1 if you have the `spawnvp' function. */
/* #undef HAVE_SPAWNVP */

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if you have the <stdarg.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_STDARG_H
#define WIZARDSTOOLKIT_HAVE_STDARG_H 1
#endif

/* Define to 1 if stdbool.h conforms to C99. */
#ifndef WIZARDSTOOLKIT_HAVE_STDBOOL_H
#define WIZARDSTOOLKIT_HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_STDINT_H
#define WIZARDSTOOLKIT_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_STDLIB_H
#define WIZARDSTOOLKIT_HAVE_STDLIB_H 1
#endif

/* define if the compiler supports ISO C++ standard library */
/* #undef HAVE_STD_LIBS */

/* Define to 1 if you have the `strcasecmp' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRCASECMP
#define WIZARDSTOOLKIT_HAVE_STRCASECMP 1
#endif

/* Define to 1 if you have the `strchr' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRCHR
#define WIZARDSTOOLKIT_HAVE_STRCHR 1
#endif

/* Define to 1 if you have the `strcspn' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRCSPN
#define WIZARDSTOOLKIT_HAVE_STRCSPN 1
#endif

/* Define to 1 if you have the `strdup' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRDUP
#define WIZARDSTOOLKIT_HAVE_STRDUP 1
#endif

/* Define to 1 if you have the `strerror_r' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRERROR_R
#define WIZARDSTOOLKIT_HAVE_STRERROR_R 1
#endif

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#ifndef WIZARDSTOOLKIT_HAVE_STRINGIZE
#define WIZARDSTOOLKIT_HAVE_STRINGIZE 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_STRINGS_H
#define WIZARDSTOOLKIT_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_STRING_H
#define WIZARDSTOOLKIT_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strncasecmp' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRNCASECMP
#define WIZARDSTOOLKIT_HAVE_STRNCASECMP 1
#endif

/* Define to 1 if you have the `strrchr' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRRCHR
#define WIZARDSTOOLKIT_HAVE_STRRCHR 1
#endif

/* Define to 1 if you have the `strspn' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRSPN
#define WIZARDSTOOLKIT_HAVE_STRSPN 1
#endif

/* Define to 1 if you have the `strstr' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRSTR
#define WIZARDSTOOLKIT_HAVE_STRSTR 1
#endif

/* Define to 1 if you have the `strtod' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRTOD
#define WIZARDSTOOLKIT_HAVE_STRTOD 1
#endif

/* Define to 1 if you have the `strtod_l' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRTOD_L
#define WIZARDSTOOLKIT_HAVE_STRTOD_L 1
#endif

/* Define to 1 if you have the `strtol' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRTOL
#define WIZARDSTOOLKIT_HAVE_STRTOL 1
#endif

/* Define to 1 if you have the `strtoul' function. */
#ifndef WIZARDSTOOLKIT_HAVE_STRTOUL
#define WIZARDSTOOLKIT_HAVE_STRTOUL 1
#endif

/* Define to 1 if you have the <sun_prefetch.h> header file. */
/* #undef HAVE_SUN_PREFETCH_H */

/* Define to 1 if you have the `sysconf' function. */
#ifndef WIZARDSTOOLKIT_HAVE_SYSCONF
#define WIZARDSTOOLKIT_HAVE_SYSCONF 1
#endif

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_MMAN_H
#define WIZARDSTOOLKIT_HAVE_SYS_MMAN_H 1
#endif

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_PARAM_H
#define WIZARDSTOOLKIT_HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <sys/resource.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_RESOURCE_H
#define WIZARDSTOOLKIT_HAVE_SYS_RESOURCE_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_STAT_H
#define WIZARDSTOOLKIT_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/syslimits.h> header file. */
/* #undef HAVE_SYS_SYSLIMITS_H */

/* Define to 1 if you have the <sys/timeb.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_TIMEB_H
#define WIZARDSTOOLKIT_HAVE_SYS_TIMEB_H 1
#endif

/* Define to 1 if you have the <sys/times.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_TIMES_H
#define WIZARDSTOOLKIT_HAVE_SYS_TIMES_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_TIME_H
#define WIZARDSTOOLKIT_HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_SYS_TYPES_H
#define WIZARDSTOOLKIT_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <sys/utime.h> header file. */
/* #undef HAVE_SYS_UTIME_H */

/* Define to 1 if you have the <termios.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_TERMIOS_H
#define WIZARDSTOOLKIT_HAVE_TERMIOS_H 1
#endif

/* Define to 1 if you have the `times' function. */
#ifndef WIZARDSTOOLKIT_HAVE_TIMES
#define WIZARDSTOOLKIT_HAVE_TIMES 1
#endif

/* Define to 1 if the system has the type `uintmax_t'. */
#ifndef WIZARDSTOOLKIT_HAVE_UINTMAX_T
#define WIZARDSTOOLKIT_HAVE_UINTMAX_T 1
#endif

/* Define to 1 if the system has the type `uintptr_t'. */
#ifndef WIZARDSTOOLKIT_HAVE_UINTPTR_T
#define WIZARDSTOOLKIT_HAVE_UINTPTR_T 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_UNISTD_H
#define WIZARDSTOOLKIT_HAVE_UNISTD_H 1
#endif

/* Define to 1 if the system has the type `unsigned long long int'. */
#ifndef WIZARDSTOOLKIT_HAVE_UNSIGNED_LONG_LONG_INT
#define WIZARDSTOOLKIT_HAVE_UNSIGNED_LONG_LONG_INT 1
#endif

/* Define to 1 if you have the `uselocale' function. */
#ifndef WIZARDSTOOLKIT_HAVE_USELOCALE
#define WIZARDSTOOLKIT_HAVE_USELOCALE 1
#endif

/* Define to 1 if you have the `usleep' function. */
#ifndef WIZARDSTOOLKIT_HAVE_USLEEP
#define WIZARDSTOOLKIT_HAVE_USLEEP 1
#endif

/* Define to 1 if you have the `utime' function. */
#ifndef WIZARDSTOOLKIT_HAVE_UTIME
#define WIZARDSTOOLKIT_HAVE_UTIME 1
#endif

/* Define to 1 if you have the <utime.h> header file. */
#ifndef WIZARDSTOOLKIT_HAVE_UTIME_H
#define WIZARDSTOOLKIT_HAVE_UTIME_H 1
#endif

/* Define to 1 if you have the `vfprintf' function. */
#ifndef WIZARDSTOOLKIT_HAVE_VFPRINTF
#define WIZARDSTOOLKIT_HAVE_VFPRINTF 1
#endif

/* Define to 1 if you have the `vfprintf_l' function. */
/* #undef HAVE_VFPRINTF_L */

/* Define to 1 if you have the `vprintf' function. */
#ifndef WIZARDSTOOLKIT_HAVE_VPRINTF
#define WIZARDSTOOLKIT_HAVE_VPRINTF 1
#endif

/* Define to 1 if you have the `vsnprintf' function. */
#ifndef WIZARDSTOOLKIT_HAVE_VSNPRINTF
#define WIZARDSTOOLKIT_HAVE_VSNPRINTF 1
#endif

/* Define to 1 if you have the `vsnprintf_l' function. */
/* #undef HAVE_VSNPRINTF_L */

/* Define to 1 if you have the `vsprintf' function. */
#ifndef WIZARDSTOOLKIT_HAVE_VSPRINTF
#define WIZARDSTOOLKIT_HAVE_VSPRINTF 1
#endif

/* Define to 1 if you have the <xlocale.h> header file. */
/* #undef HAVE_XLOCALE_H */

/* Define to 1 if you have the `_aligned_malloc' function. */
/* #undef HAVE__ALIGNED_MALLOC */

/* Define to 1 if the system has the type `_Bool'. */
#ifndef WIZARDSTOOLKIT_HAVE__BOOL
#define WIZARDSTOOLKIT_HAVE__BOOL 1
#endif

/* Define to 1 if you have the `_NSGetExecutablePath' function. */
/* #undef HAVE__NSGETEXECUTABLEPATH */

/* Define to 1 if you have the `_wfopen' function. */
/* #undef HAVE__WFOPEN */

/* define if your compiler has __attribute__ */
#ifndef WIZARDSTOOLKIT_HAVE___ATTRIBUTE__
#define WIZARDSTOOLKIT_HAVE___ATTRIBUTE__ 1
#endif

/* Define if you have jemalloc memory allocation library */
/* #undef HasJEMALLOC */

/* Define if you have umem memory allocation library */
/* #undef HasUMEM */

/* Define to use the Windows GDI32 library */
/* #undef HasWINGDI32 */

/* Directory where Wizards Toolkit architecture headers live. */
#ifndef WIZARDSTOOLKIT_INCLUDEARCH_PATH
#define WIZARDSTOOLKIT_INCLUDEARCH_PATH "/usr/local/include/WizardsToolkit-1/"
#endif

/* Directory where WizardsToolkit headers live. */
#ifndef WIZARDSTOOLKIT_INCLUDE_PATH
#define WIZARDSTOOLKIT_INCLUDE_PATH "/usr/local/include/WizardsToolkit-1/"
#endif

/* Wizard's Toolkit is formally installed under prefix */
#ifndef WIZARDSTOOLKIT_INSTALLED_SUPPORT
#define WIZARDSTOOLKIT_INSTALLED_SUPPORT 1
#endif

/* Directory where architecture-dependent files live. */
#ifndef WIZARDSTOOLKIT_LIBRARY_PATH
#define WIZARDSTOOLKIT_LIBRARY_PATH "/usr/local/lib/WizardsToolkit-1.0.9/"
#endif

/* Subdirectory of lib where Wizard architecture dependent files are installed
   */
#ifndef WIZARDSTOOLKIT_LIBRARY_RELATIVE_PATH
#define WIZARDSTOOLKIT_LIBRARY_RELATIVE_PATH "WizardsToolkit-1.0.9"
#endif

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#ifndef WIZARDSTOOLKIT_LSTAT_FOLLOWS_SLASHED_SYMLINK
#define WIZARDSTOOLKIT_LSTAT_FOLLOWS_SLASHED_SYMLINK 1
#endif

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#ifndef WIZARDSTOOLKIT_LT_OBJDIR
#define WIZARDSTOOLKIT_LT_OBJDIR ".libs/"
#endif

/* Define if you have LZMA library */
#ifndef WIZARDSTOOLKIT_LZMA_DELEGATE
#define WIZARDSTOOLKIT_LZMA_DELEGATE 1
#endif

/* Define to 1 if assertions should be disabled. */
/* #undef NDEBUG */

/* Name of package */
#ifndef WIZARDSTOOLKIT_PACKAGE
#define WIZARDSTOOLKIT_PACKAGE "WizardsToolkit"
#endif

/* Define to the address where bug reports for this package should be sent. */
#ifndef WIZARDSTOOLKIT_PACKAGE_BUGREPORT
#define WIZARDSTOOLKIT_PACKAGE_BUGREPORT "http://www.wizards-toolkit.org"
#endif

/* Define to the full name of this package. */
#ifndef WIZARDSTOOLKIT_PACKAGE_NAME
#define WIZARDSTOOLKIT_PACKAGE_NAME "WizardsToolkit"
#endif

/* Define to the full name and version of this package. */
#ifndef WIZARDSTOOLKIT_PACKAGE_STRING
#define WIZARDSTOOLKIT_PACKAGE_STRING "WizardsToolkit 1.0.9-4"
#endif

/* Define to the one symbol short name of this package. */
#ifndef WIZARDSTOOLKIT_PACKAGE_TARNAME
#define WIZARDSTOOLKIT_PACKAGE_TARNAME "WizardsToolkit"
#endif

/* Define to the home page for this package. */
#ifndef WIZARDSTOOLKIT_PACKAGE_URL
#define WIZARDSTOOLKIT_PACKAGE_URL "http://www.wizards-toolkit.org"
#endif

/* Define to the version of this package. */
#ifndef WIZARDSTOOLKIT_PACKAGE_VERSION
#define WIZARDSTOOLKIT_PACKAGE_VERSION "1.0.9-4"
#endif

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Sharearch directory name without ABI part. */
#ifndef WIZARDSTOOLKIT_SHAREARCH_BASEDIRNAME
#define WIZARDSTOOLKIT_SHAREARCH_BASEDIRNAME "config"
#endif

/* Sharearch directory dirname */
/* #undef SHAREARCH_DIRNAME */

/* Directory where architecture-independent configuration files live. */
#ifndef WIZARDSTOOLKIT_SHARE_PATH
#define WIZARDSTOOLKIT_SHARE_PATH "/usr/local/share/WizardsToolkit-1/"
#endif

/* Subdirectory of lib where architecture-independent configuration files
   live. */
#ifndef WIZARDSTOOLKIT_SHARE_RELATIVE_PATH
#define WIZARDSTOOLKIT_SHARE_RELATIVE_PATH "WizardsToolkit-1"
#endif

/* The size of `off_t', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_OFF_T
#define WIZARDSTOOLKIT_SIZEOF_OFF_T 8
#endif

/* The size of `signed int', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_SIGNED_INT
#define WIZARDSTOOLKIT_SIZEOF_SIGNED_INT 4
#endif

/* The size of `signed long', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_SIGNED_LONG
#define WIZARDSTOOLKIT_SIZEOF_SIGNED_LONG 8
#endif

/* The size of `signed long long', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_SIGNED_LONG_LONG
#define WIZARDSTOOLKIT_SIZEOF_SIGNED_LONG_LONG 8
#endif

/* The size of `signed short', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_SIGNED_SHORT
#define WIZARDSTOOLKIT_SIZEOF_SIGNED_SHORT 2
#endif

/* The size of `size_t', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_SIZE_T
#define WIZARDSTOOLKIT_SIZEOF_SIZE_T 8
#endif

/* The size of `unsigned int', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_UNSIGNED_INT
#define WIZARDSTOOLKIT_SIZEOF_UNSIGNED_INT 4
#endif

/* The size of `unsigned int*', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_UNSIGNED_INTP
#define WIZARDSTOOLKIT_SIZEOF_UNSIGNED_INTP 8
#endif

/* The size of `unsigned long', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_UNSIGNED_LONG
#define WIZARDSTOOLKIT_SIZEOF_UNSIGNED_LONG 8
#endif

/* The size of `unsigned long long', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_UNSIGNED_LONG_LONG
#define WIZARDSTOOLKIT_SIZEOF_UNSIGNED_LONG_LONG 8
#endif

/* The size of `unsigned short', as computed by sizeof. */
#ifndef WIZARDSTOOLKIT_SIZEOF_UNSIGNED_SHORT
#define WIZARDSTOOLKIT_SIZEOF_UNSIGNED_SHORT 2
#endif

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
/* #undef STAT_MACROS_BROKEN */

/* Define to 1 if you have the ANSI C header files. */
#ifndef WIZARDSTOOLKIT_STDC_HEADERS
#define WIZARDSTOOLKIT_STDC_HEADERS 1
#endif

/* Define to 1 if strerror_r returns char *. */
#ifndef WIZARDSTOOLKIT_STRERROR_R_CHAR_P
#define WIZARDSTOOLKIT_STRERROR_R_CHAR_P 1
#endif

/* Define if you have POSIX threads libraries and header files. */
#ifndef WIZARDSTOOLKIT_THREAD_SUPPORT
#define WIZARDSTOOLKIT_THREAD_SUPPORT 1
#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#ifndef WIZARDSTOOLKIT_TIME_WITH_SYS_TIME
#define WIZARDSTOOLKIT_TIME_WITH_SYS_TIME 1
#endif

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#ifndef WIZARDSTOOLKIT_VERSION
#define WIZARDSTOOLKIT_VERSION "1.0.9-4"
#endif

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */

/* Target Host CPU */
#ifndef WIZARDSTOOLKIT_WIZARD_TARGET_CPU
#define WIZARDSTOOLKIT_WIZARD_TARGET_CPU x86_64
#endif

/* Target Host OS */
#ifndef WIZARDSTOOLKIT_WIZARD_TARGET_OS
#define WIZARDSTOOLKIT_WIZARD_TARGET_OS linux-gnu
#endif

/* Target Host Vendor */
#ifndef WIZARDSTOOLKIT_WIZARD_TARGET_VENDOR
#define WIZARDSTOOLKIT_WIZARD_TARGET_VENDOR pc
#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Wizard API method prefix */
/* #undef WizardMethodPrefix */

/* Define if you have ZLIB library */
#ifndef WIZARDSTOOLKIT_ZLIB_DELEGATE
#define WIZARDSTOOLKIT_ZLIB_DELEGATE 1
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to appropriate substitue if compiler does not have __func__ */
/* #undef __func__ */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to the widest signed integer type if <stdint.h> and <inttypes.h> do
   not define. */
/* #undef intmax_t */

/* Define to the type of a signed integer type wide enough to hold a pointer,
   if such a type exists, and if the system does not define it. */
/* #undef intptr_t */

/* Define to a type if <wchar.h> does not define. */
/* #undef mbstate_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#ifndef _wizardstoolkit_restrict
#define _wizardstoolkit_restrict __restrict
#endif
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

/* Define to the widest unsigned integer type if <stdint.h> and <inttypes.h>
   do not define. */
/* #undef uintmax_t */

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
/* #undef uintptr_t */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */
 
/* once: _WIZARD_WIZARD_CONFIG_H */
#endif
