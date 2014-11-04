#  Copyright 1999-2014 Wizards Toolkit Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.wizards-toolkit.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Copying and distribution of this file, with or without modification,
#  are permitted in any medium without royalty provided the copyright
#  notice and this notice are preserved.

AC_PREREQ(2.69)

m4_define([wizard_major_version], [1])
m4_define([wizard_minor_version], [0])
m4_define([wizard_micro_version], [9])
m4_define([wizard_patchlevel_version], [2])
m4_define([wizard_version],
          [wizard_major_version.wizard_minor_version.wizard_micro_version-wizard_patchlevel_version])
m4_define([wizard_svn_revision], esyscmd([sh -c "(svnversion .) | awk '{ print \$1 }' | tr -d '\n'"]))

AC_INIT([WizardsToolkit],[wizard_version],[http://www.wizards-toolkit.org],[WizardsToolkit])
AC_CONFIG_SRCDIR([wizard/WizardsToolkit.h])
AC_CONFIG_AUX_DIR([config])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([wizard])
AC_CONFIG_HEADERS([config/config.h])
AX_PREFIX_CONFIG_H([wizard/wizard-config.h],[WizardsToolkit])
AC_CONFIG_FILES([\
  common.shi \
  config/configure.xml \
  config/WizardsToolkit.dox \
  Makefile \
  wizard.sh \
  utilities/cipher-info.1 \
  utilities/decipher.1 \
  utilities/digest.1 \
  utilities/encipher.1 \
  utilities/keyring.1 \
  windows/WizardsToolkit.bat \
  wizard/version.h \
  wizard/WizardsToolkit-config \
  wizard/WizardsToolkit.pc \
  WizardsToolkit.spec ])

#
# Save initial user-tunable values
#
USER_LIBS=$LIBS
for var in CC CFLAGS CPPFLAGS CXX CXXCPP LDFLAGS LIBS ; do
  eval isset=\${$var+set}
  if test "$isset" = 'set'; then
    eval val=$`echo $var`
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS}'${var}=${val}' "
  fi
done
AC_SUBST(DISTCHECK_CONFIG_FLAGS)

CONFIGURE_ARGS="$0 ${ac_configure_args}"
AC_SUBST(CONFIGURE_ARGS)

# Source file containing package/library versioning information.
. ${srcdir}/version.sh

echo "configuring ${PACKAGE_NAME} ${PACKAGE_VERSION}${PACKAGE_VERSION_ADDENDUM}"
AC_CANONICAL_SYSTEM
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

WIZARD_TARGET_CPU=$host_cpu
AC_SUBST(WIZARD_TARGET_CPU)
AC_DEFINE_UNQUOTED(WIZARD_TARGET_CPU,$WIZARD_TARGET_CPU,[Target Host CPU])

WIZARD_TARGET_VENDOR=$host_vendor
AC_SUBST(WIZARD_TARGET_VENDOR)
AC_DEFINE_UNQUOTED(WIZARD_TARGET_VENDOR,$WIZARD_TARGET_VENDOR,[Target Host Vendor])

WIZARD_TARGET_OS=$host_os
AC_SUBST(WIZARD_TARGET_OS)
AC_DEFINE_UNQUOTED(WIZARD_TARGET_OS,$WIZARD_TARGET_OS,[Target Host OS])

# Substitute versioning
AC_SUBST([WIZARD_MAJOR_VERSION],[wizard_major_version])
AC_SUBST([WIZARD_MINOR_VERSION],[wizard_minor_version])
AC_SUBST([WIZARD_MICRO_VERSION],[wizard_micro_version])
AC_SUBST([WIZARD_PATCHLEVEL_VERSION],[wizard_patchlevel_version])
AC_SUBST([WIZARD_VERSION],[wizard_version])
AC_SUBST([WIZARD_SVN_REVISION],[wizard_svn_revision])

# Substitute library versioning
AC_SUBST(WIZARD_LIBRARY_CURRENT)dnl
AC_SUBST(WIZARD_LIBRARY_REVISION)dnl
AC_SUBST(WIZARD_LIBRARY_AGE)dnl
AC_SUBST([WIZARD_LIBRARY_CURRENT_MIN],
         [`expr $WIZARD_LIBRARY_CURRENT - $WIZARD_LIBRARY_AGE`])
AC_SUBST([WIZARD_LIBRARY_VERSION_INFO],
         [$WIZARD_LIBRARY_CURRENT:$WIZARD_LIBRARY_REVISION:$WIZARD_LIBRARY_AGE])
AC_SUBST([WIZARD_SVN_REVISION],[wizard_svn_revision]) 

AC_SUBST(PACKAGE_NAME)dnl
AC_SUBST(PACKAGE_VERSION)dnl
AC_SUBST(PACKAGE_RELEASE)dnl
AC_SUBST(PACKAGE_CHANGE_DATE)dnl
AC_SUBST(PACKAGE_LIB_VERSION)dnl
AC_SUBST(PACKAGE_LIB_VERSION_NUMBER)dnl
AC_SUBST(PACKAGE_RELEASE_DATE)dnl
AC_SUBST(PACKAGE_VERSION_ADDENDUM)dnl

# Substitute CVS branch tag
AC_SUBST(CVS_BRANCH_TAG)dnl

# Ensure that make can run correctly
AM_SANITY_CHECK

AM_INIT_AUTOMAKE([1.12 nostdinc color-tests foreign parallel-tests silent-rules subdir-objects dist-bzip2 dist-lzip dist-xz dist-zip])

# Enable support for silent build rules
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

WIZARD_LIB_VERSION="0x"
if test ${WIZARD_LIBRARY_CURRENT} -lt 10 ; then
  WIZARD_LIB_VERSION=${WIZARD_LIB_VERSION}0
fi
WIZARD_LIB_VERSION=${WIZARD_LIB_VERSION}${WIZARD_LIBRARY_CURRENT}
if test ${WIZARD_LIBRARY_AGE} -lt 10 ; then
  WIZARD_LIB_VERSION=${WIZARD_LIB_VERSION}0
fi
WIZARD_LIB_VERSION=${WIZARD_LIB_VERSION}${WIZARD_LIBRARY_AGE}
if test ${WIZARD_LIBRARY_REVISION} -lt 10 ; then
  WIZARD_LIB_VERSION=${WIZARD_LIB_VERSION}0
fi
WIZARD_LIB_VERSION=${WIZARD_LIB_VERSION}${WIZARD_LIBRARY_REVISION}
AC_SUBST(WIZARD_LIB_VERSION)

# Definition used to define WizardLibVersionText in version.h
WIZARD_LIB_VERSION_TEXT="${PACKAGE_VERSION}"
AC_SUBST(WIZARD_LIB_VERSION_TEXT)

# Definition used to define WizardLibVersionNumber in version.h
WIZARD_LIB_VERSION_NUMBER="${WIZARD_LIBRARY_CURRENT},${WIZARD_LIBRARY_AGE},${WIZARD_LIBRARY_REVISION}"
AC_SUBST(WIZARD_LIB_VERSION_NUMBER)

# Regenerate config.status if ChangeLog or version.sh is updated.
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/version.sh'])

WIZARD_CFLAGS=''
WIZARD_CPPFLAGS=$CPPFLAGS_USER
WIZARD_PCFLAGS=$CPPFLAGS_USER
WIZARD_LDFLAGS=''
WIZARD_LIBS=''

#
# Evaluate shell variable equivalents to Makefile directory variables
#
if test "x$prefix" = xNONE; then
  prefix=$ac_default_prefix
fi
# Let make expand exec_prefix.
if test "x$exec_prefix" = xNONE; then
  exec_prefix='${prefix}'
fi

#
eval "eval PREFIX_DIR=${prefix}"
AC_SUBST(PREFIX_DIR)
eval "eval EXEC_PREFIX_DIR=${exec_prefix}"
AC_SUBST(EXEC_PREFIX_DIR)
eval "eval BIN_DIR=$bindir"
AC_SUBST(BIN_DIR)
eval "eval SBIN_DIR=$sbindir"
AC_SUBST(SBIN_DIR)
eval "eval LIBEXEC_DIR=$libexecdir"
AC_SUBST(LIBEXEC_DIR)
eval "eval DATA_DIR=$datadir"
AC_SUBST(DATA_DIR)
eval "eval DOC_DIR=$docdir"
AC_SUBST(DOC_DIR)
eval "eval SYSCONF_DIR=$sysconfdir"
AC_SUBST(SYSCONF_DIR)
eval "eval SHAREDSTATE_DIR=$sharedstatedir"
AC_SUBST(SHAREDSTATE_DIR)
eval "eval LOCALSTATE_DIR=$localstatedir"
AC_SUBST(LOCALSTATE_DIR)
eval "eval LIB_DIR=$libdir"
AC_SUBST(LIB_DIR)
eval "eval INCLUDE_DIR=$includedir"
AC_SUBST(INCLUDE_DIR)
eval "eval OLDINCLUDE_DIR=$oldincludedir"
AC_SUBST(OLDINCLUDE_DIR)
eval "eval INFO_DIR=$infodir"
AC_SUBST(INFO_DIR)
eval "eval MAN_DIR=$mandir"
AC_SUBST(MAN_DIR)

# Get full paths to source and build directories
srcdirfull="`cd $srcdir && pwd`"
builddir="`pwd`"

#
# Compute variables useful for running uninstalled software.
#
WIZARD_CONFIGURE_SRC_PATH="${srcdirfull}/config"
WIZARD_CONFIGURE_BUILD_PATH="${builddir}/config"
DIRSEP=':'
case "${build_os}" in
  mingw* )
    WIZARD_CONFIGURE_SRC_PATH=`$WinPathScript "${WIZARD_CONFIGURE_SRC_PATH}" 0`
    WIZARD_CONFIGURE_BUILD_PATH=`$WinPathScript "${WIZARD_CONFIGURE_BUILD_PATH}" 0`
    DIRSEP=';'
    ;;
