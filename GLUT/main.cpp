#include "main.h"

/*
** Create a single component texture map
*/
GLfloat *make_texture(int maxs, int maxt)
{
	int s, t;
	static GLfloat *texture;

	texture = (GLfloat *)malloc(maxs * maxt * sizeof(GLfloat));
	for (t = 0; t < maxt; t++) {
		for (s = 0; s < maxs; s++) {
			texture[s + maxs * t] = ((s >> 4) & 0x1) ^ ((t >> 4) & 0x1);
		}
	}
	return texture;
}

void render(double t)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glPushMatrix();
	{
		glTranslatef(objCenterX, objCenterY, objCenterZ);
		drawBallAndCone(t);
	}
	glPopMatrix();

	glEnable(GL_STENCIL_TEST); //Enable using the stencil buffer
	glColorMask(0, 0, 0, 0); //Disable drawing colors to the screen
	glDisable(GL_DEPTH_TEST); //Disable depth testing
	glStencilFunc(GL_ALWAYS, 1, 1); //Make the stencil test always pass
									//Make pixels in the stencil buffer be set to 1 when the stencil test passes
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	//Set all of the pixels covered by the floor to be 1 in the stencil buffer
	drawWalls();

	glColorMask(1, 1, 1, 1); //Enable drawing colors to the screen
	glEnable(GL_DEPTH_TEST); //Enable depth testing
							 //Make the stencil test pass only when the pixel is 1 in the stencil buffer
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Make the stencil buffer not change

	drawObjs(t);

	glDisable(GL_STENCIL_TEST); //Disable using the stencil buffer


	glEnable(GL_BLEND);
	drawWalls();
	glDisable(GL_BLEND);


	if (glGetError()) /* to catch programming errors; should never happen */
		printf("Oops! I screwed up my OpenGL calls somewhere\n");

	// glFlush(); /* high end machines may need this */
}

void drawObjs(double t) {

	// TOP
	glPushMatrix();
	{
		glTranslatef(objCenterX, wallTop + (wallTop - objCenterY), objCenterZ);
		glScaled(1, -1, 1);
		drawBallAndCone(t);
	}
	glPopMatrix();

	// BOTTOM
	glPushMatrix();
	{
		glTranslatef(objCenterX, wallBottom + (wallBottom - objCenterY), objCenterZ);
		glScaled(1, -1, 1);
		drawBallAndCone(t);
	}
	glPopMatrix();

	// LEFT
	glPushMatrix();
	{
		glTranslatef(wallLeft + (wallLeft - objCenterX), objCenterY, objCenterZ);
		glScaled(-1, 1, 1);
		drawBallAndCone(t);
	}
	glPopMatrix();

	// RIGHT
	glPushMatrix();
	{
		glTranslatef(wallRight + (wallRight - objCenterX), objCenterY, objCenterZ);
		glScaled(-1, 1, 1);
		drawBallAndCone(t);
	}
	glPopMatrix();

	// BACK
	glPushMatrix();
	{
		glTranslatef(objCenterX, objCenterY, wallBack + (wallBack - objCenterZ));
		glScaled(1, 1, -1);
		drawBallAndCone(t);
	}
	glPopMatrix();
}

void drawBallAndCone(double t) {

	double ballX = cos(t * 10 * M_PI) * radius;
	double ballY = sin(t * 2 * M_PI) * 50 + 50;
	double ballZ = sin(t * 10 * M_PI) * radius;

	glPushMatrix();
	{
		glTranslatef(ballX, ballY, ballZ);
		glCallList(SPHERE);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0, 0, 0);
		glCallList(CONE);
	}
	glPopMatrix();
}

void menu(int selection)
{
	rendermode = selection;
	glutPostRedisplay();
}

/* Called when window needs to be redrawn */
void redraw()
{
	glPushMatrix();
	switch (rendermode) {
	case NONE:
		render(t);
		break;
	case FIELD:
		int max = 16;

		glClear(GL_ACCUM_BUFFER_BIT);

		for (int i = 0; i < max; i++) {
			render(t - i * percent);
			glAccum(GL_ACCUM, 1.f / max);
		}
		glAccum(GL_RETURN, 1.f);

		break;
	}

	glPopMatrix();
	glutSwapBuffers();
}

void timer1(int value) {
	t += percent;
	if (t >= 1)
		t = 0;

	glutPostRedisplay();
	glutTimerFunc(10, timer1, 1);
}

void key(unsigned char key, int x, int y)
{
	if (key == '\033')
		exit(0);
}

