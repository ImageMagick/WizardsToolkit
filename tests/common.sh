SRCDIR=`dirname $0`
SRCDIR=`cd $SRCDIR && pwd`
TOPSRCDIR=`cd $srcdir && pwd`
PASSPHRASE="${TOPSRCDIR}/utilities/tests/passphrase.txt"
export SRCDIR TOPSRCDIR
cd tests || exit 1
