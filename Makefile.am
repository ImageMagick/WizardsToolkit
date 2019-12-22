#  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
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
#  Top-Level Makefile for building the Wizard's Toolkit.

topincludedir = @includedir@/wizard

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

ACLOCAL_AMFLAGS = -I m4

# Options to pass when running configure in the distcheck target.
#
# We want to preserve user-provided option variables so the same
# compiler, headers, and libraries are used as for a normal build.
DISTCHECK_CONFIGURE_FLAGS=$(DISTCHECK_CONFIG_FLAGS)

DISTCLEANFILES = _configs.sed wizard/wizard-config.h

## Make sure these will be cleaned even when they're not built by default.
CLEANFILES = \
	$(BZIP_CLEANFILES) \
	$(ZIP_CLEANFILES) \
	$(TESTS_CLEANFILES) \
	$(UTILITIES_CLEANFILES)

bin_PROGRAMS = \
	$(UTILITIES_PGMS)

# Binary scripts
bin_SCRIPTS = \
	$(WIZARD_BIN_SCRPTS) 

include_HEADERS =

# Headers which are not installed but which are distributed
noinst_HEADERS = \
	$(BZIP_LIBS_NOINST_HDRS) \
	$(ZIP_LIBS_NOINST_HDRS) \
	$(WIZARD_NOINST_HDRS)

if WIN32_NATIVE_BUILD
SRCDIR='$(shell @WinPathScript@ $(srcdir)/)'
else
SRCDIR="$(srcdir)/"
endif

# Tests with .tap extensions use the TAP protocol and TAP driver
LOG_COMPILER = $(SHELL)

SH_LOG_COMPILER = $(LOG_COMPILER)
TAP_LOG_COMPILER = $(LOG_COMPILER)

# Test extensions
TEST_EXTENSIONS = .sh .tap

TAP_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/config/tap-driver.sh

# Environment parameters to be used during tests
TESTS_ENVIRONMENT = \
	MAKE="$(MAKE)" \
	MAKEFLAGS="$(MAKEFLAGS)" \
	MEMCHECK="$(MEMCHECK)"

# Tests to run
TESTS = \
	$(TESTS_TESTS) \
	$(UTILITIES_TESTS)

# Tests which are expected to fail
XFAIL_TESTS = \
	$(TESTS_XFAIL_TESTS)

TOP_EXTRA_DIST = \
  AUTHORS.txt \
	README.txt \
	QuickStart.txt \
	ChangeLog \
	LICENSE \
	NOTICE \
	NEWS.txt \
	index.html \
	version.sh \
	winpath.sh \
	wizard.sh.in

# Additional files to distribute
EXTRA_DIST = \
	$(TOP_EXTRA_DIST) \
	$(CONFIG_EXTRA_DIST) \
	$(WIZARD_EXTRA_DIST) \
	$(BZIP_EXTRA_DIST) \
	$(ZIP_EXTRA_DIST) \
	$(TESTS_EXTRA_DIST) \
	$(UTILITIES_EXTRA_DIST) 

lib_LTLIBRARIES = $(WIZARD_LIBS) $(BZIP_LIBS) $(ZIP_LIBS)

check_PROGRAMS = \
	$(TESTS_CHECK_PGRMS) \
	$(ZIP_CHECK_PGRMS)

include m4/Makefile.am
include config/Makefile.am
include wizard/Makefile.am
include tests/Makefile.am
include utilities/Makefile.am

# Pkgconfig directory
pkgconfigdir = $(libdir)/pkgconfig

# Files to install in Pkgconfig directory
pkgconfig_DATA = \
	$(WIZARD_PKGCONFIG)

# Manual pages to install
man_MANS = \
	$(WIZARD_MANS) \
	$(UTILITIES_MANS)

# Architecture independent data files installed in the package's data directory
docdir = $(DOCUMENTATION_PATH)
doc_DATA = \
	LICENSE \
	ChangeLog \
	NEWS.txt

if MAINTAINER_MODE
MAINTAINER_TARGETS = wizard-version $(RST2HTML_TARGETS)
endif

all-local: $(BZIP_LOCAL_TARGETS) $(ZIP_LOCAL_TARGETS) $(MAINTAINER_TARGETS)

install-exec-local:

install-data-local: $(HTML_INSTALL_DATA_TARGETS)

uninstall-local: $(HTML_UNINSTALL_DATA_TARGETS)

clean-local: 

distclean-local: 

maintainer-clean-local: 

check-local: 

# drd: valgrind's newer thread error detector
drd:
	$(MAKE) MEMCHECK='valgrind --tool=drd --check-stack-var=yes --var-info=yes \
	  --quiet $(VALGRIND_EXTRA_OPTS)' check

# helgrind: valgrind's older thread error detector
helgrind:
	$(MAKE) MEMCHECK='valgrind --tool=helgrind --error-exitcode=2 --quiet \
	$(VALGRIND_EXTRA_OPTS)' check

