// Standard libraries
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// Include GLEW
#include <GL/glew.h>

#include <Rocket.hpp>

static const double _g{ -3.711 }; // m/s-2
static const int _base{ 100 };
static const int _height{ 150 };
static const int _fire{ 25 };
static const double _PI{ 3.14159265 };

  /*******************************/
 /*            C'tor            */
/*******************************/
Rocket::Rocket(const double f_x, const double f_y, const double f_vx, const double f_vy, const std::int8_t f_angle, const std::int8_t f_power, const int f_fuel) :
    pX{ f_x },
    pY{ f_y },
    x{ f_x },
    y{ f_y },
    vx{ f_vx },
    vy{ f_vy },
    ax{ f_power * sin(-f_angle * _PI / 180.) },
    ay{ f_power * cos(-f_angle * _PI / 180.) + _g },
    angle{ f_angle },
    power{ f_power },
    fuel{ f_fuel },
    isAlive{ true },
    floor_id_crash{ -1 },
    GL_rocket_buffer_data{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    GL_fire_buffer_data{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
{}

  /*******************************/
 /*            init             */
/*******************************/
void Rocket::init(const Rocket& f_rocket)
{
    pX = f_rocket.x;
    pY = f_rocket.y;
    x = f_rocket.x;
    y = f_rocket.y;
    vx = f_rocket.vx;
    vy = f_rocket.vy;
    ax = f_rocket.ax;
    ay = f_rocket.ay;
    angle = f_rocket.angle;
    power = f_rocket.power;
    fuel = f_rocket.fuel;
    isAlive = true;
}

  /*******************************/
 /*         UpdateFuel          */
/*******************************/
void Rocket::updateFuel()
{
    fuel = std::max(0, fuel - power);
}

  /*******************************/
 /*       UpdatePosition        */
/*******************************/
void Rocket::updateBuffers(const double f_elapsed)
{
    assert(0 <= f_elapsed <= 1);

    /* Formulas:
    *    pos.x = acc.x * t^2 / 2 + vx0 * t + x0
    *    pos.y = acc.y * t^2 / 2 + vy0 * t + y0
    */

    const double l_x{ ax * std::pow(f_elapsed, 2) / 2. + vx * f_elapsed + x };
    const double l_y{ ay * std::pow(f_elapsed, 2) / 2. + vy * f_elapsed + y };

    Coord_d F{ 0, -1. * _fire * power };
    Coord_d P1{ 0, _height };
    Coord_d P2{ _base / 2.f, 0 };
    Coord_d P3{ -_base / 2.f, 0 };

    const double angle_rad{ angle * _PI / 180. };
    const double c{ cos(angle_rad) };
    const double s{ sin(angle_rad) };

    applyRotation(F, c, s);
    applyRotation(P1, c, s);
    applyRotation(P2, c, s);
    applyRotation(P3, c, s);

    GL_rocket_buffer_data[0] = static_cast<GLfloat>(P1.x + l_x);
    GL_rocket_buffer_data[1] = static_cast<GLfloat>(P1.y + l_y);
    GL_rocket_buffer_data[3] = static_cast<GLfloat>(P2.x + l_x);
    GL_rocket_buffer_data[4] = static_cast<GLfloat>(P2.y + l_y);
    GL_rocket_buffer_data[6] = static_cast<GLfloat>(P3.x + l_x);
    GL_rocket_buffer_data[7] = static_cast<GLfloat>(P3.y + l_y);
    GL_fire_buffer_data[0] = static_cast<GLfloat>(l_x);
    GL_fire_buffer_data[1] = static_cast<GLfloat>(l_y);
    GL_fire_buffer_data[3] = static_cast<GLfloat>(F.x + l_x);
    GL_fire_buffer_data[4] = static_cast<GLfloat>(F.y + l_y);

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

  /*******************************/
 /*        UpdateTarget         */
/*******************************/
void Rocket::updateAngleAndPower(const std::int8_t f_angle, const std::int8_t f_power)
{
    angle = std::min(static_cast<std::int8_t>(90), std::max(static_cast<std::int8_t>(-90), static_cast<std::int8_t>(angle + f_angle)));

    if (fuel == 0)
    {
        power = 0;
    }
    else
    {
        power = std::min(static_cast<std::int8_t>(4), std::max(static_cast<std::int8_t>(0), static_cast<std::int8_t>(power + f_power)));
    }
}

  /*******************************/
 /*      Reset AccSpeedPos      */
/*******************************/
void Rocket::updateAccSpeedPos()
{
    /* Formulas:
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
    pX = x;
    pY = y;

    const double angle_rad{ -angle * _PI / 180. };

    x += 0.5 * ax + vx;
    y += 0.5 * ay + vy;
    y = std::max(0., y);

    vx += ax;
    vy += ay;

    ax = power * sin(angle_rad);
    ay = power * cos(angle_rad) + _g;
}

  /*******************************/
 /*            Debug            */
/*******************************/
void Rocket::debug(const double f_timeSec) const
{
    if (f_timeSec >= 0)
    {
        std::cout << "Elapsed time: " << f_timeSec << "s" << std::endl;
    }
    std::cout << "  X=" << x << ", Y=" << y << std::endl;
    std::cout << "  HSpeed=" << vx << "m/s, VSpeed=" << vy << "m/s" << std::endl;
    std::cout << "  Next:" << std::endl;
    std::cout << "    Fuel= " << fuel << "L" << std::endl;
    std::cout << "    Angle=" << int(angle) << "deg, Power=" << int(power) << "m/s2" << std::endl;
    std::cout << std::endl;
}

  /*******************************/
 /*           Success           */
/*******************************/
bool Rocket::isParamSuccess() const
{
    return abs(angle) <= 15 && abs(vy) <= 40 && abs(vx) <= 20;
}
