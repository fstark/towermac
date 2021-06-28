#ifndef PATH_INCLUDED__
#define PATH_INCLUDED__

#include <cassert>
#include <iostream>

class path
{
    const point origin_;
    const std::vector<int> deltas_;
    std::vector<point> screen_path_;

    std::vector<point> make_screen_path( const point &origin, const std::vector<int> &deltas )
    {
        std::vector<point> res;
        point p = origin_;
        bool vertical = false;
        res.push_back( p );
        for (int i=0;i!=deltas.size();i++)
        {
            assert( deltas[i] );
            int direction = deltas[i]/abs(deltas[i]);
            for (int j=0;j!=abs(deltas[i]);j++)
            {   if (vertical)
                    p.y+=direction;
                else
                    p.x+=direction;
                res.push_back( p );
            }
            vertical = !vertical;
        }
        return res;
    }

public:
    path( const point &origin, const std::vector<int> &deltas ) :
        origin_{ origin },
        deltas_{ deltas },
        screen_path_{ make_screen_path( origin_, deltas_ ) }
        {}

    path offset( int dx, int dy )
    {
        return { { origin_.x+dx, origin_.y+dy }, deltas_ };
    }

    bool contains( float position ) const
    {
        return position>=0 && (int)position<screen_path_.size();
    }

    point at( float position ) const
    {
        assert( contains( position ) );
        return screen_path_[(int)position];
    }

    const std::vector<point> get_points() const { return screen_path_; }

    /// 0 to 3 rotation
    int rotation_at( float position ) const
    {
        if (position==0)
            position = 1;
        auto dx = screen_path_[position].x-screen_path_[position-1].x;
        if (dx==-1)
            return 1;
        if (dx==1)
            return 3;

        auto dy = screen_path_[position].y-screen_path_[position-1].y;
        if (dy==-1)
            return 2;

        return 0;
    }

    void dump()
    {
        for (auto &p:screen_path_)
            std::clog << p.x << "," << p.y << " ";
        std::clog << "\n";
    }

};

#endif
