@echo off

IF NOT EXIST build MKDIR build

SET ENABLE_FULLSCREEN=1
SET DEBUG=0
SET PROFILER=0

SET DEFINES=-DGLEW_STATIC -DGLM_ENABLE_EXPERIMENTAL

SET WARNINGS_ON_UNIMPLEMENTED_ENUM_SWITCH_CASES=-w44062
SET COMPILER_FLAGS=/std:c++17 -nologo -WX -W4 %WARNINGS_ON_UNIMPLEMENTED_ENUM_SWITCH_CASES% -EHsc

IF "%ENABLE_FULLSCREEN%"=="1" (
    SET DEFINES=%DEFINES% -DWINDOW_FULLSCREEN
)

if "%DEBUG%"=="1" (
    SET DEFINES=%DEFINES% -DDEBUG
    SET COMPILER_FLAGS=%COMPILER_FLAGS% -Zi
) else (
    SET COMPILER_FLAGS=%COMPILER_FLAGS% -O2
)

if "%PROFILER%"=="1" (
    SET DEFINES=%DEFINES% -DPROFILER -DTRACY_ENABLE
)

SET PROJECT_INCLUDES=-I"..\code\game" -I"..\code\external" -I"..\code\engine" -I"..\code\engine\renderer" -I"..\code\engine\audio" -I"..\code\engine\shapes" -I"..\code\editor" -I"..\code\tracy-0.13.1"

SET EXTERNAL_INCLUDES=-I"C:\vclibraries\SDL3-3.2.0\include" -I"C:\vclibraries\glew-2.1.0\include" -I"C:\vclibraries\glm-1.0.2\include" -I"C:\vclibraries\stb" -I"C:\vclibraries\assimp\include" -I"C:\vclibraries\OpenALSoft\include" -I"C:\vclibraries\SDL3_ttf-3.2.2\include" -I"C:\vclibraries\Imgui\imgui-1.92.5" -I"C:\vclibraries\Imgui\imgui-1.92.5\backends" -I"C:\vclibraries\Expat 2.7.3\include" -I"C:\vclibraries\nlohmann-json" -I"C:\vclibraries\ThreadPool" -I"C:\vclibraries\FastNoise"

SET EXTERNAL_LIBRARIES=-LIBPATH:"C:\vclibraries\SDL3-3.2.0\lib\x64" -LIBPATH:"C:\vclibraries\glew-2.1.0\lib\Release\x64" -LIBPATH:"C:\vclibraries\assimp\lib\x64" -LIBPATH:"C:\vclibraries\OpenALSoft\lib\x64" -LIBPATH:"C:\vclibraries\SDL3_ttf-3.2.2\lib\x64" -LIBPATH:"C:\vclibraries\Imgui\imgui-1.92.5\build" -LIBPATH:"C:\vclibraries\Expat 2.7.3\Bin"

SET DISABLED_WARNINGS=-wd4100 -wd4101 -wd4189 -wd4201 -wd4996 -wd4456 -wd4267 -wd4505

pushd build
cl %DEFINES% %COMPILER_FLAGS% %DISABLED_WARNINGS% "..\code\game\main.cpp" %PROJECT_INCLUDES% %EXTERNAL_INCLUDES% -link -opt:ref %EXTERNAL_LIBRARIES% -SUBSYSTEM:WINDOWS glew32s.lib SDL3.lib assimp-vc143-mt.lib imgui.lib opengl32.lib OpenAL32.lib SDL3_ttf.lib libexpat.lib Comdlg32.lib -PDB:"C:\vclibraries\SDL3-3.2.0\lib\x64\SDL3.pdb" -IGNORE:4099
popd