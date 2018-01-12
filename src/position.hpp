//
// Created by Richard Hodges on 12/01/2018.
//

#pragma once

struct delta {
    int x;
    int y;
};

struct bounding_box;


struct position {
    int x;
    int y;

    friend bool operator==(position const& l, position const &r)  {
        return l.x == r.x and l.y == r.y;
    }

    friend bool operator!=(position const& l, position const &r) {
        return not operator==(l, r);
    }

    position& operator+=(delta const& d)
    {
        x += d.x;
        y += d.y;
        return *this;
    }

    friend position operator+(position l, delta const& d)
    {
        l += d;
        return l;
    }


};

struct bounding_box
{
    position pos;
    delta size;
};

inline int test_limits_generic(int x, int begin, int end)
{
    if (x < begin) return -1;
    if (x >= end) return 1;
    return 0;
}

inline int test_limits_x(position const& pos, bounding_box const& bb)
{
    return test_limits_generic(pos.x, bb.pos.x, bb.pos.x + bb.size.x);
}

inline int test_limits_y(position const& pos, bounding_box const& bb)
{
    return test_limits_generic(pos.y, bb.pos.y, bb.pos.y + bb.size.y);
}


