#  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.
#  obtain a copy of the License at
#
#    https://urban-warrior.org/WizardsToolkit/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for building Wizard's Toolkit library.

wizardincdir = $(INCLUDE_PATH)/wizard

# Headers which are installed
wizardinc_HEADERS = \
	$(WIZARD_INCLUDE_HDRS)

WIZARD_BIN_SCRPTS = \
  wizard/WizardsToolkit-config

WIZARD_PKGCONFIG = \
  wizard/WizardsToolkit.pc

WIZARD_MANS = \
  wizard/WizardsToolkit-config.1

WIZARD_LIBS = wizard/libWizardsToolkit.la

wizard_libWizardsToolkit_la_SOURCES = $(WIZARD_BASE_SRCS) $(WIZARD_PLATFORM_SRCS)
wizard_libWizardsToolkit_la_LIBADD = $(WIZARD_DEP_LIBS)

if HAVE_LD_VERSION_SCRIPT
wizard_libWizardsToolkit_la_LDFLAGS_VERSION =  -Wl,--version-script=$(top_srcdir)/wizard/libWizardsToolkit.map
else
wizard_libWizardsToolkit_la_LDFLAGS_VERSION = -export-symbols-regex ".*"
endif

wizard_libWizardsToolkit_la_LDFLAGS = -no-undefined \
  $(wizard_libWizardsToolkit_la_LDFLAGS_VERSION) \
  $(WIZARD_LT_RELEASE_OPTS) -version-info \
  $(WIZARD_LIBRARY_CURRENT):$(WIZARD_LIBRARY_REVISION):$(WIZARD_LIBRARY_AGE)

# Library base sources
WIZARD_BASE_SRCS = \
  wizard/aes.c \
  wizard/aes.h \
  wizard/authenticate.c \
  wizard/authenticate.h \
  wizard/blob.c \
  wizard/blob.h \
  wizard/blob-private.h \
  wizard/bzip.c \
  wizard/bzip.h \
  wizard/chacha.c \
  wizard/chacha.h \
  wizard/cipher.c \
  wizard/cipher.h \
  wizard/client.c \
  wizard/client.h \
  wizard/configure.c \
  wizard/configure.h \
  wizard/hashmap.h \
  wizard/crc64.c \
  wizard/crc64.h \
  wizard/entropy.c \
  wizard/entropy.h \
  wizard/exception.c \
  wizard/exception.h \
  wizard/exception-private.h \
  wizard/file.c \
  wizard/file.h \
  wizard/hash.c \
  wizard/hash.h \
  wizard/hashmap.c \
  wizard/hashmap.h \
  wizard/hmac.c \
  wizard/hmac.h \
  wizard/key.c \
  wizard/key.h \
  wizard/keyring.c \
  wizard/keyring.h \
  wizard/locale.c \
  wizard/locale_.h \
  wizard/log.c \
  wizard/log.h \
  wizard/lzma.c \
  wizard/lzma.h \
  wizard/magick.c \
  wizard/magick.h \
  wizard/md5.c \
  wizard/md5.h \
  wizard/memory.c \
  wizard/memory_.h \
  wizard/memory-private.h \
  wizard/method-attribute.h \
  wizard/mime.c \
  wizard/mime.h \
  wizard/mime-private.h \
  wizard/nt-base.h \
  wizard/option.c \
  wizard/option.h \
  wizard/passphrase.c \
  wizard/passphrase.h \
  wizard/option.h \
  wizard/random.c \
  wizard/random_.h \
  wizard/resource.c \
  wizard/resource_.h \
  wizard/sanitize.c \
  wizard/sanitize.h \
  wizard/secret.c \
  wizard/secret.h \
  wizard/semaphore.c \
  wizard/semaphore.h \
  wizard/semaphore-private.h \
  wizard/serpent.c \
  wizard/serpent.h \
  wizard/signature.c \
  wizard/signature.h \
  wizard/sha1.c \
  wizard/sha1.h \
  wizard/sha2224.c \
  wizard/sha2224.h \
  wizard/sha2256.c \
  wizard/sha2256.h \
  wizard/sha2384.c \
  wizard/sha2384.h \
  wizard/sha2512.c \
  wizard/sha2512.h \
  wizard/sha3.c \
  wizard/sha3.h \
  wizard/splay-tree.c \
  wizard/splay-tree.h \
  wizard/string.c \
  wizard/string_.h \
  wizard/string-private.h \
  wizard/studio.h \
  wizard/thread.c \
  wizard/thread_.h \
  wizard/thread-private.h \
  wizard/timer.c \
  wizard/timer.h \
  wizard/token.c \
  wizard/token.h \
  wizard/twofish.c \
  wizard/twofish.h \
  wizard/utility.c \
  wizard/utility.h \
  wizard/utility-private.h \
  wizard/version.c \
  wizard/version.h \
  wizard/wizard.c \
  wizard/wizard.h \
  wizard/wizard-type.h \
  wizard/wizard-config.h \
  wizard/WizardsToolkit.h \
  wizard/xml-tree.c \
  wizard/xml-tree.h \
  wizard/xml-tree-private.h \
  wizard/zip.c \
  wizard/zip.h 

