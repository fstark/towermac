//
//  sound_manager.cpp
//  TowerMac
//
//  Created by Zeal Iskander on 01/07/2021.
//

#include "sound_manager.hpp"
#include <iostream>
#include <algorithm>

#define MIX_AUDIO

sound_manager sound_manager::sm;

//  Stream needs to be filled by 'len' bytes of data
//  which is a frame of FRAME bytes in our case
void sound_manager::sdl_callback( void *, Uint8 *stream, int len )
{
	assert( len==FRAME );

	sm.next_frame( stream );
}

void sound_manager::next_frame( uint8_t *data )
{
	if (background_==nullptr)
	{
		for (auto i=0;i!=FRAME;i++)
			data[i] = 128;
		return ;
	}

#ifdef MIX_AUDIO
	if (foreground_ && background_)
	{
		//  mix
		for (int i=0;i!=FRAME;i++)
//            data[i] = ((int)foreground_[i]*1+(int)background_[i]*1)/2;
		{
			int s = (int)foreground_[i]+(int)background_[i]-128;
			if (s<0) s = 0;
			if (s>255) s = 255;
			data[i] = s;
		}
	  }
	else
#endif
		if (foreground_)
			std::copy( foreground_, foreground_+FRAME, data );
		else
			std::copy( background_, background_+FRAME, data );
	
	//  Advance foreground if needed
	if (foreground_)
	{
		foreground_ += FRAME;
		if (foreground_==foreground_end_)
		{
			foreground_ = foreground_end_ = nullptr;
		}
	}

	//  Always advance background
	background_ += FRAME;
	if (background_==background_end_)
		background_ = background_begin_;
}

sound_manager::sound_manager()
{
		//  The empty sound
	
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		throw "Could not initialize audio";
	spec_.freq = 22200;
	spec_.format = AUDIO_U8;
	spec_.channels = 1;
	spec_.samples = FRAME;
	spec_.callback = sdl_callback;
	if ( SDL_OpenAudio(&spec_, &spec_) < 0 ) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_PauseAudio(0);
}


std::unique_ptr<class sound> sound_manager::load_sound( const std::string &name ) const
{
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;

	auto cname = name.c_str();
	
	if (SDL_LoadWAV( cname, &wave, &data, &dlen ) == NULL)
	{
		std::cerr << "Cannot load " << name << " : " << SDL_GetError() << "\n";
		return nullptr;
	}
	auto err = SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq, spec_.format, spec_.channels, spec_.freq);
	if (err==-1)
	{
		std::cerr << "Cannot comvert " << name << " : " << SDL_GetError() << "\n";
		return nullptr;
	}
	if (err==1)
	{
		std::clog << "#### Will convert sound to 8 bits PCM\n";
		std::clog << "  freq from " << wave.freq << " to " << spec_.freq << "\n";
		std::clog << "  format from " << wave.format << " to " << spec_.format << "\n";
		std::clog << "  channels from " << (int)wave.channels << " to " << (int)spec_.channels << "\n";
		std::clog << "  on " << wave.samples << " samples\n";
	}

	cvt.len = dlen;
	cvt.buf = (unsigned char *)malloc(cvt.len*cvt.len_mult);
	memcpy(cvt.buf, data, cvt.len);
	SDL_ConvertAudio(&cvt);
	SDL_FreeWAV(data);
	
	auto sound_len = cvt.len_cvt/FRAME;
	std::clog << "Truncating to " << sound_len  << " frames (" << sound_len*FRAME << " bytes)\n";

	return std::make_unique<class sound>( cvt.buf, sound_len );
}

size_t sound_manager::register_sound( const std::string &name )
{
	auto sound = load_sound( name );
	if (!sound)
		return 0;
	sounds_.emplace_back( std::move(sound) );
	return sounds_.size()-1;
}

void sound_manager::play_background( size_t snd )
{
	background_ = background_begin_ = sounds_[snd]->begin();
	background_end_ = sounds_[snd]->end();
}

	/// Plays forground sound (if priority is right)
void sound_manager::play_foreground( size_t snd, int priority )
{
	if (foreground_ && foreground_priority_>=priority)
		return;     //  Skip if already playing an important sound

	foreground_ = sounds_[snd]->begin();
	foreground_end_ = sounds_[snd]->end();
	foreground_priority_ = priority;
}
