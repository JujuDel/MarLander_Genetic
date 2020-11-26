#ifndef UTILS_HPP
#define UTILS_HPP

#include <GL/glew.h>

#include "Rocket.hpp"

//! @brief  Coord's struct
struct Coord_d {
  double x;
  double y;
};

//! @brief  Segment's struct
struct Line_d {
  Coord_d p1;
  Coord_d p2;
};

//! @brief  Apply the given rotation on the point `P`.
//!
//! @param[out] P  The point to update.
//! @param[in]  c  cos(angle).
//! @param[in]  v  sin(angle).
void applyRotation(Coord_d &P, const double c, const double s);

//! @brief  Given two lines, check if they intersect.
//!
//! @param[in] l1, l2  The lines to check.
//!
//! @return True if l1 and l2 intersects, else false.
bool isIntersect(Line_d l1, Line_d l2);

#endif
