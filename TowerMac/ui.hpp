//
//  ui.hpp
//  TowerMac
//
//  Created by Frederic Stark on 18/07/2021.
//

#ifndef UI_INCLUDED__
#define UI_INCLUDED__

#include "core.hpp"
#include <vector>
#include <memory>

#include "image.hpp"

///	Something that can draw stuff an expose primitives (using SDL or others)
class graphics
{
public:
	void frame_rect( const rect & r );
};

///	A simple UI frmaework for towermac
///	Subset of NeXTstep's AppKit: nested views, buttons, text. Views can have borders.

///	A View is a rectangular part of the screen. It contains subviews.
class view
{
protected:
	point origin_ = {0,0};	///	Top-left in parent view
	rect bounds_;	///	Our own coordinates

	view *superview_ = nullptr;
	std::vector<std::unique_ptr<view>> subviews_;	///	Views are owned by the graphic hierarchy

public:
	view( const rect &b ) : bounds_{b} {}

	void add( view *v, point &origin )
	{
		v->origin_ = origin;
		v->superview_ = this;
		subviews_.emplace_back( v );
	}
	
	void draw( graphics &g )
	{
		g.frame_rect( bounds_ );
	}
};

#include "font.hpp"

///	A button disaplys text or image and accepts mouse down
class button : public view
{
	std::string text_="";
	const image *image_=nullptr;
	const font *font_=nullptr;
public:
	button( const point &center, const std::string &t ) : view{{{0,0},{0,0}}}, text_{t}, font_{font::normal.get()}
	{
		size_to_fit();
		origin_ = { center.x-bounds_.s.w/2, center.y-bounds_.s.h/2 };
	}

	button( const rect &b, const std::string &t, const image *img ) : view{b}, text_{t}, image_{img},font_{font::normal.get()}
	{
		
	}

	bool has_text() const { return text_!=""; }
	bool has_image() const { return image_!=nullptr; }
	
	void size_to_fit()
	{
		if (has_image())
			bounds_.s = image_->size();
		if (has_text() && font_)
			bounds_.s = { font_->measure_text( text_.c_str() ), 7 };
	}
};

#endif
