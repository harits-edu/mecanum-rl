#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <math.h>
#include "mecanum_math.h"

// Networking
int server_fd, client_fd = -1;
struct sockaddr_in address;

// State Robot & Obstacle
RobotState robot = {-3.5, -3.5, 0.0, 3.0, 3.0}; // Mulai dari pojok agar tidak menabrak rintangan saat start
int collision = 0;

// Definisi Area Rintangan (Kotak di Tengah)
float obs_min_x = -1.0f, obs_max_x = 1.0f;
float obs_min_y = -1.0f, obs_max_y = 1.0f;

// --- FUNGSI NETWORKING ---
void init_server() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    printf("Server Mecanum berjalan di port 8080...\n");
}

void handle_communication() {
    char buffer[1024] = {0};
    char response[1024] = {0};

    if (client_fd <= 0) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd > 0) fcntl(client_fd, F_SETFL, O_NONBLOCK);
    }

    if (client_fd > 0) {
        int valread = read(client_fd, buffer, 1024);
        if (valread > 0) {
            // 1. Proses Perintah dari Python
            if (strncmp(buffer, "reset", 5) == 0) {
                robot.x = -3.5; robot.y = -3.5; robot.theta = 0;
                collision = 0;
            } 
            else if (strncmp(buffer, "control", 7) == 0) {
                float vx, vy, omega;
                sscanf(buffer, "control %f %f %f", &vx, &vy, &omega);
                update_position(&robot, vx, vy, omega, 0.016); // 60 FPS update
            }

            // 2. Deteksi Tabrakan (AABB Collision Detection sederhana)
            // Kita beri sedikit margin (0.2) sesuai ukuran chassis robot
            if (robot.x > obs_min_x - 0.2 && robot.x < obs_max_x + 0.2 &&
                robot.y > obs_min_y - 0.2 && robot.y < obs_max_y + 0.2) {
                collision = 1;
            } else {
                collision = 0;
            }

            // 3. Kirim Balik State Ke Python (6 data: x, y, theta, tx, ty, collision)
            sprintf(response, "%.3f,%.3f,%.3f,%.3f,%.3f,%d", 
                    robot.x, robot.y, robot.theta, robot.target_x, robot.target_y, collision);
            send(client_fd, response, strlen(response), 0);
        }
    }
}

// --- FUNGSI VISUALISASI ---

void draw_grid() {
    glBegin(GL_LINES);
    glColor3f(0.15f, 0.15f, 0.15f);
    for(float i = -5; i <= 5; i += 0.5f) {
        glVertex2f(i, -5); glVertex2f(i, 5);
        glVertex2f(-5, i); glVertex2f(5, i);
    }
    glEnd();
}

void draw_obstacles() {
    // Gambar Rintangan sebagai kotak merah solid
    glColor3f(0.8f, 0.1f, 0.1f); 
    glBegin(GL_QUADS);
        glVertex2f(obs_min_x, obs_min_y);
        glVertex2f(obs_max_x, obs_min_y);
        glVertex2f(obs_max_x, obs_max_y);
        glVertex2f(obs_min_x, obs_max_y);
    glEnd();

    // Garis tepi rintangan agar lebih jelas
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(obs_min_x, obs_min_y);
        glVertex2f(obs_max_x, obs_min_y);
        glVertex2f(obs_max_x, obs_max_y);
        glVertex2f(obs_min_x, obs_max_y);
    glEnd();
}

void draw_wheel(float x_offset, float y_offset) {
    glPushMatrix();
    glTranslatef(x_offset, y_offset, 0);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
        glVertex2f(-0.08, -0.04); glVertex2f(0.08, -0.04);
        glVertex2f(0.08, 0.04);   glVertex2f(-0.08, 0.04);
    glEnd();
    glPopMatrix();
}

void draw_robot() {
    glPushMatrix();
    glTranslatef(robot.x, robot.y, 0);
    glRotatef(robot.theta * 180.0f / 3.14159f, 0, 0, 1);

    // Chassis (Warna berubah jadi kuning jika tabrakan)
    if (collision) glColor3f(1.0f, 1.0f, 0.0f); 
    else glColor3f(0.1f, 0.3f, 0.6f);

    glBegin(GL_QUADS);
        glVertex2f(-0.2, -0.15); glVertex2f(0.2, -0.15);
        glVertex2f(0.2, 0.15);   glVertex2f(-0.2, 0.15);
    glEnd();

    draw_wheel( 0.15,  0.18); draw_wheel( 0.15, -0.18);
    draw_wheel(-0.15,  0.18); draw_wheel(-0.15, -0.18);

    glPopMatrix();
}

void draw_target() {
    glPushMatrix();
    glTranslatef(robot.target_x, robot.target_y, 0);
    glColor3f(0.0f, 1.0f, 0.0f); // Target Hijau
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 360; i += 20) {
        float rad = i * 3.14159f / 180.0f;
        glVertex2f(cos(rad) * 0.2, sin(rad) * 0.2);
    }
    glEnd();
    glPopMatrix();
}

void display() {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    draw_grid();
    draw_obstacles(); // Gambar rintangan dulu agar berada di bawah robot
    draw_target();
    draw_robot();

    glutSwapBuffers();
}

void idle() {
    handle_communication();
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    init_server();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Mecanum RL Simulation with Obstacles");
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-5, 5, -5, 5, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}