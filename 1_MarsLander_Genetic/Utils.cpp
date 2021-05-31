// Include standard headers
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Include GLEW
#include <GL/glew.h>

#include "Utils.hpp"

/************************************************************/
void applyRotation(Coord_d &P, const double c, const double s) {
  const double x{P.x};
  const double y{P.y};
  P.x = x * c - y * s;
  P.y = x * s + y * c;
}

//! @brief  Given a line and a point check if the point is on the line.
//!
//! @param[in] l  A line.
//! @param[in] p  A point.
//!
//! @return True if p is on l, else false.
bool onLine(const Line_d &l, const Coord_d &p) {
  if (p.x <= std::max(l.p1.x, l.p2.x) && p.x >= std::min(l.p1.x, l.p2.x) &&
      (p.y <= std::max(l.p1.y, l.p2.y) && p.y >= std::min(l.p1.y, l.p2.y)))
    return true;

  return false;
}

//! @brief  Find the orientation of an ordered triplet of points (a, b, c).
//!
//! The function returns following values
//!     - 0 --> a, b and c are collinear
//!     - 1 --> Clockwise
//!     - 2 --> Counterclockwise
//!
//! @param[in] a, b, c  The points to be evaluated.
//!
//! @return 0, 1 or 2.
//-----------------------------------------------------------------
int direction(Coord_d a, Coord_d b, Coord_d c) {
  double val{(b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y)};
  if (val == 0) {
    return 0; // colinear
  } else if (val < 0) {
    return 2; // anti-clockwise direction
  } else {
    return 1; // clockwise direction
  }
}

/************************************************************/
bool isIntersect(Line_d l1, Line_d l2) {
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

/************************************************************/
void writeSolution(const std::string &f_fileName,
                   const Chromosome *f_chromosomeSolution, const int f_idxStart,
                   const std::vector<Gene> &f_solutionIncremental) {
  std::ofstream out_file(f_fileName);
  if (out_file.is_open()) {
    for (int i = 0; i < f_idxStart; ++i) {
      if (i > 0)
        out_file << " ";
      out_file << f_solutionIncremental[i].angle << ","
               << f_solutionIncremental[i].thrust;
    }
    for (int i = f_idxStart; i < _CHROMOSOME_SIZE; ++i) {
      if (i > 0)
        out_file << " ";
      out_file << f_chromosomeSolution->getGene(i)->angle << ","
               << f_chromosomeSolution->getGene(i)->thrust;
    }
    out_file.close();
  }
}
