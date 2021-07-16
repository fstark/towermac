//
//  sound_manager.hpp
//  TowerMac
//
//  Created by Zeal Iskander on 01/07/2021.
//

#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

#ifndef SOUND_MANAGER_INCLUDED__
#define SOUND_MANAGER_INCLUDED__

#include <stdio.h>
#include <array>
#include <vector>
#include <cstdint>
#include <memory>

const int FRAME = 370;  /// How many samples per ticks

/// This is a sound that can be played by the manager
class sound
{
protected:
	uint8_t *data_;   ///  The sound data
	size_t frames_;
	
public:
	/// Creates a sound from a set of frames
	sound( const uint8_t *data, size_t frames ) : frames_{frames}
	{
		data_ = (uint8_t *)malloc( frames*FRAME );
		assert( data_ );
		std::copy( data, data+frames*FRAME, data_ );
	}

	~sound()
	{
		delete data_;
		data_ = nullptr;
		frames_ = 0;
	}
	
	/// How many FRAMEs of data are in the sound
	size_t size() const { return frames_; }
	
	/// Sound start
	const uint8_t *begin() const { return data_; }

	/// Sound end
	const uint8_t *end() const { return data_+frames_*FRAME; }
};

/// The singleton that groups all the sound related functions
class sound_manager
{
	SDL_AudioSpec spec_;
	sound_manager();

	std::vector<std::unique_ptr<sound>> sounds_;
	
	std::unique_ptr<sound> load_sound( const std::string &name ) const;

	const uint8_t *background_begin_ = nullptr;
	const uint8_t *background_end_ = nullptr;
	const uint8_t *background_ = nullptr;
	const uint8_t *foreground_ = nullptr;
	const uint8_t *foreground_end_ = nullptr;
	size_t foreground_priority_ = 0;
	
	static void sdl_callback( void *, Uint8 *stream, int len );
	void next_frame( uint8_t *data );

public:
		/// Access the singleton
	static sound_manager sm;
//    void play_sound(const char *file, size_t priority, bool repeat = false) const;

		/// Loads the sound and returns a small int that references the sound.
		/// If the sound fails to load, it return 0
	size_t register_sound( const std::string &name );

		/// Plays background sound (in loop)
	void play_background( size_t snd );

		/// Plays forground sound (if priority is right)
	void play_foreground( size_t snd, int priority );
};

#endif
