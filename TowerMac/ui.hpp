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
#include "font.hpp"
#include "core.hpp"

///	Something that can draw stuff an expose primitives (using SDL or others)
class graphics
{
public:
	typedef enum
	{
		kBlack =0,
		kWhite = 1
	}	color;

protected:
	struct state
	{
		point origin;
		point location;	/// [includes origin offset]
		const font *font;
		color stroke;
		color fill;
	};

	state state_;
	
	std::vector<state> states_;
public:
	void reset()
	{
		state_.origin = {0,0};
		state_.location = {0,0};
		state_.font = font::normal.get();
		state_.stroke = kBlack;
		state_.fill = kWhite;
	}
	void set_stroke( color c ) { state_.stroke = c; }
	void set_fill( color c ) { state_.fill = c; }
	void frame_rect( const rect & r );
	void fill_rect( const rect & r );
	void set_origin( const point &p ) { state_.origin += p; }
	void set_font( const font *font ) { state_.font = font; }
	void draw_text( const std::string &s, bool inverted = false );
	void move_to( point p );

	void push() { states_.push_back( state_ ); }
	void pop() { state_ = states_.back(); states_.pop_back(); }
};

///	A simple UI frmaework for towermac
///	Subset of NeXTstep's AppKit: nested views, buttons, text. Views can have borders.

///	A View is a rectangular part of the screen. It contains subviews.
///	The border around a view is _external_ to the view.
class view
{
protected:
	point origin_ = {0,0};	///	Top-left in parent view
	rect bounds_;	///	Our own coordinates

	view *superview_ = nullptr;
	std::vector<std::unique_ptr<view>> subviews_;	///	Views are owned by the graphic hierarchy

	size_t border_width = 1;
	graphics::color border_color = graphics::kBlack;
	
	bool has_fill = false;
	graphics::color background_color = graphics::kWhite;

public:
	view( const rect &b ) : bounds_{b} {}
	view( const size &s ) : bounds_{ {0,0},s } {}
	virtual ~view() {}
	
	rect frame()
	{
		rect f = bounds_;
		f.o = origin_;
		return f;
	}

	void set_opaque( bool opaque=true ) { has_fill = opaque; }
	void set_background_color( graphics::color color ) { background_color = color; }

	void add( view *v, point origin )
	{
		v->origin_ = origin;
		v->superview_ = this;
		subviews_.emplace_back( v );
	}

	virtual void draw_self( graphics &g )
	{
		rect r = bounds_;
		if (has_fill)
		{
			g.set_fill( background_color );
			g.fill_rect( r );
		}
		if (border_width)
		{
			r.s.w--;
			r.s.h--;
			g.set_stroke( graphics::kBlack );
			g.frame_rect( r.inset( -border_width, -border_width ) );
		}
	}

	void draw( graphics &g )
	{
		g.push();
		g.set_origin( origin_ );
		draw_self( g );
		for (auto &v:subviews_)
			v->draw( g );
		g.pop();
	}
};

class custom_view : public view
{
	std::function<void( custom_view &cv, graphics &g )> callback_;
public:
	custom_view( size s, std::function<void( custom_view &, graphics & )> callback ) : view{ s }, callback_{callback} {}

	virtual void draw_self( graphics &g )
	{
		view::draw_self( g );
		callback_( *this, g );
	}
};

#include "font.hpp"

///	A button displays text or image and accepts mouse down
class button : public view
{
	struct cell
	{
		graphics::color background_color = graphics::kWhite;
		std::string text = "";
//		const image *image_ = nullptr;
		const font *font = nullptr;
		bool inverted = false;		//	White on black
		
		size best_size() const { return { font->measure_text( text.c_str() ), 7 }; }
	}	cells_[2];

	int state_ = kStateNormal;
	
public:
	button( const std::string &t, const font *f=nullptr ) : view{{{0,0},{0,0}}}
	{
		set_opaque( false );	///	Superclass does not fill
		set_text( t );
		set_font( f );
		set_inverted( true, kStateSelected );
		size_to_fit();
		set_background_color( graphics::kBlack, kStateSelected );
	}

	static const int kStateNormal = 0;
	static const int kStateSelected = 1;
	static const int kStateBoth = 2;

	bool is_selected() const { return state_==kStateSelected; }
	void set_selected( bool select ) { state_ = select?kStateSelected:kStateNormal; }
	
	void set_text( const std::string &text, int state=kStateBoth )
	{
		if (state==kStateNormal || state==kStateBoth)
		{
			cells_[kStateNormal].text = text;
		}
		if (state==kStateSelected || state==kStateBoth)
		{
			cells_[kStateSelected].text = text;
		}
	}
	
	void set_font( const font *font, int state=kStateBoth )
	{
		if (!font)
			font = font::normal.get();
		if (state==kStateNormal || state==kStateBoth)
		{
			cells_[kStateNormal].font = font;
		}
		if (state==kStateSelected || state==kStateBoth)
		{
			cells_[kStateSelected].font = font;
		}
	}
	void set_inverted( bool inverted, int state=kStateBoth )
	{
		if (state==kStateNormal || state==kStateBoth)
		{
			cells_[kStateNormal].inverted = inverted;
		}
		if (state==kStateSelected || state==kStateBoth)
		{
			cells_[kStateSelected].inverted = inverted;
		}
	}

	void set_background_color( graphics::color background_color, int state=kStateBoth )
	{
		if (state==kStateNormal || state==kStateBoth)
		{
			cells_[kStateNormal].background_color = background_color;
		}
		if (state==kStateSelected || state==kStateBoth)
		{
			cells_[kStateSelected].background_color = background_color;
		}
	}
	
	void size_to_fit()
	{
		auto r0 = cells_[kStateNormal].best_size();
		auto r1 = cells_[kStateSelected].best_size();
		bounds_.s = { std::max(r0.w, r1.w)+4, std::max(r0.h,r1.h)+2 };
	}

	virtual void draw_self( graphics &g )
	{
		view::draw_self( g );

		g.set_fill( cells_[state_].background_color );
		g.fill_rect( bounds_ );

		g.set_font( cells_[state_].font );
		g.move_to( {2,1} );
		g.draw_text( cells_[state_].text, cells_[state_].inverted );
	}
};

class window
{
	view root_;
	graphics graphics_;
	
	window( const rect &r ) : root_{ r }, graphics_{} {}
public:
	///	Creates a platorm-dependent window (512x342)
	static std::unique_ptr<window> make_window()
	{
		rect r{{0,0},{512,342}};
		return std::unique_ptr<window>{ new window{ r } };
	}
	
	view &root() { return root_; }
	
	void draw();
};

#endif
