@echo off
"..\tools\Release\txt2chunked.exe" "water.txt" "water1.rmp" 4 4 5 4 6 4 7 4
"..\tools\Release\txt2chunked.exe" "water.txt" "water2.rmp" 4 0 4 1 4 2 4 3
"..\tools\Release\txt2chunked.exe" "water.txt" "water3.rmp" 4 5 4 6 4 7 4 8
"..\tools\Release\txt2chunked.exe" "water.txt" "water4.rmp" 4 9 5 9 6 9 7 9
"..\tools\Release\chunkmerge.exe" "water.rmp" "water1.rmp" "water2.rmp" "water3.rmp" "water4.rmp"
del water1.rmp
del water2.rmp
del water3.rmp
del water4.rmp