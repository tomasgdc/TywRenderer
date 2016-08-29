# TywRenderer

Vulkan Renderer. Work in progress.
- Warning. Take the code with grain of salt as it might violate Vulkan specs.
- Always happy to hear what could be fixed or improved.

#How to Build
Currently works only under windows.
Run .bat file which will build vs2015 solution

In order to get working. You need to have working Vulkan driver and Vulkan SDK that you can download from LunarG site
- LunarG Vulkan SDK - https://lunarg.com/
- More information about glsllang - https://github.com/KhronosGroup/glslang
- More information about validation layer - https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers
- More information about Vulkan - https://github.com/KhronosGroup/Vulkan-Docs

#Dll and Lib dependencies
- Assimp
- LibPng
- Freetype 

#Please check ThirdParty Licenses
- Freetype -  https://www.freetype.org/
- ImGui -     https://github.com/ocornut/imgui
- Assimp -    https://github.com/assimp/assimp
- LibPng -    http://www.libpng.org/pub/png/libpng.htm

#Credits
- Sascha Willems - https://github.com/SaschaWillems/Vulkan


#Projects
## [Triangle](Projects/Triangle)
<img src="ScreenShots/Triangle.png" height="96px" align="right">
Shader - [Triangle Shader](Assets/Shaders/Triangle)


## [Texture](Projects/Texture)
<img src="ScreenShots/Texture.png" height="96px" align="right">
Shader - [Texture Shader](Assets/Shaders/Texture)

## [Freetype font rendering](Projects/FontRendering)
<img src="ScreenShots/FontRendering.png" height="126px" align="right">
The fonts texture were generated using Freetype2 library. The texture format that is generated by freetype is VK_FORMAT_R8_UNORM
For each glyph a texture is generated and put it into descriptors list. Of course it very bad thing to do.
The best thing is to have one texture for all glyps and point to specific char UV coordinates. In order to have alpha enabled you need
to enable blending in your VkPipeline.
First one is using signed distance field, the second one(down) does not use signed distance field.
There are still some strange edge bleeding which I do not know really why.
> - [Signed distance field shader](Assets/Shaders/FontRendering/FontRendering.frag)
> - [Non Signed distance field shader](Assets/Shaders/FontRendering/NonSdf.frag)

## [Normal Mapping](Projects/NormalMapping)
<img src="ScreenShots/NormallMapping.png" height="126px" align="right">
Shader - [Normal Mapping Shader](Assets/Shaders/NormalMapping)


## [Static Model](Projects/StaticModel)
<img src="ScreenShots/StaticModel.png" height="126px" align="right">
Shader - [Static Model Shader](Assets/Shaders/StaticModel)
There are issues with normal mapping. Shader is not complete.
Using TBN Matrix convert view vector and light vector to tangent space. Doing this results in black model.
Using my own wavefront parser.


## [Skeletal Animation](Projects/SkeletalAnimation)
<img src="ScreenShots/SkeletalAnimation.gif" height="126px" align="right">
Shader - [Static Model Shader](Assets/Shaders/StaticModel)
GPU skinning of MD5 file. The biggest trick that MD5 file usually have less then 9 bones per vertex, so had to setup second vec4 for boneWeight and jointId. Also, glm does not handle well small angles so had to use different version of slerp which would handle small angles.

## License
Copyright (c) 2016 Tomas Mikalauskas

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


Some of the assets have different license. Please comply to these when redistributing or using them in your own projects :
- [boblamp](Assets//Geometry/boblamp) - http://www.katsbits.com/download/models/
- [cyberwarrior](Assets/Geometry/cyberwarrior) - https://sketchfab.com/models/86f58bf5151c410facacf0ed6a2ebd53
- [hellknight](Assets/Geometry/hellknight) Ripped and modified from Doom 3 - http://store.steampowered.com/app/9050/
- [nanosuit](Assets/Geometry/nanosuit) - Modiffied version from  http://tf3dm.com/3d-model/crysis-2-nanosuit-2-97837.html
- [photosculpt](Assets/Textures/photosculpt) - http://photosculpt.net/free-textures/
- [doom freetype](Assets/Textures/freetype/AmazDooMLeft.ttf) http://www.dafont.com/amazdoom.font
