#!/bin/bash
#
# Copies the needed files from the GTA2 installation base directory to this
# directory
#
# Usage: install-gta2.sh <gta2-installation-base>
#

if [ "" == "${1}" ]; then
	echo "Usage: install-gta2.sh <gta2-installation base>"
	exit 1
fi

# Installationsverzeichnis ueberpruefen
if [ -d "${1}" ]; then
	if [ -e "${1}/gta2.exe" ]; then
		EXPECTED="7bebb537928c7d870cd4130fd18ea6b4  -"
		RECEIVED=`md5sum < $1/gta2.exe`
		if [ "${EXPECTED}" != "${RECEIVED}" ]; then
			echo "Unexpected md5sum ${RECEIVED} of gta2.exe"
			exit 1
		fi
	else
		echo "Missing file gta2.exe in ${1}"
		exit 1
	fi
else
	echo "${1} is not a directory"
	exit 1
fi