esac
case "${host_os}" in
  mingw* )
    DIRSEP=';'
    ;;
esac
AC_SUBST(WIZARD_CONFIGURE_SRC_PATH)
AC_SUBST(WIZARD_CONFIGURE_BUILD_PATH)
AC_SUBST(DIRSEP)

#
# Enable OS features.
#
AC_USE_SYSTEM_EXTENSIONS

# Check for programs
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LD
AC_SUBST(LD)
AC_PROG_CC_C99
AM_PROG_CC_C_O
AX_CFLAGS_WARN_ALL
AX_COMPILER_VENDOR
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_AWK
AM_WITH_DMALLOC
AX_C___ATTRIBUTE__
AX_GCC_ARCHFLAG([no])
PKG_PROG_PKG_CONFIG

# Check for linker script support 
gl_LD_VERSION_SCRIPT

#
# Tests for Windows
#
AC_EXEEXT
AC_OBJEXT

GDI32_LIBS=''
native_win32_build='no'
cygwin_build='no'
case "${host_os}" in
  cygwin* )
    cygwin_build='yes'
    GDI32_LIBS='-lgdi32'
    ;;
  mingw* )
    native_win32_build='yes'
    GDI32_LIBS='-lgdi32'
    ;;
esac
if test "${GDI32_LIBS}x" != 'x'; then
  AC_DEFINE(HasWINGDI32,1,Define to use the Windows GDI32 library)
fi
AC_SUBST(GDI32_LIBS)
AM_CONDITIONAL(HasWINGDI32, test "${GDI32_LIBS}x" != 'x' )
AM_CONDITIONAL(WIN32_NATIVE_BUILD, test "${native_win32_build}" = 'yes' )
AM_CONDITIONAL(CYGWIN_BUILD, test "${cygwin_build}" = 'yes' )
AM_CONDITIONAL(USING_CL, test "x${CC}" = 'xcl.exe' )

WinPathScript="${srcdirfull}/winpath.sh"
AC_SUBST(WinPathScript)

#
# Compiler flags tweaks
#
if test "${GCC}" != "yes"; then
  case "${host}" in
    *-*-hpux* )
      # aCC: HP ANSI C++ B3910B A.03.34
      CFLAGS="${CFLAGS} -Wp,-H30000"
      if test -n "${CXXFLAGS}"; then
        CXXFLAGS='-AA'
      else
        CXXFLAGS="${CXXFLAGS} -AA"
      fi
      ;;
    *-dec-osf5.* )
      # Compaq alphaev68-dec-osf5.1 compiler
      if test -n "${CXXFLAGS}"; then
        CXXFLAGS='-std strict_ansi -noimplicit_include'
      else
        CXXFLAGS="${CXXFLAGS} -std strict_ansi -noimplicit_include"
      fi
  esac
fi

# Check for lazy-loading.
AC_CACHE_CHECK([for linker lazyload option],[wt_cv_ld_lazyload],
[
wt_cv_ld_lazyload='none'
case "${host}" in
  *-*-solaris2.8 | *-*-solaris2.9 | *-*-solaris2.1? )
    if test "$lt_cv_prog_gnu_ld" != 'yes' ; then
       wt_cv_ld_lazyload='-Wl,-zlazyload'
    fi
    ;;
esac
])
if test "${wt_cv_ld_lazyload}" != 'none' ; then
  if test -z "${LDFLAGS}" ; then
    LDFLAGS="${wt_cv_ld_lazyload}"
  else
    LDFLAGS="${wt_cv_ld_lazyload} ${LDFLAGS}"
  fi
fi

dnl Platform-specific stuff
case "$host" in
*-darwin* | *-macos10*)
  if test -d /opt/local ; then
    CPPFLAGS="$CPPFLAGS -I/opt/local/include"
    LDFLAGS="$LDFLAGS -L/opt/local/lib"
  elif test -d /sw ; then
    CPPFLAGS="$CPPFLAGS -I/sw/include"
    LDFLAGS="$LDFLAGS -L/sw/lib"
  fi
  dnl OS X universal binary support, requires --disable-dependency-tracking
  AC_ARG_ENABLE(osx-universal-binary,
        AC_HELP_STRING([--enable-osx-universal-binary],
           [build universal binary on OS X [[default=no]]]),
        [build_osxuniversal="${enableval}"], [build_osxuniversal=no])
  
  if test "${build_osxuniversal}" != no ; then
    if test "$enable_dependency_tracking" != no ; then
      AC_MSG_ERROR([--enable-osx-universal-binary requires --disable-dependency-tracking.
Please re-run configure with these options:
  --disable-dependency-tracking --enable-osx-universal-binary
        ])
    fi
    CFLAGS="$CFLAGS -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc -arch i386"
    CXXFLAGS="$CXXFLAGS -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc -arch i386"
    LDFLAGS="$LDFLAGS -Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk -arch ppc -arch i386"
  fi
  ;;
esac

#
# ARCH specific include directory
#
AC_ARG_WITH([includearch-dir],
    [AC_HELP_STRING([--includearch-dir=DIR],
                    [ARCH specific include directory])],
                    [includearch_dir=$withval],
                    [includearch_dir=$INCLUDE_DIR])

eval "eval INCLUDEARCH_DIR=$includearch_dir"
AC_SUBST(INCLUDEARCH_DIR)

#
# ARCH specific configuration directory
#
AC_ARG_WITH([sharearch-dir],
    [AC_HELP_STRING([--sharearch-dir=DIR],
                    [ARCH specific config directory])],
                    [sharearch_dir=$withval],
                    [sharearch_dir="${LIB_DIR}"])

eval "eval SHAREARCH_DIR=$sharearch_dir"
SHAREARCH_DIR="$sharearch_dir"
AC_SUBST(SHAREARCH_DIR)

