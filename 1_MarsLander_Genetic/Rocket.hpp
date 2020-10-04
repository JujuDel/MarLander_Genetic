#ifndef ROCKET_HPP
#define ROCKET_HPP

#include <GL/glew.h>

extern const float _w;
extern const float _h;

class Rocket
{
public:
    Rocket(const double f_x = 0., const double f_y = 0., const double f_vx = 0., const double f_vy = 0., const std::int8_t f_angle = 0, const std::int8_t f_power = 0, const int f_fuel = 0);

    void init(const Rocket& f_rocket);

    void updateRocket(const std::int8_t f_angle, const std::int8_t f_power);

    void debug(const double f_timeSec = -1.) const;

    bool isParamSuccess() const;

    /* Params */
    double pX;
    double pY;
    double x;
    double y;
    double vx;
    double vy;
    double ax;
    double ay;
    std::int8_t angle;
    std::int8_t power;
    int fuel;
    bool isAlive;
    int floor_id_crash;
};

void updateBuffers(const Rocket& f_rocket, const double f_elapsed, GLfloat* GL_rocket_buffer_data, GLfloat* GL_fire_buffer_data);

#endif
