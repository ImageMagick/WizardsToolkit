#  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
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
#  Makefile for building Wizard's Toolkit utilities.

UTILITIES_PGMS = \
	utilities/encipher \
	utilities/decipher \
	utilities/cipher-info \
	utilities/keyring \
	utilities/digest

utilities_encipher_LDADD = $(WIZARD_LIBS)
utilities_encipher_LDFLAGS = $(LDFLAGS)
utilities_encipher_SOURCES = \
  utilities/encipher.c \
  utilities/content.c \
  utilities/content.h \
  utilities/utility.c \
  utilities/utility_.h

utilities_decipher_LDADD = $(WIZARD_LIBS)
utilities_decipher_LDFLAGS = $(LDFLAGS)
utilities_decipher_SOURCES = \
  utilities/decipher.c \
  utilities/content.c \
  utilities/content.h \
  utilities/utility.c \
  utilities/utility_.h

utilities_cipher_info_LDADD = $(WIZARD_LIBS)
utilities_cipher_info_LDFLAGS = $(LDFLAGS)
utilities_cipher_info_SOURCES = \
  utilities/cipher-info.c \
  utilities/content.c \
  utilities/content.h \
  utilities/utility.c \
  utilities/utility_.h

utilities_keyring_LDADD = $(WIZARD_LIBS)
utilities_keyring_LDFLAGS = $(LDFLAGS)
utilities_keyring_SOURCES = \
  utilities/keyring.c \
  utilities/content.c \
  utilities/content.h \
  utilities/utility.c \
  utilities/utility_.h

utilities_digest_LDADD = $(WIZARD_LIBS)
utilities_digest_LDFLAGS = $(LDFLAGS)
utilities_digest_SOURCES = \
  utilities/digest.c \
  utilities/content.c \
  utilities/content.h \
  utilities/utility.c \
  utilities/utility_.h

# Tests to run
UTILITIES_TESTS = \
	utilities/tests/encipher.tap \
	utilities/tests/cipher-info.tap \
	utilities/tests/decipher.tap \
	utilities/tests/keyring.tap \
	utilities/tests/digest.tap

UTILITIES_MANS = \
	utilities/encipher.1 \
	utilities/decipher.1 \
	utilities/cipher-info.1 \
	utilities/keyring.1 \
	utilities/digest.1

UTILITIES_CONFIGURE = \
	utilities/encipher.1.in \
	utilities/decipher.1.in \
	utilities/cipher-info.1.in \
	utilities/keyring.1.in \
	utilities/digest.1.in

UTILITIES_EXTRA_DIST = \
	$(UTILITIES_TESTS) \
	utilities/tests/common.shi \
	utilities/tests/passphrase.txt \
	utilities/tests/README.txt

UTILITIES_CLEANFILES = \
	utilities/tests/*.cip \
	utilities/tests/*.rdf \
	utilities/tests/*.txt~ \
	utilities/tests/*.xdm
