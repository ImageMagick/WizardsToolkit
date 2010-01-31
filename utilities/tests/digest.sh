#!/bin/sh
# Copyright (C) 1999-2010 ImageMagick Studio LLC
#
# This program is covered by multiple licenses, which are described in
# LICENSE. You should have received a copy of LICENSE with this
# package; otherwise see http://www.wizards-toolkit.org/script/license.php.
#
#  Test for '${DIGEST}' utility.
#

set -e # Exit on any error
. ${srcdir}/utilities/tests/common.sh

${DIGEST} ${PLAINTEXT} ${PLANETEXT} ${DIGESTRDF}
${DIGEST} -authenticate ${DIGESTRDF} -
