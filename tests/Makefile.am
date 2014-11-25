# Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
# dedicated to making software imaging solutions freely available.
#
# You may not use this file except in compliance with the License.
# obtain a copy of the License at
#
#   http://www.wizards-toolkit.org/script/license.php
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#  Makefile for Wizard's Toolkit tests invoked via the C API.

TESTS_CHECK_PGRMS = \
	tests/validate

tests_validate_SOURCES = tests/validate.c tests/validate.h
tests_validate_CPPFLAGS = $(AM_CPPFLAGS)
tests_validate_LDADD   = $(WIZARD_LIBS)

TESTS_XFAIL_TESTS = 

TESTS_TESTS = \
	tests/validate.tap

TESTS_EXTRA_DIST = \
	tests/common.shi \
	tests/passphrase.txt \
	$(TESTS_TESTS)

TESTS_CLEANFILES = 
