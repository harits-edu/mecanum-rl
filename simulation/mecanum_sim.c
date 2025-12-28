#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "mecanum_math.h"

// Networking
int server_fd, client_fd;
struct sockaddr_in address;
RobotState robot = {0.0, 0.0, 0.0, 3.0, 3.0};

void init_server() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Set non-blocking agar simulasi OpenGL tetap jalan tanpa menunggu Python
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

    // Coba terima koneksi baru jika belum ada client
    if (client_fd <= 0) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd > 0) fcntl(client_fd, F_SETFL, O_NONBLOCK);
    }

    if (client_fd > 0) {
        int valread = read(client_fd, buffer, 1024);
        if (valread > 0) {
            if (strncmp(buffer, "reset", 5) == 0) {
                robot.x = 0; robot.y = 0; robot.theta = 0;
            } 
            else if (strncmp(buffer, "control", 7) == 0) {
                float vx, vy, omega;
                sscanf(buffer, "control %f %f %f", &vx, &vy, &omega);
                update_position(&robot, vx, vy, omega, 0.016); // 0.016s = 60fps
            }

            // Kirim balik state: x,y,theta,tx,ty
            sprintf(response, "%.3f,%.3f,%.3f,%.3f,%.3f", 
                    robot.x, robot.y, robot.theta, robot.target_x, robot.target_y);
            send(client_fd, response, strlen(response), 0);
        }
    }
}

// --- BAGIAN VISUALISASI OPENGL SEDERHANA ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glTranslatef(robot.x, robot.y, 0);
    glRotatef(robot.theta * 180/3.14, 0, 0, 1);
    
    // Gambar Body Robot (Kotak)
    glColor3f(0.0, 0.8, 0.0);
    glBegin(GL_QUADS);
        glVertex2f(-0.2, -0.2); glVertex2f(0.2, -0.2);
        glVertex2f(0.2, 0.2);   glVertex2f(-0.2, 0.2);
    glEnd();
    glPopMatrix();

    // Gambar Target
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex2f(robot.target_x-0.1, robot.target_y); glVertex2f(robot.target_x+0.1, robot.target_y);
        glVertex2f(robot.target_x, robot.target_y-0.1); glVertex2f(robot.target_x, robot.target_y+0.1);
    glEnd();

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
    glutInitWindowSize(600, 600);
    glutCreateWindow("Mecanum RL Simulation");
    glOrtho(-5, 5, -5, 5, -1, 1);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}