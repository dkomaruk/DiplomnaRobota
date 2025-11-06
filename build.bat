@echo off

IF NOT EXIST build MKDIR build


SET ENABLE_TRANSPARENCY=0

SET DEFINES=-DGLEW_STATIC

IF "%ENABLE_TRANSPARENCY%"=="1" (
    SET DEFINES=%DEFINES% -DWINDOW_TRANSPARENT
)

pushd build
cl %DEFINES% -nologo -WX -W4 -wd4100 -wd4189 -wd4996 -EHsc -Zi "..\code\main.cpp" -I"C:\vclibraries\SDL3-3.2.0\include" -I"C:\vclibraries\glew-2.1.0\include" -I"C:\vclibraries\glm-1.0.2\include" -I"C:\vclibraries\stb_image" -I"C:\vclibraries\assimp\include" -link -LIBPATH:"C:\vclibraries\SDL3-3.2.0\lib\x64" -LIBPATH:"C:\vclibraries\glew-2.1.0\lib\Release\x64" -LIBPATH:"C:\vclibraries\assimp\lib\x64" -SUBSYSTEM:WINDOWS glew32s.lib SDL3.lib assimp-vc143-mt.lib opengl32.lib -PDB:"C:\vclibraries\SDL3-3.2.0\lib\x64\SDL3.pdb" -IGNORE:4099
popd