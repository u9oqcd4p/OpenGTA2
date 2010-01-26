#!/bin/bash
#
# Uebersetzt OpenGTA2 unter Ubuntu
CC="g++"
TMP=".tmp-opengta2"


# Altes temporaeres Verzeichnis loeschen
if [ -d "${TMP}" ]; then
	rm -r "${TMP}"
fi
mkdir "${TMP}"


# Zum Build benoetigte Dateien kopieren
cp -r "opengta2" "${TMP}/opengta2"


# Folgende Dateien muessen uebersetzt und gelinkt werden
FILES=(	"main"
	"malloc"
)


# Alle Sourcedateien loeschen
$CC --help