/* Parse arguments, and set up interface between OpenGL and window system */
int main(int argc, char *argv[])
{
	GLfloat *tex;
	static GLfloat lightpos[] = { 50.f, 50.f, -320.f, 1.f };
	static GLfloat sphere_mat[] = { 1.f, .5f, 0.f, 1.0f };
	static GLfloat cone_mat[] = { 0.f, .5f, 1.f, 1.f };
	GLUquadricObj *sphere, *cone, *base;

	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM | GLUT_SINGLE | GLUT_STENCIL);
	(void)glutCreateWindow("motion blur");
	glutDisplayFunc(redraw);
	glutKeyboardFunc(key);
	glutTimerFunc(10, timer1, 1);
	glutReshapeFunc(reshape);

	glutCreateMenu(menu);
	glutAddMenuEntry("Normal", NONE);
	glutAddMenuEntry("Motion Blur", FIELD);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* turn on features */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* place light 0 in the right place */
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	/* remove back faces to speed things up */
	glCullFace(GL_BACK);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glNewList(SPHERE, GL_COMPILE);
	/* make display lists for sphere and cone; for efficiency */
	sphere = gluNewQuadric();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sphere_mat);
	gluSphere(sphere, 20.f, 20, 20);
	gluDeleteQuadric(sphere);
	glEndList();

	glNewList(CONE, GL_COMPILE);
	cone = gluNewQuadric();
	base = gluNewQuadric();
	glRotatef(-90.f, 1.f, 0.f, 0.f);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cone_mat);
	gluDisk(base, 0., 20., 20, 1);
	gluCylinder(cone, 20., 0., 60., 20, 20);
	gluDeleteQuadric(cone);
	gluDeleteQuadric(base);
	glEndList();

	/* load pattern for current 2d texture */
	tex = make_texture(TEXDIM, TEXDIM);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, TEXDIM, TEXDIM, 0, GL_RED, GL_FLOAT, tex);
	free(tex);

	glutMainLoop();
}

void drawWalls() {
	/* material properties for objects in scene */
	static GLfloat wall_mat[] = { 1.0f, 1.0f, 1.0f, 0.5f };

	/*
	** Note: wall verticies are ordered so they are all front facing
	** this lets me do back face culling to speed things up.
	*/
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wall_mat);

	/* floor */
	/* make the floor textured */
	glEnable(GL_TEXTURE_2D);

	/*
	** Since we want to turn texturing on for floor only, we have to
	** make floor a separate glBegin()/glEnd() sequence. You can't
	** turn texturing on and off between begin and end calls
	*/
	glBegin(GL_QUADS);
	glNormal3f(0.f, 1.f, 0.f);
	glTexCoord2i(0, 0);
	glVertex3f(-100.f, -100.f, -320.f);
	glTexCoord2i(1, 0);
	glVertex3f(100.f, -100.f, -320.f);
	glTexCoord2i(1, 1);
	glVertex3f(100.f, -100.f, -640.f);
	glTexCoord2i(0, 1);
	glVertex3f(-100.f, -100.f, -640.f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	/* walls */

	glBegin(GL_QUADS);
	/* left wall */
	glNormal3f(1.f, 0.f, 0.f);
	glVertex3f(-100.f, -100.f, -320.f);
	glVertex3f(-100.f, -100.f, -640.f);
	glVertex3f(-100.f, 100.f, -640.f);
	glVertex3f(-100.f, 100.f, -320.f);

	/* right wall */
	glNormal3f(-1.f, 0.f, 0.f);
	glVertex3f(100.f, -100.f, -320.f);
	glVertex3f(100.f, 100.f, -320.f);
	glVertex3f(100.f, 100.f, -640.f);
	glVertex3f(100.f, -100.f, -640.f);

	/* ceiling */
	glNormal3f(0.f, -1.f, 0.f);
	glVertex3f(-100.f, 100.f, -320.f);
	glVertex3f(-100.f, 100.f, -640.f);
	glVertex3f(100.f, 100.f, -640.f);
	glVertex3f(100.f, 100.f, -320.f);

	/* back wall */
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, -100.f, -640.f);
	glVertex3f(100.f, -100.f, -640.f);
	glVertex3f(100.f, 100.f, -640.f);
	glVertex3f(-100.f, 100.f, -640.f);
	glEnd();
}

void reshape(int w, int h) {
	WIDTH = w;
	HEIGHT = h;

	/* draw a perspective scene */

	glViewport(0, 0, w, h);

	GLfloat aspectRatio = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, aspectRatio, 0.1f, 1000.0f);

	//glFrustum(-FRUSTDIM, FRUSTDIM, -FRUSTDIM, FRUSTDIM, FRUSTNEAR, FRUSTFAR);
	glMatrixMode(GL_MODELVIEW);
}