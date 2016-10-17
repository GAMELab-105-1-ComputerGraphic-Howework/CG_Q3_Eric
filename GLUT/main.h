#pragma once

#include "../freeglut-3.0.0/include/glut.h"
#include <stdio.h>
#include <iostream>
#include <vector>


GLfloat *make_texture(int maxs, int maxt);
void render(double t);
void menu(int selection);
void redraw();
void timer1(int value);
void key(unsigned char key, int x, int y);
int main(int argc, char *argv[]);
void reshape(int w, int h);
void drawWalls();
void drawObjs(double t);
void drawBallAndCone(double t);

const GLdouble FRUSTDIM = 100.f;
const GLdouble FRUSTNEAR = 320.f;
const GLdouble FRUSTFAR = 660.f;

enum { SPHERE = 1, CONE };

int WIDTH = 512;
int HEIGHT = 512;

double t = 0;
double percent = 1 / 1000.0;
const double radius = 40.0;
const double M_PI = 3.1415926;


const double objCenterX = 0;
const double objCenterY = -40.0;
const double objCenterZ = -540.0;

const double wallBack = -640.0;
const double wallFront = -320.0;
const double wallRight = 100.0;
const double wallLeft = -100.0;
const double wallTop = 100.0;
const double wallBottom = -100.0;


enum { NONE, FIELD };
int rendermode = NONE;
GLdouble focus = 420.;
const int TEXDIM = 256;