# Path to the pkgconfig folder
AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=DIR],
            [Path to the pkgconfig directory @<:@LIBDIR/pkgconfig@:>@]),
            [pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

# Enable support for threads
AC_ARG_WITH([threads],
    [AC_HELP_STRING([--without-threads],
                    [disable threads support])],
    [with_threads=$withval],
    [with_threads='yes'])

have_threads=no
if test "$with_threads" != 'no'; then
    AX_PTHREAD()
    if test "$ax_pthread_ok" = yes; then
        have_threads=yes
        DEF_THREAD="$PTHREAD_CFLAGS"
        CFLAGS="$CFLAGS $DEF_THREAD"
        CXXFLAGS="$CXXFLAGS $DEF_THREAD"
	THREAD_LIBS="$PTHREAD_LIBS"
        if test "$CC" != "$PTHREAD_CC"; then
            AC_MSG_WARN([Replacing compiler $CC with compiler $PTHREAD_CC to support pthreads.])
            CC="$PTHREAD_CC"
        fi
        AC_DEFINE(THREAD_SUPPORT,1,[Define if you have POSIX threads libraries and header files.])
    fi
fi
LIBS="$LIBS $THREAD_LIBS"
AC_SUBST(THREAD_LIBS)

# Enable support for OpenMP
if test "$have_threads" != 'yes'; then
  ac_cv_prog_c_openmp=unsupported
fi
AC_OPENMP([C])
CFLAGS="$OPENMP_CFLAGS $CFLAGS"
WIZARD_PCFLAGS="$WIZARD_PCFLAGS $OPENMP_CFLAGS"
AC_SUBST(OPENMP_CFLAGS)

########
#
# Check for large file support
#
########
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
LFS_CPPFLAGS=''
if test "$enable_largefile" != no; then
    case $ac_cv_sys_file_offset_bits in
    no)
        # nothing to do here as the host supports LFS fine
        ;;
    unknown)
        AC_MSG_CHECKING([for native large file support])
        AC_RUN_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>
          main () {
          exit(!(sizeof(off_t) == 8));
        }])],
        [ac_cv_sys_file_offset_bits=64; AC_DEFINE(_FILE_OFFSET_BITS,64)
         AC_MSG_NOTICE([yes])],
        [AC_MSG_NOTICE([no])])
        ;;
    *)
        LFS_CPPFLAGS="$LFS_CPPFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
        ;;
    esac
    if test "$ac_cv_sys_large_files" != 'no'; then
        LFS_CPPFLAGS="$LFS_CPPFLAGS -D_LARGE_FILES=1"
    fi
    if test "$ac_cv_sys_largefile_source" != 'no'; then
        LFS_CPPFLAGS="$LFS_CPPFLAGS -D_LARGEFILE_SOURCE=1"
    fi
fi
AC_SUBST(LFS_CPPFLAGS)

# Configure libtool
AC_LIBTOOL_DLOPEN
LT_INIT([disable-shared win32-dll])
LT_LANG([C++])
AC_SUBST(LIBTOOL_DEPS)

# Check to see if building shared libraries
libtool_build_shared_libs='no'
if test "$enable_shared" = 'yes'; then
  libtool_build_shared_libs='yes'
fi

# Check to see if building static libraries
libtool_build_static_libs='no'
if test "$enable_static" = 'yes'; then
  libtool_build_static_libs='yes'
fi

AM_CONDITIONAL(WITH_SHARED_LIBS, test "${libtool_build_shared_libs}" = 'yes')

# Enable build using delegate libraries built in subdirectories rather than installed
# delegate libraries (bzlib lzma zlib)
AC_ARG_ENABLE([delegate-build],
    [AC_HELP_STRING([--enable-delegate-build],
                    [look for delegate libraries in build directory])],
    [enable_delegate_build=$enableval],
    [enable_delegate_build='yes'])

