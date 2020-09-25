#ifndef UTILS_HPP
#define UTILS_HPP

struct Coord_d
{
    double x;
    double y;
};

struct Coord_i
{
    int x;
    int y;
};

struct Line_d
{
    Coord_d p1;
    Coord_d p2;
};

void applyRotation(Coord_d& P, const double c, const double s);

bool isIntersect(Line_d l1, Line_d l2);

bool checkCollision(const float* rocket_buffer_data, const int size_rocket_buffer, const float* floor_buffer_data, const int size_floor_buffer);

#endif
