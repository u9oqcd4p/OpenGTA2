@echo off

"..\tools\Release\txt2chunked.exe" "anim_gta2spr.txt" "zanims.tex"

"..\tools\Release\sty2tex.exe" "..\gta2\ste.sty" false 0 GTA2 GTA2FONT false true
del gta2spr.tex
ren opengta.tex gta2spr.tex
del sprites_ste.bmp
ren sprites.bmp sprites_ste.bmp

"..\tools\Release\sty2tex.exe" "..\gta2\fstyle.sty" false 0 FSTYLE FSTYLE_FONT false true
del gta2fnt.tex
ren opengta.tex gta2fnt.tex
del sprites_fstyle.bmp
ren sprites.bmp sprites_fstyle.bmp