@echo off

IF NOT EXIST build MKDIR build

pushd build

cl -DGLEW_STATIC -nologo -WX -W4 -wd4100 -wd4189 -wd4996 -EHa- "..\code\main.cpp" -I"C:\vclibraries\SDL3-3.2.0\include" -I"C:\vclibraries\glew-2.1.0\include" -link -LIBPATH:"C:\vclibraries\SDL3-3.2.0\lib\x64" -LIBPATH:"C:\vclibraries\glew-2.1.0\lib\Release\x64" -SUBSYSTEM:WINDOWS glew32s.lib SDL3.lib opengl32.lib

popd