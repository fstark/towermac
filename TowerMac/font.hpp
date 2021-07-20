//
//  font.hpp
//  TowerMac
//
//  Created by Frederic Stark on 18/07/2021.
//

#ifndef FONT_INCLUDED__
#define FONT_INCLUDED__

#include <string>

#include "core.hpp"
#include "image.hpp"

#include <memory>

class font
{
	std::string name_;
	font( const std::string &filename );

	std::shared_ptr<image> images_[256];
	std::shared_ptr<image> inverted_[256];

public:
	///	initialize the font subsystem
	static void init();
	
	///	The standard fonts
	static std::unique_ptr<font> normal;
	static std::unique_ptr<font> bold;

	///	Render text, starting at point p
	void render_text( const point &p, const char *s, bool inverted=false ) const;
	size_t measure_text( const char *s ) const;
};

#endif
