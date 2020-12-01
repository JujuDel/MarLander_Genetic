#ifndef LEVELS_HPP
#define LEVELS_HPP

#include <vector>

#include "Rocket.hpp"

//! @brief  Levels
class Levels {
public:
  //! @brief  C'tor
  //!
  //! Sets the values of the floor, and the rocket for every available levels.
  //! The levels are 1-based indexed. For the i-th level:
  //!   - floors[i] contains 2 * sizes[i] values, the coordinates of the j-th
  //!   point being at {x: floors[i][2 * j], y: floors[i][2 * j + 1]}
  //!   - rockets[i] contains the initial rocket
  Levels();

  //! @brief  Get the coordinates of the floor of the idx-th level.
  //!
  //! @param[in] idx  The requested level index.
  //!
  //! @return  The floor coordinates.
  const std::vector<int> &getFloor(const size_t idx) const;

  //! @brief  Get the size (the number of coordinates) of the floor of the
  //! idx-th level.
  //!
  //! @param[in] idx  The requested level index.
  //!
  //! @return  The size of the floor.
  const size_t &getSizeFloor(const size_t idx) const;

  //! @brief  Get the initial Rocket state of the idx-th level.
  //!
  //! @param[in] idx  The requested level index.
  //!
  //! @return  The Rocket.
  const Rocket &getRocket(const size_t idx) const;

private:
  const std::vector<int> floors[7]; //!< Floors data of the levels
  const size_t sizes[7];            //!< Sizes of the floors of the levels
  const Rocket rockets[7];          //!< Initial Rockets of the levels
};

#endif
