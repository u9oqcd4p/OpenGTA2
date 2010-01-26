#!/bin/bash
#
# Uebersetzt OpenGTA2 unter Ubuntu
CC="g++"
#INC="-Iinclude"
INC=""
OPT=""
TMP=".tmp-opengta2"


# Altes temporaeres Verzeichnis loeschen
if [ -d "${TMP}" ]; then
	rm -r "${TMP}"
fi
mkdir "${TMP}"


# Zum Build benoetigte Dateien kopieren
cp -r "include" "${TMP}/include"
cp -r "opengta2" "${TMP}/opengta2"


# Folgende Dateien muessen uebersetzt und gelinkt werden
FILES=(	"opengta2/main"
)


# Alle Sourcedateien compilieren
cd "${TMP}"
for FILE in $FILES 
do
	# Handelt es sich um eine C++ Quellcodedatei?
	if [ -e "${FILE}.cpp" ]; then
		$CC $INC $OPT -c "${FILE}.cpp" -o "${FILE}.o"
	else
		echo "Don't know how to compile ${FILE} (file $FILE.{cpp} doesn't exist)"
	fi
done








