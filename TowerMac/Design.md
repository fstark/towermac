[ARGH: we create a sprite for every mob! (or bullet, etc)]

[ideas: a 'ticks' type instead of size_t for duration]

* Choice of 3 tower in 3 random
* 3 waves + 1 boss
*   each wave => choose 1 powerup
* 8 times

* Starting towers shoot at the mouse

* a round = 3 wawe + 1 boss

* There are 16 towers position, 2 are opened at each round

* There are n possible paths

Each round selects a number of paths

On each path, there are 4 wawes (3 + boss), sometime empty, which are timestamps + mobs


struct mob_def
{
    std::string sprite_name;
    size_t hp;
    float speed;
    size_t damage;
};

struct mob_group_def
{
    size_t count;
    std::string mob_key;
    size_t spawn_delay;
    size_t spawn_rate;
};

struct wavelet_def
{
    size_t lane;
    std::vector<mob_group_def> mob_groups;
};

struct wave_def
{
    std::vector<wavelet_def> wavelets;
};

mob_defs.insert( "mob0", mob_def{ "mob04-3", 100, 1, 9999 } );
wawedef{ { 0, { { 10, "mob0", 0, 30 } } } };



"mob0" = {
    "sprite" = "mob04-3";
    "hp" = 
    "speed" = 
    "damage" =
    }
"mob1" = {
    "sprite" = "mob04-3";
    "margins" = [ 12, 12 ]
    }
"mob2" = {
    "sprite" = "mob04-3";
    "margins" = [ 12, 12 ]
    "health" = xx;
    }


{
    waves =
    [
        [
            [
                lane = 0;
                mobs = [ [ 10, "mob0", spawn_delay=0, spawn_rate=30 ] ]
            ]
        ]
        [
            [
                lane = 1;
                mobs = [ [ 8, "mob1" ] ]
            ]
        ]
        [
            [
                lane = 0;
                mobs = [ [ 7, "mob0" ], [ 3, "mob2" ] ]
            ]
        ]
        [
            [
                lane = 0;
                mobs = [ [ 5, "mob0" ] ]
            ]
            [
                lane = 1;
                mobs = [ [ 5, "mob1" ] ]
            ]
        ]
    ]
}
