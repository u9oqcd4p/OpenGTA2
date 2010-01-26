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
safe_copy "wil.sty" "${1}/data" "a7410e70d7d3ef1129d62a0882dfdd6d"
safe_copy "ste.sty" "${1}/data" "aafb64323b2dca7c39e38febc4f86df5"
safe_copy "bil.sty" "${1}/data" "64a073cb3562ff22209500b608f1d803"
safe_copy "wil.gmp" "${1}/data" "07666d575c666cd9d81b79a1b7dc4442"
safe_copy "ste.gmp" "${1}/data" "c2daac310227cc7b71a6f79ebfb8f38c"
safe_copy "bil.gmp" "${1}/data" "4ecd60b40649b1abab1653c171a931c3"
#safe_copy "wil.sdt" "${1}/data" ""
#safe_copy "wil.raw" "${1}/data" ""
#safe_copy "ste.sdt" "${1}/data" ""
#safe_copy "ste.raw" "${1}/data" ""
safe_copy "bil.sdt" "${1}/data/Audio" "e7f1178f12d2bedd0bf73045a7238d1d"
safe_copy "bil.raw" "${1}/data/Audio" "ee6638e481aa7f99ce47a0e145dd3d62"


# Finish work
echo "All files copied successfully :-)"




