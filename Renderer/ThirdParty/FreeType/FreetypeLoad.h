/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once

#include <External\freetype\include\ft2build.h>
#include FT_FREETYPE_H //Include main FREETYPE2 API



	typedef struct TYWRENDERER_API Data_
	{
		unsigned char *bitmap_buffer;   // texture data
		FT_Vector advance;  // this variable contains the information of how much we need to move to the right from the last character
		int bitmap_left;    // width of the glyph in pixels
		int bitmap_top;     // height of the glyph in pixels
		unsigned int bitmap_width;   // texture width
		unsigned int bitmap_rows;    // texture height
		unsigned int size;   // font size
		char c;     // the character of this glyph
	}Data;

class TYWRENDERER_API GlyphData 
{
public:
	GlyphData();
	bool LoadGlyph(const char* strTypeface, int point_size, int dpi);
	std::string getLog() const { return log; };
	Data getChar(char c);
	bool InitiliazeChars(const char* source);
	bool Release();
	void ReleaseBuffer();
	~GlyphData();

private:
	inline int NextP2(int a); //This Function Gets The First Power Of 2
	bool IntiliazeFont(const char* strTypeface, int point_size, int dpi);
	FT_Library  library;	/* handle to library     */
	FT_Face     face;      /* handle to face object */
	FT_GlyphSlot glyph;         /*Access glyph information*/
	std::unordered_map<char, Data> glyphs; //Create hash map
	std::string log; //Get any log information
	//char* log -> bitch, breaking bitch
};