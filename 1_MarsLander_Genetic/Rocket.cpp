// Standard libraries
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

#include "Rocket.hpp"
#include "Utils.hpp"

static const double _g{-3.711};      //!< Gravity, in m/s-2
static const int _base{100};         //!< Size of the base of the rocket.
static const int _height{150};       //!< Size of the height of the rocket.
static const int _fire{25};          //!< Size of the maximum thrust power vizu.
static const double _PI{3.14159265}; //!< PI

extern const float _w; //!< Map width.
extern const float _h; //!< Map height.

/************************************************************/
Rocket::Rocket(const double f_x, const double f_y, const double f_vx,
               const double f_vy, const std::int8_t f_angle,
               const std::int8_t f_thrust, const int f_fuel)
    : pX{f_x}, pY{f_y}, x{f_x}, y{f_y}, vx{f_vx}, vy{f_vy}, ax{f_thrust *
                                                               sin(-f_angle *
                                                                   _PI / 180.)},
      ay{f_thrust * cos(-f_angle * _PI / 180.) + _g}, angle{f_angle},
      thrust{f_thrust}, fuel{f_fuel}, isAlive{true}, floor_id_crash{-1} {}

/************************************************************/
void Rocket::init(const Rocket &f_rocket) {
  pX = f_rocket.x;
  pY = f_rocket.y;
  x = f_rocket.x;
  y = f_rocket.y;
  vx = f_rocket.vx;
  vy = f_rocket.vy;
  ax = f_rocket.ax;
  ay = f_rocket.ay;
  angle = f_rocket.angle;
  thrust = f_rocket.thrust;
  fuel = f_rocket.fuel;
  isAlive = true;
}

/************************************************************/
void Rocket::updateRocket(const std::int8_t f_angle,
                          const std::int8_t f_thrust) {
  // Previous position
  pX = x;
  pY = y;

  // Update angle and power
  angle = std::min(static_cast<std::int8_t>(90),
                   std::max(static_cast<std::int8_t>(-90),
                            static_cast<std::int8_t>(angle + f_angle)));
  const double angle_rad{-angle * _PI / 180.};

  if (fuel == 0) {
    thrust = 0;
  } else {
    thrust = std::min(static_cast<std::int8_t>(4),
                      std::max(static_cast<std::int8_t>(0),
                               static_cast<std::int8_t>(thrust + f_thrust)));
  }

  // Update fuel
  fuel = std::max(0, fuel - thrust);

  /* Update Acc, speed and position:
   *    acc.x = power * sin(-angle)
   *    acc.y = power * cos(angle) + _g
   *
   *    v.x = acc.x * t + vx0
   *    v.y = acc.y * t + vy0
   *
   *    pos.x = acc.x * t^2 / 2 + vx0 * t + x0
   *    pos.y = acc.y * t^2 / 2 + vy0 * t + y0
   *
   *    with t = 1 !
   */

  ax = thrust * sin(angle_rad);
  ay = thrust * cos(angle_rad) + _g;

  x += 0.5 * ax + vx;
  y += 0.5 * ay + vy;
  y = std::max(0., y);

  vx += ax;
  vy += ay;
}

/************************************************************/
void Rocket::debug(const double f_timeSec) const {
  if (f_timeSec >= 0) {
    std::cout << "Elapsed time: " << f_timeSec << "s" << std::endl;
  }
  std::cout << "  X=" << x << ", Y=" << y << std::endl;
  std::cout << "  HSpeed=" << vx << "m/s, VSpeed=" << vy << "m/s" << std::endl;
  std::cout << "  Next:" << std::endl;
  std::cout << "    Fuel= " << fuel << "L" << std::endl;
  std::cout << "    Angle=" << int(angle) << "deg, Thrust=" << int(thrust)
            << "m/s2" << std::endl;
  std::cout << std::endl;
}

/************************************************************/
bool Rocket::isParamSuccess() const {
  return abs(angle) <= 15 && abs(vy) <= 40 && abs(vx) <= 20;
}

/************************************************************/
void updateBuffers(const Rocket &f_rocket, const double f_elapsed,
                   GLfloat *GL_rocket_buffer_data,
                   GLfloat *GL_fire_buffer_data) {
  assert(0 <= f_elapsed <= 1);

  /* Formulaes:
   *    pos.x = acc.x * t^2 / 2 + vx0 * t + x0
   *    pos.y = acc.y * t^2 / 2 + vy0 * t + y0
   */

  const double x{f_rocket.ax * std::pow(f_elapsed, 2) / 2. +
                 f_rocket.vx * f_elapsed + f_rocket.x};
  const double y{f_rocket.ay * std::pow(f_elapsed, 2) / 2. +
                 f_rocket.vy * f_elapsed + f_rocket.y};

  Coord_d F{0, -1. * _fire * f_rocket.thrust};
  Coord_d P1{0, _height};
  Coord_d P2{_base / 2.f, 0};
  Coord_d P3{-_base / 2.f, 0};

  const double angle_rad{f_rocket.angle * _PI / 180.};
  const double c{cos(angle_rad)};
  const double s{sin(angle_rad)};

  applyRotation(F, c, s);
  applyRotation(P1, c, s);
  applyRotation(P2, c, s);
  applyRotation(P3, c, s);

  GL_rocket_buffer_data[0] = static_cast<GLfloat>(P1.x + x);
  GL_rocket_buffer_data[1] = static_cast<GLfloat>(P1.y + y);
  GL_rocket_buffer_data[3] = static_cast<GLfloat>(P2.x + x);
  GL_rocket_buffer_data[4] = static_cast<GLfloat>(P2.y + y);
  GL_rocket_buffer_data[6] = static_cast<GLfloat>(P3.x + x);
  GL_rocket_buffer_data[7] = static_cast<GLfloat>(P3.y + y);
  GL_fire_buffer_data[0] = static_cast<GLfloat>(x);
  GL_fire_buffer_data[1] = static_cast<GLfloat>(y);
  GL_fire_buffer_data[3] = static_cast<GLfloat>(F.x + x);
  GL_fire_buffer_data[4] = static_cast<GLfloat>(F.y + y);

  // From [0, _w] x [0, _h] to [-1, 1] x [-1, 1]
  GL_rocket_buffer_data[0] = 2.f * GL_rocket_buffer_data[0] / _w - 1.f;
  GL_rocket_buffer_data[1] = 2.f * GL_rocket_buffer_data[1] / _h - 1.f;
  GL_rocket_buffer_data[3] = 2.f * GL_rocket_buffer_data[3] / _w - 1.f;
  GL_rocket_buffer_data[4] = 2.f * GL_rocket_buffer_data[4] / _h - 1.f;
  GL_rocket_buffer_data[6] = 2.f * GL_rocket_buffer_data[6] / _w - 1.f;
  GL_rocket_buffer_data[7] = 2.f * GL_rocket_buffer_data[7] / _h - 1.f;
  GL_fire_buffer_data[0] = 2.f * GL_fire_buffer_data[0] / _w - 1.f;
  GL_fire_buffer_data[1] = 2.f * GL_fire_buffer_data[1] / _h - 1.f;
  GL_fire_buffer_data[3] = 2.f * GL_fire_buffer_data[3] / _w - 1.f;
  GL_fire_buffer_data[4] = 2.f * GL_fire_buffer_data[4] / _h - 1.f;
}
