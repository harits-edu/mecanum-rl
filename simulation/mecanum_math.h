#ifndef MECANUM_MATH_H
#define MECANUM_MATH_H

#include <math.h>

// Parameter Robot
#define R_WHEEL 0.05    // Jari-jari roda (m)
#define LX 0.15         // Jarak pusat ke poros depan (m)
#define LY 0.15         // Jarak pusat ke poros samping (m)

typedef struct {
    float x, y, theta;
    float target_x, target_y;
} RobotState;

// Inverse Kinematics: vx, vy, omega -> Kecepatan 4 Roda
void calculate_wheels(float vx, float vy, float omega, float *w) {
    w[0] = (vx - vy - (LX + LY) * omega) / R_WHEEL; // Front Left
    w[1] = (vx + vy + (LX + LY) * omega) / R_WHEEL; // Front Right
    w[2] = (vx + vy - (LX + LY) * omega) / R_WHEEL; // Rear Left
    w[3] = (vx - vy + (LX + LY) * omega) / R_WHEEL; // Rear Right
}

// Forward Kinematics: Update posisi berdasarkan kecepatan linear ideal
void update_position(RobotState *s, float vx, float vy, float omega, float dt) {
    float dx = (vx * cos(s->theta) - vy * sin(s->theta)) * dt;
    float dy = (vx * sin(s->theta) + vy * cos(s->theta)) * dt;
    s->x += dx;
    s->y += dy;
    s->theta += omega * dt;
}

#endif