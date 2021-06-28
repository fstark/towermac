#include "game_def.hpp"

const game_def game_def::spec;

game_def::game_def()
{
    lane_defs_.insert( { "modem0", { point{ 335+kMapX, 84+kMapY }, { 286-335, 180-84, 215-286, 144-180, 132-215 } } } );
    spot_defs_.insert( { "connector0", { { 246, 79 }, "This is the analog connector, which connects to the analog board, where the power supply and the video is located." } } );
    spot_defs_.insert( { "rom-hi0", { { 182, 127 }, "The ROM-hi contains the high order 8bits of the Macintosh 64K ROM code" } } );
    spot_defs_.insert( { "rom-lo0", { { 182, 167 }, "The ROM-lo contains the low order 8bits of the Macintosh 64K ROM code" } } );
    spot_defs_.insert( { "iwm0", { { 182, 208 }, "The Integrated Woz Machine controls the floppy drive" } } );
    mob_defs_.insert( { "mob0", mob_def{ "assets/mobs/mob04-3.bmp", 100, 1, 9999 } } );
    mob_defs_.insert( { "mob1", mob_def{ "assets/mobs/mob04-3.bmp", 50, 1, 9999 } } );
    wave_defs_.push_back(
        {
            {
                { "modem0", { { 10, "mob0", 0, 30 }, { 5, "mob1", 90, 15 } } } }
            }
        );

        //  link

    for (auto &w:wave_defs_)
        for (auto &wl:w.wavelets)
        {
            wl.path_ = &lane_defs_.at(wl.lane_key);
            for (auto &mg:wl.mob_groups)
                mg.mob_def_ = &mob_defs_.at(mg.mob_key);
        }

}
