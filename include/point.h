#ifndef SCOTT_ROGUELIKE_POINT_H
#define SCOTT_ROGUELIKE_POINT_H

#include <iostream>     // remove
#include <cassert>

/**
 * This point class is specially designed for our 3d rogue game. It
 * is heavily inspired from Sauerbraten's ivec class.
 *
 * The point class defines a 3d (x,y,z) coordinate space oriented around
 * the given d axis. X is the row, y is the column and z is the coordinate
 * of the reference axis (Z/depth being the default)
 *
 * There is some fun bit tricks in here to make dealing with this really
 * fast, and actually intutitive once you get the hang of things.
 */

// X=0, Y=1, Z=2
const int PR[3] = { 1, 2, 0 };  // row
const int PC[3] = { 2, 0, 1 };  // col
const int PD[3] = { 0, 1, 2 };  // depth

class Point
{
public:
    Point()
        : x(0), y(0), z(0)
    {
    }

    Point( int x_, int y_, int z_ )
        : x(x_), y(y_), z(z_)
    {
    }

    Point( int axis, int x_, int y_, int z_ )
    {
        assert( axis >= 0 && axis < 3 );

        v[PR[axis]] = x_;
        v[PC[axis]] = y_;
        v[PD[axis]] = z_;
    }

    Point( const Point& point )
        : x(point.x), y(point.y), z(point.z)
    {
    }

    const Point& operator = ( const Point& rhs )
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }

    bool operator == ( const Point& rhs ) const
    {
        return ( x == rhs.x && y == rhs.y && z == rhs.z );
    }

    bool operator != ( const Point& rhs ) const
    {
        return ( x != rhs.x || y != rhs.y || z != rhs.z );
    }

    int& operator[] ( int index )
    {
        assert( index >= 0 && index < 3 );
        return v[index];
    }

    int  operator[] ( int index ) const
    {
        assert( index >= 0 && index < 3 );
        return v[index];
    }

    bool isZero() const
    {
        return ( x == 0 && y == 0 && z == 0 );
    }

    friend std::ostream& operator << ( std::ostream& s, const Point& p );

protected:
    union 
    {
        struct { int x; int y; int z; };
        int v[3];
    };
};

#endif