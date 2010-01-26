#!/bin/bash
#
# Verifies and unpacks the needed contents of GTA2INSTALLER.ZIP
#
INSTALLER="GTA2INSTALLER.ZIP"
MD5SUM="4bf0b5f995d659090b681dd7b410499e  ${INSTALLER}"
SHA512SUM="4377285b70b101c70d401b8addfbfb4d7639825d46e81d127ca8a6c83ef97bcc060610447abeb166718eed1dee27172a955e3cf2cac015bb813537cb6b7d3764  ${INSTALLER}"



# Does the file exist?
if [ -f "${INSTALLER}" ]; then
	echo "Verifing and unpacking ${INSTALLER}"
else
	echo "You need to download ${INSTALLER} and copy it into `pwd`"
	echo "If you don't know what this means, read read_this.txt"
	exit 1
fi



# Verify archive
echo -n "Verifing md5sum..."
SUM=`md5sum $INSTALLER`
if [ "${MD5SUM}" != "${SUM}" ]; then
	echo "Archive ${INSTALLER} corrupted"
	echo "\tExpected: \"${MD5SUM}\""
	echo "\tReceived: \"${SUM}\""
	exit 1
fi
echo " ok"

echo -n "Verifing sha512sum..."
SUM=`sha512sum $INSTALLER`
if [ "${SHA512SUM}" != "${SUM}" ]; then
	echo "Archive ${INSTALLER} corrupted"
	echo "\tExpected: \"${SHA512SUM}\""
	echo "\tReceived: \"${SUM}\""
	exit 1
fi
echo " ok"


echo "ok"



