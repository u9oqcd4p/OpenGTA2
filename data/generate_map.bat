@echo off

"..\tools\Release\gmp2map.exe" "..\gta2\wil" WIL0 0 0 WIL
"..\tools\Release\gmp2map.exe" "..\gta2\bil" BIL0 5 0 BIL
"..\tools\Release\gmp2map.exe" "..\gta2\ste" STE0 5 5 STE

del wil.rmp
del bil.rmp
del ste.rmp
ren WIL0.rmp wil.rmp
ren BIL0.rmp bil.rmp
ren STE0.rmp ste.rmp