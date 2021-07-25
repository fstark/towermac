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

	void render_text(
					 const point &origin,	//	Point to render the text
					 const char *text,		//	Text to render
					 size_t length,			//	Length of text to render
					 bool inverted,			//	Invert text?
					 fract space,			//	Width of each space (4 is good)
					 fract separator		//	Width of each character separation (1 is good)
					) const;

	struct measure
	{
		size_t skip;			///	Chars to skips
		size_t length;			///	Chars used (including spaces)
		fract width;			///	Pixel width of resulting string (including spaces and seps)
		size_t space_count;		///	Number of spaces
		size_t char_count;		///	Total number of non-space characters
		bool eol;				///	If true, there was an EOL (or EOS) in the line
	};

	static bool is_separator( char c ) { return c==' ' || c=='\n'; }
	static bool is_eol( char c ) { return c=='\n'; }

	///	Returns a measure structure that descrives the space used by the text
	measure measure_text_words( const char *text, size_t text_length, size_t width, fract space, fract separator ) const
	{
		int c;
		auto img = images_;
		fract x{ 0 };

		measure res;
		res.skip = 0;
		res.length = 0;
		res.width = 0;
		res.space_count = 0;
		res.char_count = 0;
		res.eol = false;

		bool first_word = true;

		int i = 0;
		
		do
		{
			// Count the leading spaces
			size_t leading_spaces = 0;
			while (i<text_length && is_separator(text[i]))
			{
				leading_spaces++;
				if (i && is_eol(text[i]))
				{
					res.eol = true;
					goto done;
				}
				i++;
			}
						
			fract remaining_width{ width };
			
			// Iterate in the current word
			bool first_letter = true;
			fract word_width{0};
			size_t word_len = 0;
			while (i<text_length && !is_separator(text[i]))
			{
				size_t letter_width = img[text[i]]?img[text[i]]->width():0;
				
				if (!first_letter)
					word_width = word_width + separator;
				first_letter = false;
				word_width = word_width + letter_width;
				if (res.width+space+separator+separator+word_width>remaining_width)	// #### NOPE, we need to decrement remaining_width
					goto done;		//	The word doesn't fit, we have finished the work
				word_len++;
				std::clog << i << ":" << text[i] << " " << letter_width << "px word=" << word_width.int_value() << "px " << " line=" << res.width.int_value() << "px charcount=" << res.char_count << " len=" << res.length << "\n";
				
				i++;
			}

			if (i==text_length)
				res.eol = true;
				
				//	Count the spaces
			if (first_word)
			{
				res.skip = leading_spaces;
				res.length += word_len;
				res.space_count = 0;
				res.char_count = word_len;
				first_word = false;
			}
			else
			{	//	Add a space
				res.width = res.width + space + separator + separator;
				res.length += leading_spaces + word_len;
				res.space_count += leading_spaces;
				res.char_count += word_len;
			}
			
			res.width = res.width + word_width;
			assert( !(res.width>remaining_width) );
		} while (i!=text_length);
	
	done:
		return res;
	}
};

#endif
