#ifndef SPRITE_INCLUDED__
#define SPRITE_INCLUDED__

#include <SDL2/SDL.h>

#ifdef __linux__
#include <SDL2/SDL_image.h>
#else // macosx
#include <SDL2_image/SDL_image.h>
#endif

#include <iostream>

extern SDL_Renderer *gRenderer;

class sprite
{
	SDL_Surface* sprite_;
	SDL_Texture* texture_;
	SDL_Rect rect_;

	bool offset_ = true;

public:
	sprite( const char *s, bool offset = true ) : offset_{ offset }
	{
		sprite_ = IMG_Load( s );
		if (!sprite_)
		{
			std::cerr << "Cannot load sprite [" << s << "]\n";
			throw "Cannot load sprite";
		}
		assert( sprite_ );
		texture_ = SDL_CreateTextureFromSurface( gRenderer, sprite_ );
		assert( texture_ );
		rect_ = sprite_->clip_rect;
	}

	~sprite()
	{
		SDL_FreeSurface( sprite_ );
		SDL_DestroyTexture( texture_ );
	}

	void render( const point &p, int rotate=0 ) const
	{
		SDL_Rect dst_rect = rect_;
		if (offset_)
		{
			dst_rect.x = p.x-rect_.w/2;
			dst_rect.y = p.y-rect_.h/2;
		}
		else
		{
			dst_rect.x = p.x;
			dst_rect.y = p.y;
		}
		int result = SDL_RenderCopyEx( gRenderer, texture_, &rect_, &dst_rect, rotate*90, nullptr, SDL_FLIP_NONE );
		// int result  = SDL_BlitScaled( sprite_, &rect_, gScreen, &dst_rect );
		// int result  = SDL_BlitSurface( sprite_, &rect_, gScreen, &dst_rect );
		if (result<0)
		{
			std::cerr << "Error = " << result << "\n";
			throw "Blit error";
		}
	}
};

#endif
