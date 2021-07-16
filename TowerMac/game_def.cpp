#include "game_def.hpp"

const game_def game_def::spec;

class resource_def
{
	FILE *file_;
	char buffer[1025];
	bool saved_line_ = false;   /// If true, the content of the buffer is already correct for the next read_line
	
	bool done = false;
	
	void fgets()
	{
		while (!done)
		{
			done = !::fgets( buffer, 1024, file_ );
			if (buffer[strlen(buffer)-1]=='\n')
				buffer[strlen(buffer)-1] = 0;
			if (buffer[0]!='#')
				return ;
		}
	}

	char *current_ = nullptr;
	char *token_ = nullptr;

	void next_token( const char *sep = " " )
	{
		token_ = strsep( &current_, sep );
	}

public:
	resource_def( FILE *file ) : file_{ file } {}
	~resource_def() { fclose(file_) ;}

	bool eol() { return current_==nullptr || *current_==0; }

	bool load_line()
	{
		if (saved_line_)
		{
			current_ = buffer;
			saved_line_ = false;
			return true;
		}

		fgets();
		if (!done)
		{
			current_ = buffer;
		}
		return !done;
	}
	
	std::string read_name() { next_token(); assert( token_); std::string res{ token_ }; return res; }
	std::string read_name_opt() { if (*current_=='"') return ""; next_token(); if (!token_) return ""; std::string res{ token_ }; return res; }
	int read_int() { next_token(); assert( token_); int res = atoi(token_); return res; }
	size_t read_size_t() { next_token(); assert( token_); size_t res = atoi(token_); return res; }
	point read_point() { auto x = read_size_t(); auto y = read_size_t(); return {x,y}; }
	std::string read_text() { assert( *current_++=='"' ); next_token( "\"" ); assert( token_); std::string res{ token_ }; return res; }

	size_t count_tabs() const { const char *p = current_; while (*p=='\t') p++; return p-current_; }
	void skip_tabs( size_t tab_count )
	{
		while (tab_count--)
		{
			if (*current_!='\t')
				throw "Missing tab at line start";
			current_++;
		}
	}

	void unload_line()
	{
		saved_line_ = true;
	}
};

class resource_manager
{
public:
	static std::unique_ptr<resource_def> open( const std::string &file )
	{
		auto f = fopen( file.c_str(), "r" );
		if (!f)
		{
			std::cerr << "Cannot open " << file << "\n";
			throw "Cannot load file";
		}
		return std::make_unique<resource_def>( f );
	}
};

void load_lanes( std::map<std::string,path> &def, const std::string &file )
{
	auto f = resource_manager::open( file );
	
	f->load_line();
	auto name = f->read_name();
	std::vector<point> p;

	while (!f->eol())
		p.push_back( f->read_point()+point{ kMapX, kMapY } );

	def.insert( { name, p } );
}

void load_spots( std::map<std::string,spot> &def, const std::string &file )
{
	auto f = resource_manager::open( file );
	
	while (f->load_line())
	{
		auto name = f->read_name();
		auto pt = f->read_point();
		auto description = f->read_text();

		std::cout << "    [" << description << "]\n";

		def.insert( { name, { pt, description } } );
	}
}

void load_groups( std::map<std::string,spot_group> &def, const std::string &file, const game_def &gd )
{
	auto f = resource_manager::open( file );
	while (f->load_line())
	{
		spot_group sg;
		
		auto name = f->read_name();

		std::string s;
		while ((s=f->read_name_opt())!="")
			sg.spots.push_back( gd.spot_by_name( s ) );
		sg.description = f->read_text();
		
		def.insert( { name, sg } );
	}
}

void load_mobs( std::map<const std::string,mob_def> &def, const std::string &file )
{
	auto f = resource_manager::open( file );
	
	while (f->load_line())
	{
		auto name = f->read_name();
		auto image = f->read_name();

		auto hp = f->read_size_t();
		auto speed = f->read_size_t();
		auto damage = f->read_size_t();

		def.insert( { name, mob_def{ image, hp, (float)speed, damage } } );
	}
}

void load_waves( std::vector<wave_def> &def, const std::string &file )
{
	auto f = resource_manager::open( file );
	while (f->load_line())
	{
		auto description = f->read_text();
		std::vector<wavelet_def> wavelets;
		
		while (f->load_line())
		{
			if (f->count_tabs()!=1)
			{
				f->unload_line();
				break;
			}

			f->skip_tabs( 1 );
			auto lane = f->read_name();
			
			wavelet_def wd{ lane };
			
			while (f->load_line())
			{
				if (f->count_tabs()!=2)
				{
					f->unload_line();
					break;
				}
				f->skip_tabs( 2 );
			 
				auto count = f->read_size_t();
				auto mob_key = f->read_name();
				auto spawn_delay = f->read_size_t();
				auto spawn_rate = f->read_size_t();
				
				wd.mob_groups.push_back( { count, mob_key, spawn_delay, spawn_rate } );
			}
			
			wavelets.push_back( wd );
		}

		def.push_back( { wavelets } );
	}
}

game_def::game_def()
{
		//	Load defintions
	load_lanes( lane_defs_, "assets/defs/lanes.def" );
	load_spots( spot_defs_, "assets/defs/spots.def" );
	load_groups( groups_, "assets/defs/groups.def", *this );

	load_mobs( mob_defs_, "assets/defs/mobs.def" );
	load_waves( wave_defs_, "assets/defs/waves.def" );

		//  Link definitions
	for (auto &w:wave_defs_)
		for (auto &wl:w.wavelets)
		{
			wl.path_ = &lane_defs_.at(wl.lane_key);
			for (auto &mg:wl.mob_groups)
				mg.mob_def_ = &mob_defs_.at(mg.mob_key);
		}

}
