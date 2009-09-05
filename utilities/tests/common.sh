SRCDIR=`dirname $0`
SRCDIR=`cd $SRCDIR && pwd`
TOPSRCDIR=`cd $srcdir && pwd`
export SRCDIR TOPSRCDIR
CIPHERTEXT="${TOPSRCDIR}/utilities/tests/README.cip"
DIGESTRDF="${TOPSRCDIR}/utilities/tests/digest.rdf"
KEYRINGRDF="${TOPSRCDIR}/utilities/tests/keyring.rdf"
MYKEYRING="${TOPSRCDIR}/utilities/tests/keyring.xdm"
PASSPHRASE="${TOPSRCDIR}/utilities/tests/passphrase.txt"
PLAINTEXT="${TOPSRCDIR}/utilities/tests/README.txt"
PLANETEXT="${TOPSRCDIR}/utilities/tests/README.txt~"
mkdir -p utilities/tests
cd utilities/tests || exit 1
