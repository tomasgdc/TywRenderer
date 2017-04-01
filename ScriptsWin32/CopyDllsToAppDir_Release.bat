cd ..
mkdir App

::GET Vulkan
copy %VK_SDK_PATH%\\Source\\lib\\vulkan-1.dll App\\vulkan-1.dll

::GET SDL
copy external\\sdl\\build\\Release\\SDL2.dll App\\SDL2.dll

::GET ASSIMP
copy external\\assimp\build\code\Release\\assimp-vc140-mt.dll App\\assimp-vc140-mt.dll

::GET FREETYPE
copy "External\\freetype\\builds\\windows\\visualc\\x64\\Release Multithreaded\\freetype.dll" App\\freetype.dll

cd ScriptsWin32
timeout 2
