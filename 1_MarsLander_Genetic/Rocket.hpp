#ifndef ROCKET_HPP
#define ROCKET_HPP

#include <GL/glew.h>

//! @brief  Rocket's struct
struct Rocket {
  double pX, pY;      //!< Previous coordinates.
  double x, y;        //!< Current coordinates.
  double vx, vy;      //!< Rocket's velocity.
  double ax, ay;      //!< Rocket's acceleration.
  std::int8_t angle;  //!< Rocket's angle.
  std::int8_t thrust; //!< Rocket's thrust power.
  int fuel;           //!< Rocket's remaining fuel.
  bool isAlive;       //!< Whether or not the Rocket is alive.
  int floor_id_crash; //!< ID of the floor where the Rocket crashed.

  //! @brief  C'tor.
  //!
  //! @param[in] f_x, f_y    Initial coordinates.
  //! @param[in] f_vx, f_vy  Initial velocity.
  //! @param[in] f_angle     Initial angle.
  //! @param[in] f_thrust    Initial thrust power.
  //! @param[in] f_fuel      Initial fuel quantity.
  Rocket(const double f_x = 0., const double f_y = 0., const double f_vx = 0.,
         const double f_vy = 0., const std::int8_t f_angle = 0,
         const std::int8_t f_thrust = 0, const int f_fuel = 0);

  //! @brief  Re-init the rocket value.
  //!
  //! @param[in] f_rocket  The rocket to re-init the values with.
  void init(const Rocket &f_rocket);

  //! @brief  Apply the next angle and thrust requests.
  //!
  //! @param[in] f_angle  The angle request.
  //! @param[in] f_thrust The thrust power request.
  void updateRocket(const std::int8_t f_angle, const std::int8_t f_thrust);

  //! @brief  Prints out the rocket info on the command
  //!
  //! @param[in] f_timeSec  The elapsed time. Default is -1: do not display the
  //!                       elapsed time.
  void debug(const double f_timeSec = -1.) const;

  //! @brief  Whether or not the current angle and speeds are in the good range
  //!         to land.
  //!
  //! @return True if landing could be successful, else False.
  bool isParamSuccess() const;
};

#endif
