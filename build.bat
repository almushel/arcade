@echo off

set compiler_flags=/Zi /nologo 
set libs=gdi32.lib raylib.lib shell32.lib user32.lib winmm.lib
set linker_flags=/INCREMENTAL:NO /NODEFAULTLIB:libcmt /OUT:"snake.exe"

if not exist "bin" mkdir bin
pushd bin
cl %compiler_flags% ../src/game.c -I"../../../../include" /link -LIBPATH:"../../../../lib/win64" %linker_flags% %libs% 
popd
