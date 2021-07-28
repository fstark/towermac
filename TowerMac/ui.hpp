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

typedef enum
{
    kNone,
    kWord,
    kLinebreak
} eTokenType;

struct letter
{
    char c;
    uint8_t spacing = 0;
};
    
struct token
{
    eTokenType tokenType = kNone;
    size_t width = 0;
    std::vector<letter> letters;
};

struct styled_string
{
    const std::string text;
    const font *font;
    const bool inverted;
};


//  "I am a regular string with a bolded word."
//  "I am a regular string with a **bolded** word."

// WyK : templatize vector into iterator
static void tokenize(std::vector<token> &tokens, const styled_string &string)
{
    token token;
    for(auto c : string.text)
    {
        switch(c)
        {
            case '\n':
                if(token.tokenType!=kNone)
                    tokens.push_back(std::move(token));
                tokens.push_back({kLinebreak});
                token = {};
                break;
            case ' ':
                if(token.tokenType!=kNone)
                    tokens.push_back(std::move(token));
                token = {};
                break;
            default:
                token.tokenType = kWord;
                token.letters.push_back({c});
                token.width+= string.font->widthof(c);
        }
    }
    if(token.tokenType==kWord)
    {
        tokens.push_back(std::move(token));
    }
}

typedef std::vector<token> line;

static std::vector<line> arrange( const std::vector<token> &tokens, size_t max_line_width, size_t min_space_length, size_t min_separator_length, size_t &actual_max_width )
{
    // WyK : TOKEN IS COPIED. CHANGE THAT.
    std::vector<line> res;
    line line;
    size_t width = 0;
    bool first_word_of_line = true;
    
    for(token t : tokens)
    {
        retry:
        size_t min_word_width =  t.tokenType == kWord ? (first_word_of_line ? 0 : min_space_length) + t.width+ (t.letters.size()+(first_word_of_line ? 0 : 2)-1)*min_separator_length : 0;
        if(t.tokenType == kLinebreak)
        {
            actual_max_width = std::max(actual_max_width, width);
            width = 0;
            line.push_back(t);
            res.push_back(std::move(line));
            line.clear();
            continue;
        }
        if(t.tokenType == kWord && width + min_word_width > max_line_width)
        {
            if( first_word_of_line )
            {
                actual_max_width = max_line_width;
                line.push_back(t);
                first_word_of_line = true;
                width = 0;
                res.push_back(std::move(line));
                line.clear();
                continue;
            }
            first_word_of_line = true;
            actual_max_width = std::max(actual_max_width, width);
            width = 0;
            res.push_back(std::move(line));
            line.clear();
            goto retry;
        }
        width += min_word_width;
        line.push_back(t);
        first_word_of_line = false;
    }
    actual_max_width = std::max(actual_max_width, width);
    res.push_back(std::move(line));
    return res;
}

static size_t widthof(const line line)
{
    size_t res = 0;
    for(auto token : line)
    {
        res+=token.width;
    }
    return res;
}

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
		fract space_width = 2;
		fract char_interval = 1;
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
		state_.space_width = 2;
		state_.char_interval = 1;
	}
	void set_space_width( fract w ) { state_.space_width = w; }
	void set_char_interval( fract w ) { state_.char_interval = w; }
	void set_stroke( color c ) { state_.stroke = c; }
	void set_fill( color c ) { state_.fill = c; }
	void frame_rect( const rect & r );
	void fill_rect( const rect & r );
	void set_origin( const point &p ) { state_.origin += p; }
	void set_font( const font *font ) { state_.font = font; }
    void draw_text( const line &line);
	void move_to( point p );

	void push() { states_.push_back( state_ ); }
	void pop() { state_ = states_.back(); states_.pop_back(); }

	void draw_text( const char *p, size_t length, bool inverted );
};

///	A simple UI framework for towermac
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

	void set_border( bool border=true ) { border_width = border?1:0; }
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
		//g.draw_text( cells_[state_].text, cells_[state_].inverted );
	}
};

///	This class specifies how a text is layed out on the screen
class layout
{
	const styled_string &text_;	///	The text we want to lay out
	size_t width_;		///	Width in pixel of each line of text
	typedef enum
	{
		kLeft,
		kRight,
		kCenter,
		kJustified
	}	eAlignment;

	eAlignment alignment_;
    
