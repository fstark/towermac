#ifndef CORE_INCLUDED__
#define CORE_INCLUDED__

#include <cstdint>
#include <cmath>
#include <cassert>

template <class T>
class node
{
public:
    T *next_;
    T *prev_;

    node() : next_{(T*)this}, prev_{(T*)this} {}
    virtual ~node() {}

    void remove() { next_->prev_ = prev_; prev_->next_ = next_; }
    void insert_after( T *prev ) { prev_ = prev; next_ = prev_->next_; prev_->next_ = (T*)this; next_->prev_ = (T*)this; }
};

template <class T>
class dlist : public node<T>
{
public:
    T *begin() const { return node<T>::next_; };
    T *end() const { return (T*)this; }
    bool is_empty() const { return begin()==end(); }
    void add( T *t ) { t->insert_after((T*)this); }
};

const size_t ZoomFactor = 2;
const size_t MAP_SIZE = 21*16;

const size_t SCREEN_WIDTH = 512;
const size_t SCREEN_HEIGHT = 342;

const size_t kMapY = (SCREEN_HEIGHT-MAP_SIZE)/2;
const size_t kMapX = kMapY;

const size_t kBaseX = 106;
const size_t kBaseY = 97;





struct vector2f;

struct point
{
    size_t x;
    size_t y;

//    point operator-( const point &o ) const { return point{ x-o.x, y-o.y }; }
    vector2f operator/( double v ) const;
    operator vector2f() const;
};

struct vector2f
{
    double x;
    double y;

    vector2f() {}
    vector2f( double ax, double ay ) : x{ax}, y{ay} {}
    vector2f( const point &p ) : x{(double)p.x}, y{(double)p.y} {}

    vector2f operator/( double v ) const { return vector2f{ x/v, y/v }; }
    vector2f operator*( double v ) const { return vector2f{ x*v, y*v }; }
    vector2f operator+( const vector2f &o ) const { return vector2f{ x+o.x, y+o.y }; }
    vector2f operator-( const vector2f &o ) const { return vector2f{ x-o.x, y-o.y }; }

    operator point() const { return point{ (size_t)x, (size_t)y }; }
};

inline point::operator vector2f() const { return vector2f{ (double)x, (double)y }; }
inline vector2f point::operator/( double v ) const { return vector2f{ x/v, y/v }; }

inline double distance( const point p0, const point p1 )
{
    return sqrt( (p0.x-p1.x)*(p0.x-p1.x) + (p0.y-p1.y)*(p0.y-p1.y) );
}

inline double norm( const vector2f &v )
{
    return sqrt( v.x*v.x + v.y*v.y );
}

inline double norm( const point &v )
{
    return sqrt( v.x*v.x + v.y*v.y );
}

inline vector2f normalize( const vector2f &v )
{
    auto n = norm( v );
    return v/n;
}

inline vector2f normalize( const point &v )
{
    auto n = norm( v );
    return v/n;
}

template <typename V> bool in_map( const V &p ) { return p.x>=kMapX && p.y>=kMapY && p.x<MAP_SIZE+kMapX && p.y<MAP_SIZE+kMapY; }

#endif
