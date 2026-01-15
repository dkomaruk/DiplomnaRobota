@echo off

IF NOT EXIST build MKDIR build

SET ENABLE_TRANSPARENCY=0
SET LOAD_ASSETS=0

SET DEFINES=-DGLEW_STATIC

IF "%ENABLE_TRANSPARENCY%"=="1" (
    SET DEFINES=%DEFINES% -DWINDOW_TRANSPARENT
)

IF "%LOAD_ASSETS%"=="1" (
    SET DEFINES=%DEFINES% -DLOAD_ASSETS
)


SET PROJECT_INCLUDES=-I"..\code\game" -I"..\code\external" -I"..\code\engine" -I"..\code\engine\renderer" -I"..\code\engine\audio"

SET EXTERNAL_INCLUDES=-I"C:\vclibraries\SDL3-3.2.0\include" -I"C:\vclibraries\glew-2.1.0\include" -I"C:\vclibraries\glm-1.0.2\include" -I"C:\vclibraries\stb" -I"C:\vclibraries\assimp\include" -I"C:\vclibraries\OpenALSoft\include" -I"C:\vclibraries\SDL3_ttf-3.2.2\include" -I"C:\vclibraries\Imgui\imgui-1.92.5" -I"C:\vclibraries\Imgui\imgui-1.92.5\backends"

SET EXTERNAL_LIBRARIES=-LIBPATH:"C:\vclibraries\SDL3-3.2.0\lib\x64" -LIBPATH:"C:\vclibraries\glew-2.1.0\lib\Release\x64" -LIBPATH:"C:\vclibraries\assimp\lib\x64" -LIBPATH:"C:\vclibraries\OpenALSoft\lib\x64" -LIBPATH:"C:\vclibraries\SDL3_ttf-3.2.2\lib\x64" -LIBPATH:"C:\vclibraries\Imgui\imgui-1.92.5\build"

SET DISABLED_WARNINGS=-wd4100 -wd4101 -wd4189 -wd4996 -wd4456

pushd build
cl %DEFINES% -nologo -WX -W4 %DISABLED_WARNINGS% -EHsc -Zi "..\code\game\main.cpp" %PROJECT_INCLUDES% %EXTERNAL_INCLUDES% -link %EXTERNAL_LIBRARIES% -SUBSYSTEM:WINDOWS glew32s.lib SDL3.lib assimp-vc143-mt.lib imgui.lib opengl32.lib OpenAL32.lib SDL3_ttf.lib -PDB:"C:\vclibraries\SDL3-3.2.0\lib\x64\SDL3.pdb" -IGNORE:4099
popd