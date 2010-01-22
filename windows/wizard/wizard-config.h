// The Wizard's Toolkit build feature configuration.
//
// Enabled options are of the form:
//
//   #define option
//
// while disabled options are initially in the form
//
//   // #undef option
//
// so it is necessary to remove the comment, and change "undef" to "define"
// in order for the option to be enabled.

// Define to build a Wizard's Toolkit which uses registry settings or
// hard-coded paths to locate installed components.  This supports using
// the "setup.exe" style installer, or using hard-coded path definitions
// (see below).  If you want to be able to simply copy the built Wizard's
// Toolkit to any directory on any directory on any machine, then do not
// use this setting.

//
// #undef WIZARDSTOOLKIT_INSTALLED_SUPPORT

// When building the Wizard Toolkit using DLLs, include a DllMain()
// function which automatically invokes WizardsToolkitGenesis(NULL), and
// WizardsToolkitTerminus() so that the user doesn't need to. This is enabled
// by default.
//
#define ProvideDllMain

/////////////
//
// Optional packages
//
// All packages except autotrace are included by default in the build.

// Define to use the bzip2 compression library
#define HasBZLIB

// Define to use the zlib ZIP compression library
#define HasZLIB

// Define to enable self-contained, embeddable, zero-configuration
// Wizard's Toolkit (experimental)
//
// #undef WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT
/////////////

//
// Hard Coded Paths
// 
// If hard-coded paths are defined via the the following define
// statements, then they will override any values from the Windows
// registry.    It is unusual to use hard-coded paths under Windows.

// Optional: Specify where encipher.exe and support executables are installed
// #define WIZARDSTOOLKIT_EXECUTABLE_PATH "c:\\Wizard's Toolkit\\"

// Optional: Specify where operating system specific files are installed
// #define WIZARDSTOOLKIT_LIBRARY_PATH  "c:\\Wizard's Toolkit\\"

// Optional: Specify where operating system independent files are installed
// #define WIZARDSTOOLKIT_SHARE_PATH  "c:\\Wizard's Toolkit\\"

// Wizard API method prefix.  Define to add a unique prefix to all API methods.
// #undef WIZARDSTOOLKIT_NAMESPACE_PREFIX

/////////////
//
// The remaining defines should not require user modification.
//

// Use Visual C++ C inline method extension to improve performance
#define inline __inline

// Visual C++ does not define restrict by default.
#if !defined(restrict)
  #define restrict
#endif

// Visual C++ does not define ssize_t by default.
#if !defined(ssize_t)
#  define ssize_t  long
#endif

#if !defined(__FUNCTION__)
  #define __FUNCTION__  "unknown"
#endif
#define __func__  __FUNCTION__

/* Define to 1 if you have the `ftruncate' function. */
#define WIZARDSTOOLKIT_HAVE_FTRUNCATE 1

// Define to support memory mapping files for improved performance
#define WIZARDSTOOLKIT_HAVE_MMAP_FILEIO 1

/* Define to 1 if you have the `raise' function. */
#define WIZARDSTOOLKIT_HAVE_RAISE 1

/* Define to 1 if you have the `memmove' function. */
#define WIZARDSTOOLKIT_HAVE_MEMMOVE 1

/* Define to 1 if you have the `sysconf' function. */
#define WIZARDSTOOLKIT_HAVE_SYSCONF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define WIZARDSTOOLKIT_HAVE_VSNPRINTF 1

/* Define to 1 if you have the `popen' function. */
#define WIZARDSTOOLKIT_HAVE_POPEN 1

/* Define to   if you have the `strcasecmp' function. */
#define WIZARDSTOOLKIT_HAVE_STRCASECMP 1

/* Define to 1 if you have the `strncasecmp' function. */
#define WIZARDSTOOLKIT_HAVE_STRNCASECMP 1

/* Define to 1 if you have the `tempnam' function. */
#define WIZARDSTOOLKIT_HAVE_TEMPNAM 1

// Define to include the <sys/types.h> header file
#define WIZARDSTOOLKIT_HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `_wfopen' function. */
#define WIZARDSTOOLKIT_HAVE__WFOPEN 1

#define WIZARDSTOOLKIT_HAVE_VSNPRINTF 1
#define WIZARDSTOOLKIT_HAVE_TEMPNAM 1
#define WIZARDSTOOLKIT_HAVE_RAISE 1
#define WIZARDSTOOLKIT_HAVE_UTIME 1
#define WiZARDSTOOLKIT_STDC_HEADERS 1
#define WiZARDSTOOLKIT_HAVE_STRING_H
