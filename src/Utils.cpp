
#include <algorithm>
#include <assert.h>
#include <iostream>

#include <Utils.hpp>

void applyRotation(Coord_d& P, const double c, const double s)
{
    const double x{ P.x };
    const double y{ P.y };
    P.x = x * c - y * s;
    P.y = x * s + y * c;
}

bool onLine(Line_d l1, Coord_d p)
{
    if (p.x <= std::max(l1.p1.x, l1.p2.x) && p.x >= std::min(l1.p1.x, l1.p2.x) &&
        (p.y <= std::max(l1.p1.y, l1.p2.y) && p.y >= std::min(l1.p1.y, l1.p2.y)))
        return true;

    return false;
}

int direction(Coord_d a, Coord_d b, Coord_d c)
{
    double val{ (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y) };
    if (val == 0)
    {
        return 0;       // colinear
    }
    else if (val < 0)
    {
        return 2;       // anti-clockwise direction
    }
    else
    {
        return 1;       // clockwise direction
    }
}

bool isIntersect(Line_d l1, Line_d l2)
{
    // Four direction for one lines and a point of the other line
    int dir1 = direction(l1.p1, l1.p2, l2.p1);
    int dir2 = direction(l1.p1, l1.p2, l2.p2);
    int dir3 = direction(l2.p1, l2.p2, l1.p1);
    int dir4 = direction(l2.p1, l2.p2, l1.p2);

    if (dir1 != dir2 && dir3 != dir4)
        return true; // they are intersecting

    if (dir1 == 0 && onLine(l1, l2.p1)) // when p2 of line2 is on line1
        return true;

    if (dir2 == 0 && onLine(l1, l2.p2)) // when p1 of line2 is on line1
        return true;

    if (dir3 == 0 && onLine(l2, l1.p1)) // when p2 of line1 is on line2
        return true;

    if (dir4 == 0 && onLine(l2, l1.p2)) // when p1 of line1 is on line2
        return true;

    return false;
}

bool checkCollision(const float* rocket_buffer_data, const int size_rocket_buffer, const float* floor_buffer_data, const int size_floor_buffer)
{
    assert(size_rocket_buffer == 9);
    Coord_d P1{ rocket_buffer_data[0], rocket_buffer_data[1] };
    Coord_d P2{ rocket_buffer_data[3], rocket_buffer_data[4] };
    Coord_d P3{ rocket_buffer_data[6], rocket_buffer_data[7] };

    Line_d L1{ P1, P2 };
    Line_d L2{ P1, P3 };
    Line_d L3{ P3, P2 };

    for (int i = 0; i < size_floor_buffer; i += 6)
    {
        Coord_d S1{ floor_buffer_data[i + 0], floor_buffer_data[i + 1] };
        Coord_d S2{ floor_buffer_data[i + 3], floor_buffer_data[i + 4] };
        Line_d line{ S1, S2 };

        if (isIntersect(line, L1))
        {
            std::cout << "CRASH: " << std::endl;
            std::cout << "  Floor " << int(i / 6) << ":" << std::endl;
            std::cout << "    X=" << line.p1.x << ", Y=" << line.p1.y << std::endl;
            std::cout << "    X=" << line.p2.x << ", Y=" << line.p2.y << std::endl;
            std::cout << "  Rocket:" << std::endl;
            std::cout << "    X=" << L1.p1.x << ", Y=" << L1.p1.y << std::endl;
            std::cout << "    X=" << L1.p2.x << ", Y=" << L1.p2.y << std::endl;
            return true;
        }
        if (isIntersect(line, L2))
        {
            std::cout << "CRASH: " << std::endl;
            std::cout << "  Floor " << int(i / 6) << ":" << std::endl;
            std::cout << "    X=" << line.p1.x << ", Y=" << line.p1.y << std::endl;
            std::cout << "    X=" << line.p2.x << ", Y=" << line.p2.y << std::endl;
            std::cout << "  Rocket:" << std::endl;
            std::cout << "    X=" << L2.p1.x << ", Y=" << L2.p1.y << std::endl;
            std::cout << "    X=" << L2.p2.x << ", Y=" << L2.p2.y << std::endl;
            return true;
        }
        if (isIntersect(line, L3))
        {
            std::cout << "CRASH: " << std::endl;
            std::cout << "  Floor " << int(i / 6) << ":" << std::endl;
            std::cout << "    X=" << line.p1.x << ", Y=" << line.p1.y << std::endl;
            std::cout << "    X=" << line.p2.x << ", Y=" << line.p2.y << std::endl;
            std::cout << "  Rocket:" << std::endl;
            std::cout << "    X=" << L3.p1.x << ", Y=" << L3.p1.y << std::endl;
            std::cout << "    X=" << L3.p2.x << ", Y=" << L3.p2.y << std::endl;
            return true;
        }
    }

    return false;
}
