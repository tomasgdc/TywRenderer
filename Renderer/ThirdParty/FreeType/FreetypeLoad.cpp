/* ============================================================================
* Tywyl Engine
* Platform:      Windows
* WWW:
* ----------------------------------------------------------------------------
* Copyright 2015 Tomas Mikalauskas. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY TOMAS MIKALAUSKAS ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL TOMAS MIKALAUSKAS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Tomas Mikalauskas.

DISCLAIMER
The engine design is based on Doom3 BFG engine
https://github.com/id-Software/DOOM-3-BFG.
A lot of tweaks maded to suit my needs.
Tywyll game engine design and functionality will be changed with time.
============================================================================
*/
#include <RendererPch\stdafx.h>
#include "FreetypeLoad.h"

#define HRES  64
#define HRESf 64.f
#define DPI   96



inline int GlyphData::NextP2(int a)
{
	int rval = 2;
	// rval<<=1 Is A Prettier Way Of Writing rval*=2;
	while (rval<a) rval <<= 1;
	return rval;
}


GlyphData::GlyphData()
{
	log = "No log information \r\n";
}


//Initialize library and font
bool GlyphData::LoadGlyph(const char* strTypeface, int point_size, int dpi)
{
	
	if (!IntiliazeFont(strTypeface, point_size, dpi)) 
	{
		return false;
	}

	glyph = face->glyph;
	return true;
}

//Load specified characters and store them in 'glyphs' data
bool GlyphData::InitiliazeChars(const char* source)
{
	FT_UInt  glyph_index;
	
	
	if (source == nullptr){
		log = "ERROR: Initialized pointer is NULL \r\n";
		return false;
	}

	

	size_t size = strlen(source);
	//Could be used std::string iterator
	for (int i = 0; i < size; i++)
	{
		Data gd = { nullptr,{ 0,0 }, 0, 0, 0, 0, 0,0 };
		char c = source[i];
		glyph_index = FT_Get_Char_Index(face, c);/* retrieve glyph index from character code */

		if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))/* load glyph image into the slot (erase previous one) */
		{
			//fprintf(stderr, "Could not load character %c\n", c);
			log = "ERROR: Could not load character \r\n"; //c
			return false;
		}

		if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))/* convert to an anti-aliased bitmap */
		{
			//fprintf(stderr, "Could not convert to an anti-aliased bitmap - %c\n", c);
			log = "ERROR: Could not convert to an anti-aliased bitmap - \r\n"; //C
			return false;
		}

		//int width = NextP2(glyph->bitmap.width);
		//int height = NextP2(glyph->bitmap.rows);
		// init the gd.bitmap_buffer array to be the size needed
		try
		{
			gd.bitmap_buffer = TYW_NEW unsigned char[glyph->bitmap.width*glyph->bitmap.rows];
		}
		catch (...) 
		{
			SAFE_DELETE_ARRAY(gd.bitmap_buffer);
			log = "ERROR: Could not allocate memory for bitmap_buffer \r\n";
			return false;
		}
		// copy the texture data from 'glyph' to 'gd'
		memcpy(gd.bitmap_buffer, glyph->bitmap.buffer, glyph->bitmap.rows * glyph->bitmap.width);

		/*
		for (int j = 0; j < width; ++j)
		{
			for (int i = 0; i < height; ++i)
			{
				gd.bitmap_buffer[j*width + i] = (j >= glyph->bitmap.width || i >= glyph->bitmap.rows ? 0 : glyph->bitmap.buffer[j*glyph->bitmap.width + i]);
			}
		}
		*/
		

		/*
		//Load texture into memory
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Match texture alignement

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
			);
		*/
		//=======================================================================================
		gd.bitmap_width = glyph->bitmap.width; // copy texture width
		gd.bitmap_rows = glyph->bitmap.rows; // copy texture height
		gd.bitmap_left = glyph->bitmap_left; // copy glyph width (pixels)
		gd.bitmap_top = glyph->bitmap_top; // copy glyph height (pixels)
		gd.advance = glyph->advance; // copy the advance vector (note this isn't c++ vector, this is real vector)
		gd.c = c; // set the character
		glyphs.insert(std::unordered_map<char, Data>::value_type(c, gd));
		//SAFE_DELETE_ARRAY(gd.bitmap_buffer);
	}
	return true;
}

//Loads library and needed fonts
bool GlyphData::IntiliazeFont(const char * strTypeface, int point_size, int dpi)
{
	if (FT_Init_FreeType(&library)) {
		log = "ERROR: could not load Freetype library\r\n";
		return false;
	}


	if (FT_New_Face(library, strTypeface, 0, &face)) {
		log = "ERROR: could not load specified .fft file\r\n";
		return false;
	}


	if (FT_Set_Char_Size(face, point_size * HRES, point_size * HRES, dpi, dpi)) {
		log = "ERROR: FT_Set_Char_Size failed \r\n";
		return false;
	}


	if (FT_Set_Pixel_Sizes(face, 0, point_size)) {
		log = "ERROR: FT_Set_Pixel_Sizes failed \r\n";
		return false;
	}
	return true;
}

Data GlyphData::getChar(char c) {
	Data gd = { nullptr,{ 0,0 }, 0, 0, 0, 0, 0 };
	std::unordered_map<char, Data>::const_iterator got = glyphs.find(c);
	if (got == glyphs.end()) {
		log = "Could not find specified character \r\n";
		return gd;
	}
	return got->second;
}

void GlyphData::ReleaseBuffer() {
	for (auto it = glyphs.begin(); it != glyphs.end(); ++it) {
		SAFE_DELETE_ARRAY(it->second.bitmap_buffer);
	}
}

bool GlyphData::Release() {
	//SAFE_DELETE(log); //somewhy -> breaks

	 //free FreeType resources
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	//delete glyph data memory
	ReleaseBuffer();
	glyphs.clear();
	return true;
}

GlyphData::~GlyphData(){
	//In case i forget to delete it
	//Release();
	//if memory for ft_donace_face
	//was released and you traying to do it again
	//then you going to get seg fault.. I think
	//but in vs you just get fucking break
}