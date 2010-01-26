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



# Check installation directory
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



# Copies file $1 from $2 to ./ and checks if it has the md5sum $3
function safe_copy {
	FROM="${2}/${1}"
	TO="./${1}"
	if [ -f "${FROM}" ]; then
		cp "${FROM}" "${1}"
	else
		echo "File ${1} doesn't exist in ${2}"
		return
	fi

	EXPECTED="${3}  -"
	RECEIVED=`md5sum < $TO`
	if [ "${EXPECTED}" != "${RECEIVED}" ]; then
		echo "Warning: file ${1} is corrupted"
	fi
}



# Copy needed files
safe_copy "fstyle.sty" "${1}/data" "70f7f4b491a2607e00651cf3a08e378e"



# Finish work
echo "All files copied successfully :-)"