if WIN32_NATIVE_BUILD
WIZARD_PLATFORM_SRCS = \
  wizard/nt-base.c \
  wizard/nt-base.h
else
if CYGWIN_BUILD
WIZARD_PLATFORM_SRCS =
else
WIZARD_PLATFORM_SRCS =
endif # if CYGWIN_BUILD
endif # if WIN32_NATIVE_BUILD

WIZARD_INCLUDE_HDRS = \
  wizard/authenticate.h \
  wizard/blob.h \
  wizard/bzip.h \
  wizard/cipher.h \
  wizard/client.h \
  wizard/configure.h \
  wizard/entropy.h \
  wizard/exception.h \
  wizard/file.h \
  wizard/hash.h \
  wizard/hashmap.h \
  wizard/hmac.h \
  wizard/key.h \
  wizard/keyring.h \
  wizard/locale_.h \
  wizard/log.h \
  wizard/lzma.h \
  wizard/magick.h \
  wizard/md5.c \
  wizard/md5.h \
  wizard/memory_.h \
  wizard/method-attribute.h \
  wizard/mime.h \
  wizard/option.h \
  wizard/passphrase.h \
  wizard/random_.h \
  wizard/resource_.h \
  wizard/sanitize.h \
  wizard/secret.h \
  wizard/semaphore.h \
  wizard/splay-tree.h \
  wizard/string_.h \
  wizard/thread_.h \
  wizard/timer.h \
  wizard/token.h \
  wizard/utility.h \
  wizard/version.h \
  wizard/wizard.h \
  wizard/wizard-type.h \
  wizard/wizard-config.h \
  wizard/WizardsToolkit.h \
  wizard/xml-tree.h\
  wizard/zip.h 

WIZARD_NOINST_HDRS = \
  wizard/aes.h \
  wizard/chacha.h \
  wizard/blob-private.h \
  wizard/crc64.h \
  wizard/exception-private.h \
  wizard/memory-private.h \
  wizard/mime-private.h \
  wizard/semaphore-private.h \
  wizard/serpent.h \
  wizard/sha1.h \
  wizard/sha2224.h \
  wizard/sha2256.h \
  wizard/sha2384.h \
  wizard/sha2512.h \
  wizard/sha3.h \
  wizard/string-private.h \
  wizard/studio.h \
  wizard/thread-private.h \
  wizard/twofish.h \
  wizard/utility-private.h \
  wizard/xml-tree-private.h

WIZARD_EXTRA_DIST = \
	wizard/libWizardsToolkit.map \
	wizard/WizardsToolkit-config.in \
	$(WIZARD_MANS) \
	wizard/WizardsToolkit.pc.in