# memcheck: valgrind's memory access checker
memcheck:
	$(MAKE) MEMCHECK='valgrind --tool=memcheck --leak-check=full --read-var-info=yes \
	--error-exitcode=2 --track-origins=yes --num-callers=12 \
	--quiet $(VALGRIND_EXTRA_OPTS)' check

# ptrcheck: valgrind's experimental pointer checking tool.
ptrcheck:
	$(MAKE) MEMCHECK='valgrind --tool=exp-ptrcheck --quiet $(VALGRIND_EXTRA_OPTS)' check

# Non-Automake subdirectories to distribute
DISTDIRS = images scripts windows www
dist-hook:
	( \
	  builddir=`pwd` ; \
	  cd $(srcdir) && \
	  ( \
	    for dir in $(DISTDIRS) ; do \
	      find $$dir -depth -print | egrep -v '(~$$)|(/CVS)|(/\.#)|(/\.deps)|(/\.svn)' \
	        | cpio -pdum $$builddir/$(distdir) 2> /dev/null ; \
	    done \
	  ) \
	)

#
# Additional install rules
#

# Install HTML files
pkgdocdir = $(DOCUMENTATION_PATH)
DOCDIRS = images www www/api www/api/WizardsToolkit
HTML_INSTALL_DATA_TARGETS = install-data-html
install-data-html:
	$(mkinstalldirs) $(DESTDIR)$(pkgdocdir)
	$(INSTALL_DATA) $(srcdir)/index.html $(DESTDIR)$(pkgdocdir)
	@for dir in $(DOCDIRS) ; do \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$$dir && \
	  for file in $(srcdir)/$$dir/*.* ; do \
	    echo "$(INSTALL_DATA) $$file $(DESTDIR)$(pkgdocdir)/$$dir" ; \
	    $(INSTALL_DATA) "$$file" $(DESTDIR)$(pkgdocdir)/$$dir ; \
	  done ; \
	done

# Uninstall HTML files
HTML_UNINSTALL_DATA_TARGETS = uninstall-data-html
uninstall-data-html:
	rm -f $(DESTDIR)$(pkgdocdir)/index.html
	for dir in $(DOCDIRS) ; do \
	  rm -f -r $(DESTDIR)$(pkgdocdir)/$$dir ; \
	done

# Ensure that version.h at $(srcdir)/wizard/version.h is kept up to date.
wizard-version: wizard/version.h
	@if test -f "$(srcdir)/windows/installer/scripts/version.isx.in" ; then \
	  ./config.status --file="$(srcdir)/windows/installer/scripts/version.isx" 2> /dev/null ; \
	fi
	@if test -n "$(VPATH)" ; then \
	  cmp wizard/version.h $(srcdir)/wizard/version.h > /dev/null ; \
          if test $$? -eq 1 ; then \
	    echo "Updating $(srcdir)/wizard/version.h ..."; \
	    cp wizard/version.h $(srcdir)/wizard/version.h ; \
	  fi ; \
	fi ; \
	touch wizard-version

${srcdir}/configure: ${srcdir}/version.sh

# Automatically reconfigure libtool
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# Format C API documentation
html-local:
	doxygen config/WizardsToolkit.dox

#
# RPM build support
#
if RPM_DELEGATE

DIST_ARCHIVE_SRPM=$(distdir)-1.src.rpm
.PHONY: srpm
$(DIST_ARCHIVE_SRPM) srpm: dist
	rm -f $(DIST_ARCHIVE_SRPM)
	$(RPM) --define="_sourcedir `pwd`" --define="_srcrpmdir `pwd`" --nodeps --bs WizardsToolkit.spec
	@echo ==============================================================
	@echo $(DIST_ARCHIVE_SRPM) is ready for distribution.
	@echo ==============================================================

RPMDIR=rpmbuild
RPMARCH=$(WIZARD_TARGET_CPU)

DIST_ARCHIVE_RPM= \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-devel-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm

.PHONY: rpm
rpm: dist
	rm -rf $(RPMDIR)
	$(mkinstalldirs) $(RPMDIR)
	$(mkinstalldirs) $(RPMDIR)/BUILD
	$(mkinstalldirs) $(RPMDIR)/RPMS
	$(RPM) --define="_sourcedir `pwd`" --define="_rpmdir `pwd`/$(RPMDIR)/RPMS" --define="_builddir `pwd`/$(RPMDIR)/BUILD" --nodeps -bb WizardsToolkit.spec
	@echo ==============================================================
	@echo $(DIST_ARCHIVE_RPM) is ready for distribution.
	@echo ==============================================================

else
DIST_ARCHIVE_RPM=
endif # RPM_DELEGATE

#
# Build a validated snapshot release and move to the snapshots directory.
#
snapshot: distcheck
	$(MAKE) $(DIST_ARCHIVE_SRPM)
	$(MAKE) $(DIST_WINDOWS_SRC_ZIP)
	mv -f $(DIST_ARCHIVES) $(DIST_WINDOWS_SRC_ZIP) $(DIST_ARCHIVE_SRPM) /var/ftp/pub/WizardsToolkit/beta/
