#!/bin/bash
#
# Uebersetzt OpenGTA2 unter Ubuntu
CC="g++"
#INC="-Iinclude"
INC=""
OPT=""
TMP=".tmp-opengta2"



# Alle Sourcedateien in einem Ordner compilieren
#
# @param $1 Ordner in welchem sich die Sourcedateien befinden
# @param $2 Array von in diesem Ordner zu uebersetzenden Quelldateien
function compile {
	local FILES=(`echo "$2"`)

	if [ -d "${TMP}" ]; then
		cd "${TMP}"
	else
		echo "Warning: could not change into directory ${TMP} (pwd is `pwd`)"
	fi

	if [ -d "${1}" ]; then
		echo "Compiling sources in directory ${1}"
	else
		echo "Directory ${1} doesn't exist"
	fi

	CNT=0
	TOTAL=${#FILES[@]}
	for FILE in $2
	do
		# Information an Benutzer ausgeben
		let CNT=CNT+1
		echo "Compiling ${1}/${FILE} (${CNT}/$TOTAL)"

		# Handelt es sich um eine C++ Quellcodedatei?
		if [ -e "${1}/${FILE}.cpp" ]; then
			$CC $INC $OPT -c "${1}/${FILE}.cpp" -o "${1}/${FILE}.o"
		else
			echo "Don't know how to compile ${1}/${FILE} (file ${1}/$FILE.{cpp} doesn't exist)"
		fi
	done

	cd "../"
}





# Altes temporaeres Verzeichnis loeschen
if [ -d "${TMP}" ]; then
	rm -r "${TMP}"
fi
mkdir "${TMP}"


# Zum Build benoetigte Dateien kopieren
cp -r "include" "${TMP}/include"
cp -r "opengta2" "${TMP}/opengta2"


# Folgende Dateien muessen uebersetzt und gelinkt werden
OPENGTA2=(			\
	"anim"			\
	"camera"		\
	"chunkload"		\
	"clientlist"		\
	"cmain"			\
	"collision"		\
	"console"		\
	"convar"		\
	"debugtest"		\
	"draw"			\
	"font"			\
	"game"			\
	"graphics"		\
	"input"			\
	"log"			\
	"main"			\
	"malloc"		\
	"map"			\
	"map_geom_cl"		\
	"map_geom"		\
	"map_render"		\
	"matrix"		\
	"network"		\
	"ped"			\
	"ped_mgr"		\
	"ped_render"		\
	"perfhud"		\
	"screen"		\
	"smain"			\
	"texture_atlas"		\
	"thread"		\
	"timer"			\
	"utf8"			\
	"vbuffer"		\
	"vector"		\
)


# Kompletten opengta2 Ordner uebersetzen (@see http://www.faqs.org/docs/abs/HTML/assortedtips.html)
arg=`echo ${OPENGTA2[@]}`
compile "opengta2" "$arg"

# Testweisen nur einzelne Dateien uebersetzen
#TEST_FILES=( "network" )
#arg=`echo ${TEST_FILES[@]}`
#compile "opengta2" "$arg"









