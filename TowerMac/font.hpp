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
#include <vector>
#include <iterator>

#include <memory>

template <int SCALE> class fixed
{	int32_t value_;
	static fixed make_fixed( int32_t value ) { fixed v{0}; v.value_ = value; return v; }
public:
	fixed() : value_{0} {}
	fixed( int v ) : value_{ v<<SCALE } {}
	fixed( size_t v ) : value_{ ((int32_t)v)<<SCALE } {}
	int int_value() const { return (value_+(1<<(SCALE-1)))>>SCALE; }
	size_t size_t_value() const { return (size_t)int_value(); }

	fixed operator+( const fixed<SCALE> &other ) const { return make_fixed( value_+other.value_ ); }
	fixed operator-( const fixed<SCALE> &other ) const { return make_fixed( value_-other.value_ ); }
	fixed operator+( size_t value ) const { return make_fixed( value_+(((int32_t)value)<<SCALE) ); }
	fixed operator+( int value ) const { return make_fixed( value_+(value<<SCALE) ); }
	fixed operator/( int div ) const { return make_fixed( value_/div ); }
	fixed operator*( size_t value ) const { return make_fixed( value_*value ); }

	bool operator<( const fixed<SCALE> &other ) const { return value_<other.value_; }
	bool operator>( const fixed<SCALE> &other ) const { return value_>other.value_; }
};

typedef fixed<16> fract;

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

	size_t measure_text( const char *s ) const;

	///	Render text, starting at point p
	//	Spacing is the number of pixels we should add to all spaces throught the display
	//	IntraSpacing is the number of pixels we should add to all letters throught the display

	void draw_letter(point &screen_pointer, char c) const;

	static bool is_separator( char c ) { return c==' ' || c=='\n'; }
	static bool is_eol( char c ) { return c=='\n'; }
    
    const image *at( size_t i ) const
    {
        if (images_[i])
            return images_[i].get();
        assert( images_[' '] );
        return images_[' '].get();
    }
    size_t widthof(char c) const;
};

	///	Returns a measure structure that descrives the space used by the text

#endif
