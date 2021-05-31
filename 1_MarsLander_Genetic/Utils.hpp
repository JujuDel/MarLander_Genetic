#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

#include <GL/glew.h>

#include "Genetic.hpp"
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

//! @brief  Write the solution vector into a file.
//!
//! @param[in] f_filename             Name of the file.
//! @param[in] f_chromosomeSolution   The chromosome solution to dump.
//! @param[in] f_idxStart             Frontier idx between incremental and
//!                                   solution. Default is 0.
//! @param[in] f_solutionIncremental  The incremental solution vector to dump.
//!                                   Default is empty.
void writeSolution(const std::string &f_fileName,
                   const Chromosome *f_chromosomeSolution,
                   const int f_idxStart = 0,
                   const std::vector<Gene> &f_solutionIncremental = {});

#endif