# Build a version of Wizard which operates uninstalled.
# Used to build distributions located via WIZARD_HOME / executable path
AC_ARG_ENABLE([installed],
    [AC_HELP_STRING([--disable-installed],                    [Formally install Wizard's Toolkit under PREFIX])],
    [enable_installed=$enableval],
    [enable_installed='yes'])

if test "$enable_installed" = 'yes'; then
  AC_DEFINE(INSTALLED_SUPPORT,1,[Wizard's Toolkit is formally installed under prefix])
else
  DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --disable-installed "
fi
LIBS="$LIBS $THREAD_LIBS"
AC_SUBST(THREAD_LIBS)

# Build an embeddable version of Wizard's Toolkit.
AC_ARG_ENABLE([embeddable],
    [AC_HELP_STRING([--enable-embeddable],
                    [enable self-contained, embeddable, zero-configuration Wizard's Toolkit])],
    [enable_embeddable=$enableval],
    [enable_embeddable='no'])

if test "$with_embeddable" = 'yes'; then
  AC_DEFINE(EMBEDDABLE_SUPPORT,1,[Build self-contained, embeddable, zero-configuration Wizards Toolkit (experimental)])
fi

# Build a version of Wizard with assert statements.
AC_ARG_ENABLE([assert],
    [AC_HELP_STRING([--disable-assert],
                    [disable assert() statements in build])],
    [enable_assert=$enableval],
    [enable_assert='yes'])

if test "$enable_assert" = 'no'; then
  AC_DEFINE(NDEBUG,1,[Turn off assert statements])
fi

# Add configure option --enable-maintainer-mode which enables dependency
# checking and generation useful to package maintainers.  This is made an
# option to avoid confusing end users.
AM_MAINTAINER_MODE

# Enable hugepages support
AC_ARG_ENABLE([hugepages],
    [AC_HELP_STRING([--enable-hugepages],
                    [enable 'huge pages' support])],
    [enable_hugepages=$enableval],
    [enable_hugepages='no'])

# Enable Wizard Debugging
AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--disable-debug],
                    [disable Wizard Executable Environment debugging])],
    [enable_debug=$enableval],
    [enable_debug='yes'])

if test "$enable_debug" = 'yes'; then
  AC_DEFINE(Debug,1,[Enable Wizard Executable Environment debugging])
fi

# Enable ccmalloc memory debugging support
AC_ARG_ENABLE([ccmalloc],
    [AC_HELP_STRING([--enable-ccmalloc],
                    [enable 'ccmalloc' memory debug support])],
    [enable_ccmalloc=$enableval],
    [enable_ccmalloc='no'])

# Enable Electric Fence memory debugging support
AC_ARG_ENABLE([efence],
    [AC_HELP_STRING([--enable-efence],
                    [enable 'efence' memory debug support])],
    [enable_efence=$enableval],
    [enable_efence='no'])

# Enable prof-based profiling support
AC_ARG_ENABLE([prof],
    [AC_HELP_STRING([--enable-prof],
                    [enable 'prof' profiling support])],
    [enable_prof=$enableval],
    [enable_prof='no'])

# Enable gprof-based profiling support
AC_ARG_ENABLE([gprof],
    [AC_HELP_STRING([--enable-gprof],
                    [enable 'gprof' profiling support])],
    [enable_gprof=$enableval], 
    [enable_gprof='no'])

# Enable gcov-based profiling support
AC_ARG_ENABLE([gcov],
    [AC_HELP_STRING([--enable-gcov],
                    [enable 'gcov' profiling support])],
    [enable_gcov=$enableval],
    [enable_gcov='no'])

with_profiling='no'
if test "$enable_prof" = 'yes' || test "$enable_gprof" = 'yes' || test "$enable_gcov" = 'yes'; then
  with_profiling='yes'

  if test "$libtool_build_shared_libs" = 'yes'; then
    echo "Warning: Can not profile code using shared libraries"
  fi
fi

# Wizard API method prefix
AC_ARG_WITH([method-prefix],
    [AC_HELP_STRING([--with-method-prefix=PREFIX],
                    [prefix WizardsToolkit API methods])],
    [with_method_prefix=$withval],
    [with_method_prefix='no'])

if test "$with_method_prefix" != 'no'; then
  AC_DEFINE_UNQUOTED(WizardMethodPrefix,$with_method_prefix,[Wizard API method prefix])
  DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-method-prefix "
fi

# Enable jemalloc, object-caching memory allocation library.
AC_ARG_WITH(jemalloc,
        [  --with-jemalloc         enable jemalloc memory allocation library support],
        [with_jemalloc=$withval],
        [with_jemalloc='no'])
if test "$with_jemalloc" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-jemalloc=$with_jemalloc "
fi

# Enable umem, object-caching memory allocation library.
AC_ARG_WITH(umem,
        [  --with-umem             enable umem memory allocation library support],
        [with_umem=$withval],
        [with_umem='no'])
if test "$with_umem" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-umem=$with_umem "
fi

#
# Specify path to shared libstdc++ if not in normal location
#
AC_ARG_WITH([libstdc],
    [AC_HELP_STRING([--with-libstdc=DIR],
                    [ use libstdc++ in DIR (for GNU C++)])],
    [with_libstdc=$withval],
    [with_libstdc=''])

if test "$with_libstdc" != ''; then
    if test -d "$with_libstdc"; then
        LIBSTDCLDFLAGS="-L$with_libstdc"
    fi
fi
AC_SUBST(LIBSTDCLDFLAGS)

# Does gcc required -traditional?
AC_PROG_GCC_TRADITIONAL

########
#
# Set defines required to build DLLs and modules using MinGW
#
########
# These options are set for multi-thread DLL module build
#  libWizard:           _DLL _WIZARDMOD_ _WIZARDLIB_
#  module:              _DLL
#  executable/Wizard++: _DLL _WIZARDMOD_
MODULE_EXTRA_CPPFLAGS=''
LIBRARY_EXTRA_CPPFLAGS=''
if test "${native_win32_build}" = 'yes'; then
  if test "${libtool_build_shared_libs}" = 'yes'; then
    CPPFLAGS="$CPPFLAGS -D_DLL"
    WIZARD_CPPFLAGS="$WIZARD_CPPFLAGS -D_DLL"
    WIZARD_PCFLAGS="$WIZARD_PCFLAGS -D_DLL"
    LIBRARY_EXTRA_CPPFLAGS="$LIBRARY_EXTRA_CPPFLAGS -D_WIZARDLIB_"

    if test "$with_modules" = 'yes'; then
      LIBRARY_EXTRA_CPPFLAGS="$LIBRARY_EXTRA_CPPFLAGS -D_WIZARDMOD_"
    else
      MODULE_EXTRA_CPPFLAGS="$MODULE_EXTRA_CPPFLAGS -D_WIZARDLIB_"
    fi

  else
    CPPFLAGS="$CPPFLAGS -D_LIB"
    WIZARD_CPPFLAGS="$WIZARD_CPPFLAGS -D_LIB"
    WIZARD_PCFLAGS="$WIZARD_PCFLAGS -D_LIB"

  fi
  if test "$with_threads" = 'yes'; then
     CPPFLAGS="$CPPFLAGS -D_MT"
     WIZARD_CPPFLAGS="$WIZARD_CPPFLAGS -D_MT"
     WIZARD_PCFLAGS="$WIZARD_PCFLAGS -D_MT"
  fi
fi
AC_SUBST(MODULE_EXTRA_CPPFLAGS)
AC_SUBST(LIBRARY_EXTRA_CPPFLAGS)

# Check standard headers
AC_HEADER_STDC
if ! test x"$ac_cv_header_stdc" = x"yes"; then
  AC_MSG_WARN([configure has detected that you do not have the ANSI standard C
    header files.  Compilation cannot proceed.  Please install the ANSI C
    headers and rerun this script.]);
fi
AC_HEADER_ASSERT
AC_HEADER_DIRENT

# Check additional headers
AC_CHECK_HEADERS(argz.h arm/limits.h fcntl.h limits.h linux/unistd.h locale.h mach-o/dyld.h machine/param.h malloc.h process.h sun_prefetch.h stdarg.h sys/mman.h sys/syslimits.h sys/resource.h sys/time.h sys/timeb.h sys/times.h sys/utime.h termios.h utime.h xlocale.h)

########
#
# Checks for typedefs, structures, and compiler characteristics.
#
########

AC_HEADER_STDBOOL
AC_C_VOLATILE
AC_C_STRINGIZE
AC_HEADER_STAT
AC_HEADER_TIME
AC_STRUCT_TM
AC_SYS_INTERPRETER

#
# Checks for language qualifiers and semantics.
#
AC_C_CHAR_UNSIGNED
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_C_VOLATILE

# If words are stored with the most significant byte first (like
# Motorola and SPARC CPUs), define `WORDS_BIGENDIAN'.
AC_C_BIGENDIAN

# Define to a suitable type, if standard headers do not define it.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INTMAX_T
AC_TYPE_INTPTR_T
AC_TYPE_LONG_DOUBLE
AC_TYPE_LONG_DOUBLE_WIDER
AC_TYPE_LONG_LONG_INT
AC_TYPE_MBSTATE_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINTMAX_T
AC_TYPE_UINTPTR_T
AC_TYPE_UNSIGNED_LONG_LONG_INT

# If the C type char is unsigned, define __CHAR_UNSIGNED__, unless the
# C compiler predefines it.
AC_C_CHAR_UNSIGNED

# Obtain size of an 'signed short' and define as SIZEOF_SIGNED_SHORT
AC_CHECK_SIZEOF(signed short)

# Obtain size of an 'unsigned short' and define as SIZEOF_UNSIGNED_SHORT
AC_CHECK_SIZEOF(unsigned short)

# Obtain size of an 'signed int' and define as SIZEOF_SIGNED_INT
AC_CHECK_SIZEOF(signed int)

# Obtain size of an 'unsigned int' and define as SIZEOF_UNSIGNED_INT
AC_CHECK_SIZEOF(unsigned int)

# Obtain size of a 'signed long' and define as SIZEOF_SIGNED_LONG
AC_CHECK_SIZEOF(signed long)

# Obtain size of a 'unsigned long' and define as SIZEOF_UNSIGNED_LONG
AC_CHECK_SIZEOF(unsigned long)

# Obtain size of a 'long long' and define as SIZEOF_SIGNED_LONG_LONG.  If
# 'signed long long' is not supported then the value defined is zero.
AC_CHECK_SIZEOF(signed long long)

# Obtain size of a 'unsigned long long' and define as
# SIZEOF_UNSIGNED_LONG_LONG.  If 'unsigned long long' is not
# supported then the value defined is zero.
AC_CHECK_SIZEOF(unsigned long long)

# Obtain size of off_t and define as SIZEOF_OFF_T
AC_CHECK_SIZEOF(off_t)

# Obtain size of size_t and define as SIZEOF_SIZE_T
AC_CHECK_SIZEOF(size_t)

# Obtain size of an unsigned int pointer and define as SIZEOF_UNSIGNED_INTP
AC_CHECK_SIZEOF(unsigned int*)

#
# Compute sized types for current CPU and compiler options.
#
AC_MSG_CHECKING(for signed 8-bit type)
INT8_T='signed char'
AC_MSG_NOTICE($INT8_T)
AC_SUBST(INT8_T)

AC_MSG_CHECKING(for unsigned 8-bit type)
UINT8_T='unsigned char'
AC_MSG_NOTICE($UINT8_T)
AC_SUBST(UINT8_T)

AC_MSG_CHECKING(for signed 16-bit type)
INT16_T='signed short'
AC_MSG_NOTICE($INT16_T)
AC_SUBST(INT16_T)

AC_MSG_CHECKING(for unsigned 16-bit type)
UINT16_T='unsigned short'
AC_MSG_NOTICE($UINT16_T)
AC_SUBST(UINT16_T)

AC_MSG_CHECKING(for signed 32-bit type)
INT32_T='none'
INT32_F='none'
if test $ac_cv_sizeof_signed_int -eq 4; then
  INT32_T='signed int'
  INT32_F='""'
elif test $ac_cv_sizeof_signed_long -eq 4; then
  INT32_T='signed long'
  INT32_F='"l"'
fi
AC_MSG_NOTICE($INT32_T)
AC_SUBST(INT32_T)
AC_SUBST(INT32_F)

AC_MSG_CHECKING(for unsigned 32-bit type)
UINT32_T='none'
UINT32_F='none'
if test $ac_cv_sizeof_unsigned_int -eq 4; then
  UINT32_T='unsigned int'
  UINT32_F='""'
elif test $ac_cv_sizeof_unsigned_long -eq 4; then
  UINT32_T='unsigned long'
  UINT32_F='"l"'
fi
AC_MSG_NOTICE($UINT32_T)
AC_SUBST(UINT32_T)
AC_SUBST(UINT32_F)

AC_MSG_CHECKING(for signed 64-bit type)
INT64_T='none'
INT64_F='none'
if test $ac_cv_sizeof_signed_long -eq 8; then
  INT64_T='signed long'
  INT64_F='"l"'
elif test $ac_cv_sizeof_signed_long_long -eq 8; then
  INT64_T='signed long long'
  INT64_F='"ll"'
fi
case "${host_os}" in
  mingw* )
    INT64_F='"I64"'
    ;;
esac
AC_MSG_NOTICE($INT64_T)
AC_SUBST(INT64_T)
AC_SUBST(INT64_F)

AC_MSG_CHECKING(for unsigned 64-bit type)
UINT64_T='none'
UINT64_F='none'
if test $ac_cv_sizeof_unsigned_long -eq 8; then
  UINT64_T='unsigned long'
  UINT64_F='"l"'
elif test $ac_cv_sizeof_unsigned_long_long -eq 8; then
  UINT64_T='unsigned long long'
  UINT64_F='"ll"'
fi
case "${host_os}" in
  mingw* )
    UINT64_F='"I64"'
    ;;
esac
AC_MSG_NOTICE($UINT64_T)
AC_SUBST(UINT64_T)
AC_SUBST(UINT64_F)

AC_MSG_CHECKING(for unsigned maximum type)
UINTMAX_T='none'
UINTMAX_F='none'
if test "$UINT64_T" != 'none'; then
  UINTMAX_T=$UINT64_T
  UINTMAX_F=$UINT64_F
elif test "$UINT32_T" != 'none'; then
  UINTMAX_T=$UINT32_T
  UINTMAX_F=$UINT32_F
fi
AC_MSG_NOTICE($UINTMAX_T)
AC_SUBST(UINTMAX_T)
AC_SUBST(UINTMAX_F)

AC_MSG_CHECKING(for pointer difference type)
UINTPTR_T='none'
UINTPTR_F='none'
if test $ac_cv_sizeof_unsigned_long -eq $ac_cv_sizeof_unsigned_intp; then
  UINTPTR_T='unsigned long'
  UINTPTR_F='"l"'
elif test $ac_cv_sizeof_unsigned_long_long -eq $ac_cv_sizeof_unsigned_intp; then
  UINTPTR_T='unsigned long long'
  UINTPTR_F='"ll"'
fi
AC_MSG_NOTICE($UINTPTR_T)
AC_SUBST(UINTPTR_T)
AC_SUBST(UINTPTR_F)



# Test for C compiler __func__ support
AC_MSG_CHECKING([whether our compiler supports __func__])
AC_TRY_COMPILE([],
 [{ const char *func = __func__; return(func != 0 ? 0 : 1); }],
 AC_MSG_NOTICE([yes]),
 AC_MSG_NOTICE([no])
 AC_MSG_CHECKING([whether our compiler supports __FUNCTION__])
 AC_TRY_COMPILE([],
   [{ const char *func = __FUNCTION__; return(func != 0 ? 0 : 1); }],
   AC_MSG_NOTICE([yes])
   AC_DEFINE(__func__, __FUNCTION__,
     [Define to appropriate substitue if compiler does not have __func__]),
   AC_MSG_NOTICE([no])
   AC_DEFINE(__func__, __FILE__,
     [Define to appropriate substitue if compiler does not have __func__])))

########
#
# Check for functions
#
########
WIZARD_FUNC_MMAP_FILEIO
AC_FUNC_VPRINTF
AC_FUNC_STRTOD
AC_FUNC_MMAP
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([_aligned_malloc atexit clock fchmod fprintf_l ftime getcwd getexecname getdtablesize getpid getpagesize getrusage gettimeofday gmtime_r isascii isnan localtime_r lstat madvise memmove memset mkdir munmap mkstemp newloacle _NSGetExecutablePath pclose poll popen posix_fallocate posix_memalign pow pread pwrite raise readdir_r readlink realpath sbrk select strtod strtod_l setvbuf sysconf sigemptyset sigaction spawnvp strlcat strlcpy strcasecmp strncasecmp setlocale strchr strcspn strdup strrchr strspn strstr strtol strtoul times uselocale usleep utime vfprintf vfprintf_l vsprintf vsnprintf vsnprintf_l _wfopen])

#
# Check for clock_gettime().
#
AC_SEARCH_LIBS(clock_gettime, rt,
[
  AC_DEFINE([HAVE_CLOCK_GETTIME],[1],[Define to 1 if you have clock_gettime.])
  AC_MSG_CHECKING([whether clock_gettime supports CLOCK_REALTIME])
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM(
      [[#include <time.h>]],
       [[clockid_t clockType = CLOCK_REALTIME;]]))],
      [
        AC_MSG_NOTICE(yes)
      	AC_DEFINE([HAVE_CLOCK_REALTIME],[1],
          [Define to 1 if clock_gettime supports CLOCK_REALTIME.])
      ],
      AC_MSG_NOTICE(no)
    )
  ],
  [
    AC_CHECK_FUNCS([gettimeofday ftime], [break])
  ]
)
########
#       
# Check for function prototypes
#
########

AC_CHECK_DECLS([pread, pwrite],[],[],[
#include <unistd.h>])

AC_CHECK_DECLS([strlcpy],[],[],[
#include <strings.h>])

AC_CHECK_DECLS([vsnprintf],[],[],[
#include <stdio.h>
#include <stdarg.h>])

########
#
# C++ Support Tests (For Wizard++)
#
########
have_wizard_plus_plus='no'
if test "$with_wizard_plus_plus" = 'yes'; then
 OLIBS="$LIBS"
 LIBS=''
 AC_LANG_PUSH(C++)

# Full set of headers used ...
# algorithm cctype cerrno cmath cstdio cstdlib cstring ctime exception
# functional iomanip iosfwd iostream iterator list string strstream utility
 AC_LANG([C++])
 AC_PROG_CXX
 AX_CXX_BOOL
 AX_CXX_NAMESPACES
 AX_CXX_NAMESPACE_STD
 AC_CXX_HAVE_STD_LIBS
 AC_OPENMP([C++])
 AC_LANG_POP

 AC_MSG_CHECKING([whether C++ compiler is sufficient for the Wizard's Toolkit])
 if \
  test $ac_cv_cxx_have_bool = 'yes' && \
  test $ac_cv_cxx_have_lstring = 'yes' && \
  test $ac_cv_cxx_have_namespaces = 'yes' && \
  test $ac_cv_cxx_have_std_libs = 'yes' && \
  test $ac_cv_cxx_have_std_namespace = 'yes'; then
    have_wizard_plus_plus='yes'
 else
    have_wizard_plus_plus='no (failed tests)'
 fi
 AC_MSG_NOTICE([$have_wizard_plus_plus])
 LIBS="$OLIBS"
fi
AM_CONDITIONAL(WITH_WIZARD_PLUS_PLUS, test "$have_wizard_plus_plus" = 'yes')

# Only check for delegate libraries in subdirectories if requested.
if test "$with_delegate_build" != 'no'; then
  # Check for delegate sub-directories and add -I & -L options as required.
  # This presumes that delegates are installed as detailed in the Wizard's
  # Toolkit README. If delegates are installed in a standard location where the
  # compiler will automatically find them then these options should not be
  # required.

  #
  # Most delegates have includes in the same directory as the library, but not all ...
  #
  # Includes
  for dir in bzlib lcms zlib
  do
    if test -d "$builddir/$dir"; then
      CPPFLAGS="$CPPFLAGS -I$builddir/$dir"
    else
      if test -d "$srcdirfull/$dir"; then
        CPPFLAGS="$CPPFLAGS -I$srcdirfull/$dir"
      fi
    fi
  done

  # Libraries
  for dir in bzlib lcms zlib
  do
    if test -d "$builddir/$dir/.libs"; then
      LDFLAGS="$LDFLAGS -L$builddir/$dir/.libs"
    else
      if test -d "$srcdirfull/$dir/.libs"; then
        LDFLAGS="$LDFLAGS -L$srcdirfull/$dir/.libs"
      fi
    fi
    if test -d "$builddir/$dir"; then
      LDFLAGS="$LDFLAGS -L$builddir/$dir"
    else
      if test -d "$srcdirfull/$dir"; then
        LDFLAGS="$LDFLAGS -L$srcdirfull/$dir"
      fi
    fi
  done
fi

# Assume that delegate headers reside under same directory as Wizard's
# Toolkit installation prefix.
WIZARD_CPPFLAGS="-I$INCLUDE_DIR/${PACKAGE_NAME}-$WIZARD_MAJOR_VERSION $WIZARD_CPPFLAGS"

#
# Find OpenMP library
#
GOMP_LIBS=''
if test "$enable_openmp" != 'no'; then
  if test "${GCC}" = "yes"; then
    # Open64 (passes for GCC but uses different OpenMP implementation)
    if test "x$GOMP_LIBS" = x ; then
      if $CC --version 2>&1 | grep Open64 > /dev/null ; then
        AC_CHECK_LIB(openmp,omp_get_num_procs,GOMP_LIBS="-lopenmp",,)
      fi
    fi
    # GCC
    if test "x$GOMP_LIBS" = x ; then
      AC_CHECK_LIB(gomp,GOMP_parallel_start,GOMP_LIBS="-lgomp",,)
    fi
  else
    # Sun CC
    if test "x$GOMP_LIBS" = x ; then
      AC_CHECK_LIB(mtsk,sunw_mp_register_warn,GOMP_LIBS="-lmtsk",,)
    fi
    # AIX xlc
    if test "x$GOMP_LIBS" = x ; then
    AC_CHECK_LIB(xlsmp,_xlsmpFlush,GOMP_LIBS="-lxlsmp",,)
    fi
    # SGI IRIX 6.5 MIPSpro C/C++
    if test "x$GOMP_LIBS" = x ; then
    AC_CHECK_LIB(mp,mp_destroy,GOMP_LIBS="-lmp",,)
    fi
  fi
  LIBS="$GOMP_LIBS $LIBS"
fi
AC_SUBST(GOMP_LIBS)

#
# Find Posix threads library
#
THREAD_LIBS=''
if test "$with_threads" != 'no' && test "$have_threads" = 'yes'; then

  if test "x$PTHREAD_LIBS" = "x"; then
    case "${host_cpu}-${host_os}" in
      *-freebsd*)
        WIZARD_CHECK_PTHREAD_LIB(c_r,PTHREAD_LIBS=-lc_r) ;;
    esac
  fi

  for lib in pthread pthreads
  do
    if test "x$PTHREAD_LIBS" = "x"; then
      WIZARD_CHECK_PTHREAD_LIB([$lib],[PTHREAD_LIBS=-l$lib])
    fi
  done

  THREAD_LIBS="$PTHREAD_LIBS"
  LIBS="$LIBS $THREAD_LIBS"
fi
AC_SUBST(THREAD_LIBS)

#
# Check for jemalloc.
#
have_jemalloc='no'
JEMALLOC_LIBS=''
if test "$with_jemalloc" != 'no'; then
  AC_MSG_CHECKING(for JEMALLOC support )
  AC_MSG_RESULT()
  failed=0
  passed=0
  AC_CHECK_HEADER([jemalloc/jemalloc.h],[passed=`expr $passed + 1`,failed=`expr $failed + 1`])
  AC_CHECK_LIB(jemalloc,[malloc_stats_print],[passed=`expr $passed + 1`,failed=`expr $failed + 1`],)
  AC_MSG_CHECKING(if jemalloc memory allocation library is complete)
  if test $passed -gt 0; then
    if test $failed -gt 0; then
      AC_MSG_RESULT(no -- some components failed test)
      have_jemalloc='no (failed tests)'
    else
      JEMALLOC_LIBS='-ljemalloc'
      LIBS="$JEMALLOC_LIBS $LIBS"
      AC_DEFINE(HasJEMALLOC,1,Define if you have jemalloc memory allocation library)
      AC_MSG_RESULT(yes)
      have_jemalloc='yes'
    fi
  else
    AC_MSG_RESULT(no)
  fi
fi
AM_CONDITIONAL(HasJEMALLOC, test "$have_jemalloc" = 'yes')
AC_SUBST(JEMALLOC_LIBS)

#
# Check for umem.
#
have_umem='no'
UMEM_LIBS=''
if test "$with_umem" != 'no'; then
  AC_MSG_CHECKING(for UMEM support )
  AC_MSG_NOTICE()
  failed=0
  passed=0
  AC_CHECK_HEADER(umem.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
  AC_CHECK_LIB(umem,umem_alloc,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(umem,umem_free,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_MSG_CHECKING(if umem memory allocation library is complete)
  if test $passed -gt 0; then
    if test $failed -gt 0; then
      AC_MSG_NOTICE(no -- some components failed test)
      have_umem='no (failed tests)'
    else
      UMEM_LIBS='-lumem'
      LIBS="$UMEM_LIBS $LIBS"
      AC_DEFINE(HasUMEM,1,Define if you have umem memory allocation library)
      AC_MSG_NOTICE(yes)
      have_umem='yes'
    fi
  else
    AC_MSG_NOTICE(no)
  fi
fi
AM_CONDITIONAL(HasUMEM, test "$have_umem" = 'yes')
AC_SUBST(UMEM_LIBS)

#
# Add support for ccmalloc memory debugging library if requested
#
have_ccmalloc='no'
CCMALLOC_LIBS=''
if test "$enable_ccmalloc" = 'yes'; then
  AC_PATH_PROG(CCMALLOCDelegate,ccmalloc,)
  if test -n "$CCMALLOCDelegate"; then
    eval `grep PREFIX= $CCMALLOCDelegate | sed -e 's/PREFIX/CCMALLOC_PREFIX/'`
    OLIBS="$LIBS"
    # Assume that gcc is used with ccmalloc.
    LIBS="$LIBS $CCMALLOC_PREFIX/lib/ccmalloc-gcc.o"
    AC_CHECK_LIB(ccmalloc,ccmalloc_malloc,CCMALLOC_LIBS="$CCMALLOC_PREFIX/lib/ccmalloc-gcc.o -lccmalloc -ldl",,-ldl)
    if test -n "$CCMALLOC_LIBS"; then
      LIBS="$OLIBS"
      LIBS="$LIBS $CCMALLOC_LIBS"
      have_ccmalloc='yes'
     else
      LIBS="$OLIBS"
    fi
  fi
fi

#
# Add support for 'huge pages' if requested
#
AS_IF([test "x$enable_hugepages" != "xno"],[
  AC_DEFINE(HAVE_HUGEPAGES, 1, [Compile with hugepage support])
])

#
# Add support for efence memory debugging library if requested
#
if test "$enable_efence" = 'yes'; then
  EFENCE_LIBS='-lefence'
  LIBS="$EFENCE_LIBS $LIBS"
fi

#
# Find math library
#
MATH_LIBS=''
AC_CHECK_LIB(m,sqrt,MATH_LIBS="-lm",,)
LIBS="$MATH_LIBS $LIBS"
AC_SUBST(MATH_LIBS)

dnl ===========================================================================

# Encode the this name into the shared library.
AC_ARG_WITH([package-release-name],
    [AC_HELP_STRING([--with-package-release-name=NAME],
                    [encode this name into the shared library])],
[WIZARD_LT_RELEASE_OPTS="-release $withval"])
AC_SUBST(WIZARD_LT_RELEASE_OPTS)


#
# Check for BZLIB
#

AC_ARG_WITH([bzlib],
    [AC_HELP_STRING([--without-bzlib],                    [disable BZLIB support])],
    [with_bzlib=$withval],
    [with_bzlib='yes'])

if test "$with_bzlib" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-bzlib=$with_bzlib "
fi

have_bzlib='no'
if test "$with_bzlib" != 'no'; then
    BZLIB_LIBS=''
    AC_MSG_NOTICE([-------------------------------------------------------------])
    AC_MSG_CHECKING(for BZLIB)
    AC_MSG_NOTICE([])
    failed=0
    passed=0
    found_libbz=0
    AC_CHECK_HEADER(bzlib.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_LIB(bz2,BZ2_bzDecompress,found_libbz=`expr $found_libbz + 1`,,)
    if test "$native_win32_build" = 'yes'; then
      AC_CHECK_LIB(bz2,_imp__BZ2_decompress,found_libbz=`expr $found_libbz + 1`,,)
    fi
    if test $found_libbz -gt 0; then
      passed=`expr $passed + 1`
    else
      failed=`expr $failed + 1`
    fi
    AC_MSG_CHECKING(if BZLIB package is complete)
    if test $passed -gt 0; then
      if test $failed -gt 0; then
        AC_MSG_NOTICE(no -- some components failed test)
        have_bzlib='no (failed tests)'
      else
        BZLIB_LIBS='-lbz2'
        LIBS="$BZLIB_LIBS $LIBS"
        AC_DEFINE(BZLIB_DELEGATE,1,Define if you have the bzip2 library)
        AC_MSG_NOTICE(yes)
        have_bzlib='yes'
      fi
    else
      AC_MSG_NOTICE(no)
    fi
fi
AM_CONDITIONAL(BZLIB_DELEGATE, test "$have_bzlib" = 'yes')
AC_SUBST(BZLIB_LIBS)

dnl ===========================================================================

#
# Check for LZMA
#
AC_ARG_WITH(lzma,
	      [  --without-lzma          disable LZMA support],
	      [with_lzma=$withval],
	      [with_lzma='yes'])
if test "$with_lzma" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-lzma=$with_lzma "
fi

LZMA_PKG=""
if test "x$with_lzma" = "xyes"; then
    AC_MSG_NOTICE([-------------------------------------------------------------])
    PKG_CHECK_MODULES(LZMA,[liblzma >= 2.9.0], have_lzma=yes, have_lzma=no)
    AC_MSG_NOTICE([])
fi

if test "$have_lzma" = 'yes'; then
    AC_DEFINE(LZMA_DELEGATE,1,Define if you have LZMA library)
    if test "$with_modules" = 'no'; then
        CPPFLAGS="$LZMA_CFLAGS $CPPFLAGS"
    fi
fi

AM_CONDITIONAL(LZMA_DELEGATE, test "$have_lzma" = 'yes')
AC_SUBST(LZMA_CFLAGS)
AC_SUBST(LZMA_LIBS)

dnl ===========================================================================

#
# Check for ZLIB
#

AC_ARG_WITH([zlib],
    [AC_HELP_STRING([--without-zlib],
                    [disable ZLIB support])],
    [with_zlib=$withval],
    [with_zlib='yes'])

if test "$with_zlib" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-zlib=$with_zlib "
fi

have_zlib='no'
ZLIB_LIBS=''
dnl PNG requires zlib so enable zlib check if PNG is requested
if test "$with_zlib" != 'no' || test "$with_png" != 'no'; then
  AC_MSG_NOTICE([-------------------------------------------------------------])
  AC_MSG_CHECKING(for ZLIB)
  AC_MSG_NOTICE([])
  failed=0
  passed=0
  AC_CHECK_HEADER(zconf.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
  AC_CHECK_HEADER(zlib.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
  AC_CHECK_LIB(z,compress,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,uncompress,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,deflate,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,deflateBound,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,inflate,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,gzseek,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(z,gztell,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_MSG_CHECKING([if ZLIB package is complete])
  if test $passed -gt 0; then
    if test $failed -gt 0; then
      AC_MSG_NOTICE([no -- some components failed test])
      have_zlib='no (failed tests)'
    else
      ZLIB_LIBS='-lz'
      LIBS="$ZLIB_LIBS $LIBS"
      AC_DEFINE(ZLIB_DELEGATE,1,Define if you have zlib compression library)
      AC_MSG_NOTICE([yes])
      have_zlib='yes'
    fi
  else
    AC_MSG_NOTICE([no])
  fi
fi
AM_CONDITIONAL(ZLIB_DELEGATE, test "$have_zlib" = 'yes')
AC_SUBST(ZLIB_LIBS)

#
# If profiling, then check for -ldl and dlopen (required for Solaris & gcc)
#
LIB_DL=''
if test "$with_profiling" = 'yes'; then
  AC_CHECK_LIB(dl,dlopen,LIB_DL='-ldl',,)
  LIBS="$LIB_DL $LIBS"
fi
AC_SUBST(LIB_DL)

#
# Configure install Paths
#

# Path to WizardsToolkit header files
INCLUDE_RELATIVE_PATH="${PACKAGE_NAME}-${WIZARD_MAJOR_VERSION}"
INCLUDE_PATH="${INCLUDEARCH_DIR}/${INCLUDE_RELATIVE_PATH}"
DEFINE_INCLUDE_PATH="${INCLUDEARCH_DIR}/${INCLUDE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_INCLUDE_PATH=`$WinPathScript "$DEFINE_INCLUDE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(INCLUDE_PATH,"$DEFINE_INCLUDE_PATH",Directory where WizardsToolkit headers live.)
AC_SUBST(INCLUDE_PATH)

# Path to Wizards Toolkit header files (arch part)
INCLUDEARCH_PATH="${INCLUDEARCH_DIR}/${INCLUDE_RELATIVE_PATH}"
DEFINE_INCLUDEARCH_PATH="${INCLUDEARCH_DIR}/${INCLUDE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_INCLUDEARCH_PATH=`$WinPathScript "$DEFINE_INCLUDEARCH_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(INCLUDEARCH_PATH,"$DEFINE_INCLUDE_PATH",[Directory where Wizards Toolkit architecture headers live.])
AC_SUBST(INCLUDEARCH_PATH)

# Subdirectory under lib to place Wizard lib files
LIBRARY_RELATIVE_PATH="${PACKAGE_NAME}-${PACKAGE_VERSION}"
AC_DEFINE_UNQUOTED(LIBRARY_RELATIVE_PATH,"$LIBRARY_RELATIVE_PATH",Subdirectory of lib where Wizard architecture dependent files are installed)

# Path to Wizard bin directory
EXECUTABLE_PATH="${BIN_DIR}"
DEFINE_EXECUTABLE_PATH="${BIN_DIR}/"
case "${build_os}" in
  mingw* )
    DEFINE_EXECUTABLE_PATH=`$WinPathScript "$DEFINE_EXECUTABLE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(EXECUTABLE_PATH,"$DEFINE_EXECUTABLE_PATH",Directory where executables are installed.)
AC_SUBST(EXECUTABLE_PATH)

# Path to Wizard lib
LIBRARY_PATH="${LIB_DIR}/${LIBRARY_RELATIVE_PATH}"
DEFINE_LIBRARY_PATH="${LIB_DIR}/${LIBRARY_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_LIBRARY_PATH=`$WinPathScript "$DEFINE_LIBRARY_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(LIBRARY_PATH,"$DEFINE_LIBRARY_PATH",Directory where architecture-dependent files live.)
AC_SUBST(LIBRARY_PATH)

#         
# Path to Wizard document files
DOCUMENTATION_RELATIVE_PATH="${WIZARD_MAJOR_VERSION}"
DOCUMENTATION_PATH="${DOC_DIR}-${DOCUMENTATION_RELATIVE_PATH}"
DEFINE_DOCUMENTATION_PATH="${DOC_DIR}-${DOCUMENTATION_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_DOCUMENTATION_PATH=`$WinPathScript "$DEFINE_DOCUMENTATION_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(DOCUMENTATION_PATH,"$DEFINE_DOCUMENTATION_PATH","Directory where the Wizard's Toolkit documents live.") 
AC_SUBST(DOCUMENTATION_PATH)

# Subdirectory under lib to place Wizard configuration files
CONFIGURE_RELATIVE_PATH="${PACKAGE_NAME}-${WIZARD_MAJOR_VERSION}"
AC_DEFINE_UNQUOTED(CONFIGURE_RELATIVE_PATH,"$CONFIGURE_RELATIVE_PATH",Subdirectory of lib where architecture-dependent configuration files live.)
CONFIGURE_PATH="${SYSCONF_DIR}/${CONFIGURE_RELATIVE_PATH}"
DEFINE_CONFIGURE_PATH="${SYSCONF_DIR}/${CONFIGURE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_CONFIGURE_PATH=`$WinPathScript "$DEFINE_CONFIGURE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(CONFIGURE_PATH,"$DEFINE_CONFIGURE_PATH",Directory where architecture-dependent configuration files live.)
AC_SUBST(CONFIGURE_PATH)

# Subdirectory under share to place Wizard configuration files
SHARE_RELATIVE_PATH="${PACKAGE_NAME}-${WIZARD_MAJOR_VERSION}"
AC_DEFINE_UNQUOTED(SHARE_RELATIVE_PATH,"$SHARE_RELATIVE_PATH",Subdirectory of lib where architecture-independent configuration files live.)
SHARE_PATH="${DATA_DIR}/${SHARE_RELATIVE_PATH}"
DEFINE_SHARE_PATH="${DATA_DIR}/${SHARE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_SHARE_PATH=`$WinPathScript "$DEFINE_SHARE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(SHARE_PATH,"$DEFINE_SHARE_PATH",Directory where architecture-independent configuration files live.)
AC_SUBST(SHARE_PATH)

# Subdirectory under share to place Wizard configuration files
if test "x$SHAREARCH_BASEDIRNAME" = "x"; then
   SHAREARCH_BASEDIRNAME="config"
fi
AC_DEFINE_UNQUOTED(SHAREARCH_BASEDIRNAME,"$SHAREARCH_BASEDIRNAME",[Sharearch directory name without ABI part.])
AC_SUBST(SHAREARCH_BASEDIRNAME)

if test "x$SHAREARCH_DIRNAME" = "x"; then
  SHAREARCH_DIRNAME=${SHAREARCH_BASEDIRNAME}-${WIZARD_ABI_SUFFIX}
else
  AC_DEFINE_UNQUOTED(SHAREARCH_DIRNAME,"$SHAREARCH_DIRNAME",[Sharearch directory dirname])
fi
AC_SUBST(SHAREARCH_DIRNAME)

SHAREARCH_RELATIVE_PATH="${SHAREARCH_DIRNAME}"
AC_SUBST(SHAREARCH_RELATIVE_PATH)
SHAREARCH_PATH="${LIBRARY_PATH}/${SHAREARCH_RELATIVE_PATH}"
AC_SUBST(SHAREARCH_PATH)

#
# program_transform_name is formed for use in a Makefile, so create a
# modified version for use in a shell script.
configure_transform_name=`echo ${program_transform_name} | sed 's,\\$\\$,$,'`

# Default delegate definitions
BrowseDelegateDefault='mozilla'
EchoDelegateDefault='echo'
LPDelegateDefault='lp'
LPRDelegateDefault='lpr'
MANDelegateDefault='groff'
MVDelegateDefault='mv'
RMDelegateDefault='rm'
TXTDelegateDefault='enscript'
WWWDecodeDelegateDefault='wget'
ZipDelegateDefault='gzip'

#
# Test for font directories
#
type_include_files=''

# Windows
windows_font_dir=''
if test "$with_windows_font_dir" != "yes" && test -n "$with_windows_font_dir"; then
   windows_font_dir="${with_windows_font_dir}/"
fi
if test -n "$windows_font_dir"; then
  if test -f '/usr/X11R6/lib/X11/fonts/truetype/arial.ttf'; then
     windows_font_dir='/usr/X11R6/lib/X11/fonts/truetype/'
  fi
  if test -f '/usr/X11R7/lib/X11/fonts/truetype/arial.ttf'; then
     windows_font_dir='/usr/X11R7/lib/X11/fonts/truetype/'
  fi
  if test -f '/usr/share/fonts/msttcore/arial.ttf'; then
     windows_font_dir='/usr/share/fonts/msttcore/truetype/'
  fi
fi
if test -n "$windows_font_dir"; then
  type_include_files="$type_include_files "'<include file="type-windows.mgk" />'
fi
AC_SUBST(windows_font_dir)

# Adobe Postscript fonts on various systems
case $host_os in
  solaris*) type_include_files="$type_include_files "'<include file="type-solaris.mgk" />';;
esac

#
# RPM support.
#
RPM=''
AC_CHECK_PROGS(TAR,gnutar gtar tar)
AC_CHECK_PROGS(RPM,rpmbuild rpm)
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AC_SUBST(RPM)
AM_CONDITIONAL(RPM_DELEGATE, test "x$RPM" != "x" )

#
# 7ZIP support (http://p7zip.sourceforge.net/)
#
P7ZIP=''
AC_CHECK_PROGS(P7ZIP,[7za])
AC_SUBST(P7ZIP)
AM_CONDITIONAL(P7ZIP_DELEGATE, test "x$P7ZIP" != "x" )

#
# ZIP support (http://www.info-zip.org/Zip.html)
#
ZIP=''
AC_CHECK_PROGS(ZIP,[zip])
AC_SUBST(ZIP)
AM_CONDITIONAL(ZIP_DELEGATE, test "x$ZIP" != "x" )

#
# Handle special compiler flags
#

# Add '-p' if prof source profiling support enabled
if test "$enable_prof" = 'yes'; then
  CFLAGS="-p $CFLAGS"
  CXXFLAGS="-p $CXXFLAGS"
  LDFLAGS="-p $LDFLAGS"
fi

# Add '-pg' if gprof source profiling support enabled
if test "$enable_gprof" = 'yes'; then
  CFLAGS="-pg $CFLAGS"
  CXXFLAGS="-pg $CXXFLAGS"
  LDFLAGS="-pg $LDFLAGS"
fi

# Add '-ftest-coverage -fprofile-arcs' if gcov source profiling support enabled
# This is a gcc-specific feature
if test "$enable_gcov" = 'yes'; then
  CFLAGS="-ftest-coverage -fprofile-arcs  $CFLAGS"
  CXXFLAGS="-ftest-coverage -fprofile-arcs  $CXXFLAGS"
  LDFLAGS="-ftest-coverage -fprofile-arcs $LDFLAGS"
fi

#
# Build library dependency list for libWizard
#

if test "$with_modules" != 'no'; then
  WIZARD_DEP_LIBS="$USER_LIBS $IPC_LIBS $BZLIB_LIBS $LZMA_LIBS $ZLIB_LIBS $GDI32_LIBS $MATH_LIBS $GOMP_LIBS $UMEM_LIBS $JEMALLOC_LIBS $THREAD_LIBS"
else
  WIZARD_DEP_LIBS="$USER_LIBS $IPC_LIBS $BZLIB_LIBS $LZMA_LIBS $ZLIB_LIBS $GDI32_LIBS $MATH_LIBS $UMEM_LIBS $GOMP_LIBS $UMEM_LIBS $JEMALLOC_LIBS $THREAD_LIBS"
fi
AC_SUBST(WIZARD_DEP_LIBS)

#
# Remove extraneous spaces from output variables (asthetic)
#
X_CFLAGS=`echo $X_CFLAGS | sed -e 's/  */ /g'`
X_PRE_LIBS=`echo $X_PRE_LIBS | sed -e 's/  */ /g'`
X_LIBS=`echo $X_LIBS | sed -e 's/  */ /g'`
X_EXTRA_LIBS=`echo $X_EXTRA_LIBS | sed -e 's/  */ /g'`

CC=`echo $CC | sed -e 's/  */ /g'`
CFLAGS=`echo $CFLAGS | sed -e 's/  */ /g'`
CPPFLAGS=`echo $CPPFLAGS | sed -e 's/  */ /g'`
CXXFLAGS=`echo $CXXFLAGS | sed -e 's/  */ /g'`
LDFLAGS=`echo $LDFLAGS | sed -e 's/  */ /g'`
TESTED_LIBS=`echo $LIBS | sed -e 's/  */ /g'`
WIZARD_DEP_LIBS=`echo $WIZARD_DEP_LIBS | sed -e 's/  */ /g'`
#LIBS=`echo $LIBS | sed -e 's/  */ /g'`

# Pass only user-provided LIBS as "global" libraries
LIBS=$USER_LIBS

#AC_SUBST(CPPFLAGS)
AC_SUBST(X_CFLAGS)
#AC_SUBST(LDFLAGS)
#AC_SUBST(X_PRE_LIBS)
#AC_SUBST(X_LIBS)
#AC_SUBST(X_EXTRA_LIBS)

WIZARD_CFLAGS=$CFLAGS
WIZARD_CPPFLAGS=`echo $WIZARD_CPPFLAGS | sed -e 's/  */ /g'`
WIZARD_PCFLAGS=`echo $WIZARD_PCFLAGS | sed -e 's/  */ /g'`
WIZARD_LDFLAGS="-L$LIB_DIR $LDFLAGS"
WIZARD_LIBS="-lWizardsToolkit $WIZARD_DEP_LIBS"

AC_SUBST(WIZARD_CFLAGS)
AC_SUBST(WIZARD_CPPFLAGS)
AC_SUBST(WIZARD_PCFLAGS)
AC_SUBST(WIZARD_LDFLAGS)
AC_SUBST(WIZARD_LIBS)

# Set configured scripts to executable.
AC_CONFIG_COMMANDS([WizardsToolkit-config.in],[chmod +x wizard/WizardsToolkit-config])

AC_OUTPUT

rm -f wizard-version

AC_MSG_NOTICE([
The Wizards Toolkit is configured as follows. Please verify that
this configuration matches your expectations.

Host system type: $host
Build system type: $build

                  Option                        Value
-------------------------------------------------------------------------
Shared libraries  --enable-shared=$enable_shared		$libtool_build_shared_libs
Static libraries  --enable-static=$enable_static		$libtool_build_static_libs
GNU ld            --with-gnu-ld=$with_gnu_ld		$lt_cv_prog_gnu_ld

Delegate Configuration:
BZLIB             --with-bzlib=$with_bzlib               $have_bzlib
LZMA              --with-lzma=$with_lzma                $have_lzma
ZLIB              --with-zlib=$with_zlib                $have_zlib

Options used to compile and link:
  PREFIX      = $PREFIX_DIR
  EXEC-PREFIX = $EXEC_PREFIX_DIR
  VERSION     = $PACKAGE_VERSION
  CC          = $CC
  CFLAGS      = $CFLAGS
  CPPFLAGS    = $CPPFLAGS
  PCFLAGS     = $PCFLAGS
  DEFS        = $DEFS
  LDFLAGS     = $LDFLAGS
  LIBS        = $LIBS
  CXX         = $CXX
  CXXFLAGS    = $CXXFLAGS
])
