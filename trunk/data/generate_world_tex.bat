@echo off

"..\tools\Release\sty2tex.exe" "..\gta2\ste.sty" false STE 0 0 true false
del ste.tex
ren opengta.tex ste.tex
"..\tools\Release\txt2chunked.exe" "anim_ste.txt" "tmp"
"..\tools\Release\chunkmerge.exe" "tmp2" "ste.tex" "tmp"
del ste.tex
ren tmp2 ste.tex
del tmp

"..\tools\Release\sty2tex.exe" "..\gta2\bil.sty" false BIL 0 0 true false
del bil.tex
ren opengta.tex bil.tex
"..\tools\Release\txt2chunked.exe" "anim_bil.txt" "tmp"
"..\tools\Release\chunkmerge.exe" "tmp2" "bil.tex" "tmp"
del bil.tex
ren tmp2 bil.tex
del tmp

"..\tools\Release\sty2tex.exe" "..\gta2\wil.sty" false WIL 0 0 true false
del wil.tex
ren opengta.tex wil.tex
"..\tools\Release\txt2chunked.exe" "anim_wil.txt" "tmp"
"..\tools\Release\chunkmerge.exe" "tmp2" "wil.tex" "tmp"
del wil.tex
ren tmp2 wil.tex
del tmp