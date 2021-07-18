#ifndef IMAGE_INCLUDED__
#define IMAGE_INCLUDED__

#include <SDL2/SDL.h>

#ifdef __linux__
#include <SDL2/SDL_image.h>
#else // macosx
#include <SDL2_image/SDL_image.h>
#endif

#include <iostream>

extern SDL_Renderer *gRenderer;

///	This is an image that can be drawn on screen
class image
{
	SDL_Texture* texture_;	///	The SDL texture containing the image
	SDL_Rect rect_;			///	The bounds of the image

	bool offset_ = true;	///	If true, we draw it centered (#### it should no be there)

public:
	image( SDL_Surface *s, bool offset ) : offset_{ offset }
	{
		assert( s );
		texture_ = SDL_CreateTextureFromSurface( gRenderer, s );
		assert( texture_ );
		rect_ = s->clip_rect;
	}

	image( const char *s, bool offset = true ) : offset_{ offset }
	{
		SDL_Surface *image_ = IMG_Load( s );
		if (!image_)
		{
			std::cerr << "Cannot load image [" << s << "]\n";
			throw "Cannot load image";
		}
		assert( image_ );
		texture_ = SDL_CreateTextureFromSurface( gRenderer, image_ );
		assert( texture_ );
		rect_ = image_->clip_rect;
		SDL_FreeSurface( image_ );
	}

	~image()
	{
		SDL_DestroyTexture( texture_ );
	}

	void render( const point &p, int rotate=0 ) const
	{
		SDL_Rect dst_rect = rect_;
		if (offset_)
		{
			dst_rect.x = (int)p.x-rect_.w/2;
			dst_rect.y = (int)p.y-rect_.h/2;
		}
		else
		{
			dst_rect.x = (int)p.x;
			dst_rect.y = (int)p.y;
		}
		int result = SDL_RenderCopyEx( gRenderer, texture_, &rect_, &dst_rect, rotate*90, nullptr, SDL_FLIP_NONE );
		// int result  = SDL_BlitScaled( image_, &rect_, gScreen, &dst_rect );
		// int result  = SDL_BlitSurface( image_, &rect_, gScreen, &dst_rect );
		if (result<0)
		{
			std::cerr << "Error = " << result << "\n";
			throw "Blit error";
		}
	}
	
	size_t height() const { return rect_.h; }
	size_t width() const { return rect_.w; }
	size size() const { return { (size_t)rect_.h, (size_t)rect_.w}; }
};

#endif
