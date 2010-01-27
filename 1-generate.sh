#!/bin/bash
#
# Converts the original GTA2 files into files which can be read by OpenGTA2
#
TMP=".tmp-opengta2"

cp -r "gta2" "${TMP}/gta2"

cd "${TMP}/tools"
./all2map.sh
cd ../

