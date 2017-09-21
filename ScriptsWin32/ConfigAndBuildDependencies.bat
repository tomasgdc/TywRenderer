::BUILD ASSIMP

cd ../External/assimp
mkdir build
cd build

cmake .. -G "Visual Studio 15 2017 Win64" -DASSIMP_BUILD_ASSIMP_TOOLS=off -DASSIMP_BUILD_SAMPLES=off -DASSIMP_BUILD_TESTS=off -DBUILD_SHARED_LIBS=off
call cmake --build . --config Release
call cmake --build . --config Debug
cd ..\..\..\ScriptsWin32

::BUILD SDL
cd ../External/sdl
mkdir build
cd build

cmake .. -G "Visual Studio 15 2017 Win64"
call cmake --build . --config Release
call cmake --build . --config Debug
cd ..\..\..\ScriptsWin32

timeout 2
