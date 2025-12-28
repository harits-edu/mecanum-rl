CC = gcc
CFLAGS = -Wall -O2
# Library untuk OpenGL dan Matematika
LIBS = -lglut -lGLU -lGL -lm

all: mecanum_sim

mecanum_sim: mecanum_sim.c mecanum_math.h
	$(CC) $(CFLAGS) mecanum_sim.c -o mecanum_sim.exe $(LIBS)

clean:
	rm -f *.exe *.o