    void allocate_remaining_width(size_t n_spaces, size_t n_separators, size_t remaining_width, size_t &remaining_width_for_spaces, size_t &remaining_width_for_separators, size_t &space_width, size_t &separator_width)
    {
        if(n_spaces == 0)
        {
            remaining_width_for_spaces = 0;
            space_width = 0;
            remaining_width_for_separators = remaining_width;
            separator_width = remaining_width_for_separators / n_separators;
            return;
        }
        remaining_width_for_separators = n_separators;
        remaining_width_for_spaces = remaining_width - remaining_width_for_separators;
        if(remaining_width_for_spaces <= 4*n_spaces)
        {
            space_width = remaining_width_for_spaces / n_spaces;
            separator_width = 1;
            return;
        }
        space_width = 3;
        remaining_width_for_spaces = 3*n_spaces;
        remaining_width_for_separators = remaining_width - remaining_width_for_spaces;
        do
        {
            space_width++;
            remaining_width_for_spaces += n_spaces;
            remaining_width_for_separators -= n_spaces;
            separator_width = n_separators ? remaining_width_for_separators / n_separators : 0;
        }
        while(space_width <= separator_width);
    }
    void bresenham(letter &letter, int &counter, size_t width, int items, size_t total_width)
    {
        letter.spacing += width;
        counter += total_width;
        if(counter*2 >= items)
        {
            letter.spacing++;
            counter -= items;
        }
    }
    
    void justify(line &line, size_t remaining_width)
    {
        int n_spaces = line.size()-1;
        int n_separators = 0;
        for(auto &word : line)
        {
            n_separators += word.letters.size()+1;
        }
        n_separators-=2;
        size_t remaining_width_for_separators;
        size_t remaining_width_for_spaces;
        size_t space_width;
        size_t separator_width;
        allocate_remaining_width(n_spaces, n_separators, remaining_width, remaining_width_for_spaces, remaining_width_for_separators, space_width, separator_width);
        int sp_c = 0;
        int se_c = 0;
        for(auto &word : line)
        {
            for (auto letter = word.letters.begin(); letter != std::prev(word.letters.end()); ++letter)
                bresenham(*letter, se_c, separator_width, n_separators, remaining_width_for_separators - n_separators * separator_width);
            bresenham(word.letters.back(), se_c, separator_width, n_separators, remaining_width_for_separators - n_separators * separator_width);
            bresenham(word.letters.back(), sp_c, space_width, n_spaces, remaining_width_for_spaces - n_spaces * space_width);
            bresenham(word.letters.back(), se_c, separator_width, n_separators, remaining_width_for_separators - n_separators * separator_width);
        }
    }
    
    void default_spacing(line &line)
    {
        for(auto &word : line)
        {
            if(word.tokenType == kLinebreak)
                continue;
            for (auto letter = word.letters.begin(); letter != std::prev(word.letters.end()); ++letter)
                letter->spacing = 1;
            word.letters.back().spacing = 4;
        }
    }
    
protected:
	std::vector<line> lines_;

public:
	layout( const styled_string &text, size_t width ) : text_(text)
	{
        std::vector<token> tokens;
        tokenize(tokens, text);
        size_t max_width = 0;
        lines_ = arrange(tokens, width, 2, 1, max_width); // magical values are magical
        width_ = max_width;
        for(auto line = lines_.begin(); line != std::prev(lines_.end()); ++line) // every line but the last
        {
            if(line->back().tokenType == kLinebreak) // lines ending with linebreaks aren't justified.
                default_spacing(*line);
            else
                justify(*line, max_width - widthof(*line));
        }
        default_spacing(lines_.back()); // last line isn't justified.
    }

	size_t line_count() const { return lines_.size(); }
    size_t line_width() const { return width_; }

	void render( graphics g )
	{
		g.set_font( text_.font );
		
        size_t h = 1;
		for (auto &line : lines_)
		{
			g.move_to( { 3, h } );
			g.draw_text( line );
            h+=9;
		}
	}
};

class static_text : public view
{
	styled_string text_;
	
	layout layout_;
public:
	static_text( styled_string text, size_t width ) : view{{{0,0},{width,0}}}, text_{text}, layout_{text_,bounds_.s.w-6}
	{
		set_border( true );
		set_opaque( false );	///	Superclass does not fill
		size_to_fit();
	}
	void size_to_fit()
	{
		bounds_.s.h = layout_.line_count()*9;
        bounds_.s.w = layout_.line_width()+6;
	}

	virtual void draw_self( graphics &g )
	{
		view::draw_self( g );

		g.set_fill( background_color );
		g.fill_rect( bounds_ );

		layout_.render( g );
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
