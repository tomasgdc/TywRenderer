# TywRenderer

Vulkan Renderer

#How to Build
Currently works only under windows.
Run .bat file which will build vs2015 solution

#Dll and Lib dependencies
> - Assimp
> - LibPng
> - Freetype

#Credits
>- Sascha Willems - https://github.com/SaschaWillems/Vulkan


#Projects
## [Triangle](Projects/Triangle)
<img src="ScreenShots/Triangle.png" height="96px" align="right">
Shader - [Triangle Shader](Assets/Shaders/Triangle)


## [Texture](Projects/Texture)
<img src="ScreenShots/Texture.png" height="96px" align="right">
Shader - [Texture Shader](Assets/Shaders/Texture)

## [Freetype font rendering](Projects/FontRendering)
<img src="ScreenShots/FontRendering.png" height="96px" align="right">
The fonts texture were generated using Freetype2 library.
For each glyph a texture is generated and put it into descriptors list. Of course it very bad thing to do.
The best thing is to have one texture for all glyps and point specific char UV coordinates.
First one is using signed distance field, the second one(down) does not use signed distance field.
There are still some strange edge bleeding which I do not know really why.

## [Normal Mapping](Projects/NormalMapping)
<img src="ScreenShots/NormallMapping.png" height="96px" align="right">
Shader - [Normal Mapping Shader](Assets/Shaders/NormalMapping)


## [Static Model](Projects/StaticModel)
<img src="ScreenShots/StaticModel.png" height="96px" align="right">
Shader - [Static Model Shader](Assets/Shaders/StaticModel)
There are issues with normal mapping. Shader is not complete.
Using TBN Matrix convert view vector and light vector to tangent space. Doing this results in black model.
Using my own wavefront parser.


## [Skeletal Animation](Projects/SkeletalAnimation)
Shader - [Static Model Shader](Assets/Shaders/StaticModel)
Project is not finnished
Using my own MD5 parser.
