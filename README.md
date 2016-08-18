# TywRenderer

Vulkan Renderer

#How to Build
Currently works only under windows.
Run .bat file which will build vs2015 solution

#Dll and Lib dependencies
> - Assimp
> - LibPng


#Projects
## [Triangle](Projects/Triangle)
<img src="ScreenShots/Triangle.png" height="96px" align="right">
Shader - [Triangle Shader](Assets/Shaders/Triangle)


## [Texture](Projects/Texture)
<img src="ScreenShots/Texture.png" height="96px" align="right">
Shader - [Texture Shader](Assets/Shaders/Texture)


## [Normal Mapping](Projects/NormalMapping)
<img src="ScreenShots/NormallMapping.png" height="96px" align="right">
Shader - [Normal Mapping Shader](Assets/Shaders/NormalMapping)

## [Static Model](Projects/StaticModel)
<img src="ScreenShots/StaticModel.png" height="96px" align="right">
Shader - [Static Model Shader](Assets/Shaders/StaticModel)
There are issues with normal mapping. Shader is not complete.
Using TBN Matrix convert view vector and light vector to tangent space. Doing this results in black model.


## [Skeletal Animation](Projects/SkeletalAnimation)
Shader - [Static Model Shader](Assets/Shaders/StaticModel)
Project is not finnished
