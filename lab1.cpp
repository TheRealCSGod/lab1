//
//modified by:Erik Soto
//date: 01-24-18
//
//3350 Spring 2018 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git
//
//elements we will add to program...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

const int MAX_PARTICLES = 8000;
const float GRAVITY = 0.1;
//some structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};
struct Point {
    float x, y;
};
struct Circle{
    float radius;
    Vec center;
};



class Global {
    public:
	int xres, yres;
	Shape box[5];
	Particle particle[MAX_PARTICLES];
	Point point[100];
	Circle myCircle;
	Point center;
	int n;
	int npoints = 50;
	int radius = 100.0;
	Global() {
	    xres = 800;
	    yres = 600;
	    //define a box shape
	    box[0].width = 100;
	    box[0].height = 15;
	    box[0].center.x = xres/2 -260;
	    box[0].center.y = 560;
	    for(int i = 1; i <5 ; i++){
		box[i].width = box[0].width;
		box[i].height = box[0].height;
		box[i].center.x = box[i-1].center.x + 100;
		box[i].center.y = box[i-1].center.y - 80;

	    }


	    n = 0;
	    myCircle.radius = 220;
	    myCircle.center.x = xres;
	    myCircle.center.y = 0;
	    myCircle.center.z = 0;

	    center.x = 800;
	    center.y = 50;
	    radius = 170.0;
	}
}g;

class X11_wrapper {
    private:
	Display *dpy;
	Window win;
	GLXContext glc;
    public:
	~X11_wrapper() {
	    XDestroyWindow(dpy, win);
	    XCloseDisplay(dpy);
	}
	X11_wrapper() {
	    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	    int w = g.xres, h = g.yres;
	    dpy = XOpenDisplay(NULL);
	    if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	    }
	    Window root = DefaultRootWindow(dpy);
	    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	    if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	    } 
	    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	    XSetWindowAttributes swa;
	    swa.colormap = cmap;
	    swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	    set_title();
	    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	    glXMakeCurrent(dpy, win, glc);
	}
	void set_title() {
	    //Set the window title bar.
	    XMapWindow(dpy, win);
	    XStoreName(dpy, win, "3350 Lab1");
	}
	bool getXPending() {
	    //See if there are pending events.
	    return XPending(dpy);
	}
	XEvent getXNextEvent() {
	    //Get a pending event.
	    XEvent e;
	    XNextEvent(dpy, &e);
	    return e;
	}
	void swapBuffers() {
	    glXSwapBuffers(dpy, win);
	}


} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
    srand(time(NULL));
    init_opengl();
    //Main animation loop
    int done = 0;
    while (!done) {
	//Process external events.
	while (x11.getXPending()) {
	    XEvent e = x11.getXNextEvent();
	    check_mouse(&e);
	    done = check_keys(&e);
	}
	movement();
	render();
	x11.swapBuffers();
    }
    return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    // Fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(int x, int y)
{
    if (g.n >= MAX_PARTICLES)
	return;
    cout << "makeParticle() " << x << " " << y << endl;
    //position of particle
    Particle *p = &g.particle[g.n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = (float) rand() / (float)(RAND_MAX) * 1.0;
    p->velocity.x = (float) rand() / (float)(RAND_MAX) * 1.0 - 0.5;
    ++g.n;
}

void check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    if (e->type != ButtonRelease &&
	    e->type != ButtonPress &&
	    e->type != MotionNotify) {
	//This is not a mouse event that we care about.
	return;
    }
    //
    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = g.yres - e->xbutton.y;
	    for(int i = 0; i < 50; i++)
		makeParticle(e->xbutton.x, y);
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    if (e->type == MotionNotify) {
	/*The mouse moved!
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
	    savex = e->xbutton.x;
	    savey = e->xbutton.y;
	    int y = g.yres - e->xbutton.y;
	    for(int i = 0; i < 10; i++)
		makeParticle(e->xbutton.x, y);

	}
	*/
    }
}

int check_keys(XEvent *e)
{
    if (e->type != KeyPress && e->type != KeyRelease)
	return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    switch (key) {
	case XK_1:
	    //Key 1 was pressed
	    break;
	case XK_a:
	    //Key A was pressed
	    break;
	case XK_Escape:
	    //Escape key was pressed
	    return 1;
    }
    return 0;
}


void movement()
{
    if (g.n <= 0)
	return;
    for(int i = 0; i < g.n; i++){
	Particle *p = &g.particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;

	p->velocity.y -= GRAVITY;

	//check for collision with shapes...
	for(int j = 0; j < 5; j++){
	    Shape *s = &g.box[j];
	    if(p->s.center.y < s->center.y + s->height &&
		    p->s.center.y > s->center.y - s->height &&
		    p->s.center.x > s->center.x - s->width &&
		    p->s.center.x < s->center.x + s->width){
		p->velocity.y = -p->velocity.y;
		p->velocity.y *= 0.4;
		p->velocity.x += 0.005;
	    }
	    for(int i=0; i <g.npoints*2; i++){
		if(p->s.center.y == g.point[i].y &&
			p->s.center.x == g.point[i].x){
		    p->velocity.y = -p->velocity.y;
		    p->velocity.y *= 0.4;
		    p->velocity.x += 0.005;
		}
	    }
	}
	//check for off-screen
	if (p->s.center.y < 0.0) {
	    cout << "off screen" << endl;
	    g.particle[i] = g.particle[--g.n];
	}









	float dist = sqrt(pow(p->s.center.x - g.myCircle.center.x,2) + pow(p->s.center.y - g.myCircle.center.y,2));
	if(dist < g.myCircle.radius){
	    p->velocity.y = -p->velocity.y;
	    p->velocity.y *= 0.55;

	    p->velocity.x -= 0.25;
	}

    }

}



void drawCircle(){
    GLint sides = 360;
    Circle *c = &g.myCircle;
    glColor3ub(255,51,153);
    GLint vertices = sides + 2;
    GLfloat doublePi = 2.0f * 3.14159;
    GLfloat circleVerticesX[sides];
    GLfloat circleVerticesY[sides];
    GLfloat circleVerticesZ[sides];

    circleVerticesX[0] = c->center.x;
    circleVerticesY[0] = c->center.y;
    circleVerticesZ[0] = c->center.z;

    for(int i = 1; i < vertices; i++){
	circleVerticesX[i] = c->center.x + (c->radius * cos(i*doublePi/sides));
	circleVerticesY[i] = c->center.y + (c->radius * sin(i*doublePi/sides));
	circleVerticesZ[i] = c->center.z;
    }

    GLfloat allCircleVertices[vertices * 3];

    for(int i = 0; i < vertices; i++){
	allCircleVertices[i * 3] = circleVerticesX[i];
	allCircleVertices[(i * 3) + 1] = circleVerticesY[i];
	allCircleVertices[(i * 3) + 2] = circleVerticesZ[i];
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, allCircleVertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
    glDisableClientState(GL_VERTEX_ARRAY);

}




const float PI = 3.14159265;

void trianglestrip()
{
    glColor3ub(30,60,90);
    float angle =0.0;
    float inc = PI *2.0 / (float)g.npoints;
    for(int i =0; i<g.npoints*2; i++){
	g.point[i].x = cos(angle)* g.radius;
	g.point[i].y = sin(angle)* g.radius;

	i++;
	g.point[i].x = cos(angle)* (g.radius-30);
	g.point[i].y = sin(angle)* (g.radius-30);

	angle +=inc;
    }
    // draw the stuff

    int r = 10;
    int g1 = 10;
    int b = 10;
    glColor3ub(r,g1,b);
    glBegin(GL_TRIANGLE_STRIP);

    for(int i=0; i<g.npoints*2;i++)
    {

	glColor3ub(r,g1,b);
	glVertex2f((int)g.center.x-g.point[i].x , g.center.y-g.point[i].y);
	r +=60;
	g1 +=30;
	b +=90;

    }

    glVertex2f((int)g.center.x-g.point[0].x , g.center.y-g.point[1].y);
    glVertex2f((int)g.center.x-g.point[1].x , g.center.y-g.point[0].y);
    glEnd();
}




void lab8_circle(){

    glColor3ub(255,255,255);
    float x = g.radius;
    float y = 0.0;

    glBegin(GL_POINTS);
    while(x>=y){
	glVertex2f(g.center.x+x , g.center.y+y);
	glVertex2f(g.center.x+y , g.center.y+x);

	glVertex2f(g.center.x+x , g.center.y-y);
	glVertex2f(g.center.x+y , g.center.y-x);

	glVertex2f(g.center.x-x , g.center.y+y);
	glVertex2f(g.center.x-y , g.center.y+x);

	glVertex2f(g.center.x-x , g.center.y-y);
	glVertex2f(g.center.x-y , g.center.y-x);
	y += 1.0;
	// lets decrement
	float distance = sqrt(x*x + y*y);
	if (distance > g.radius)
	    x -=1.0;

    }
    glEnd();
}


void render()
{
    Rect r;
    /// clean screen
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);


    //Draw shapes...
    //draw a box
    Shape *s;
    float w, h;
    glColor3ub(90,140,90);
    for(int i = 0 ; i < 5; i++){
	s = &g.box[i];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);

	w = s->width;
	h = s->height;
	//boxes
	glBegin(GL_QUADS);
	glVertex2i(-w, -h);
	glVertex2i(-w,  h);
	glVertex2i( w,  h);
	glVertex2i( w, -h);
	glEnd();
	glPopMatrix();
    }
    s = &g.box[0];

    for(int i = 0; i < 5; i++){
	makeParticle(s->center.x+70, 600);
    }

    for(int i = 0; i < g.n; i++){
	//Random blue color
	int r = rand() % 151;	//random red
	//Draw the particle here
	glPushMatrix();

	glColor3ub(r, 160, 250);
	Vec *c = &g.particle[i].s.center;
	w =
	    h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
    }




    drawCircle();   

    //
    //Draw your 2D text here

    unsigned int c= 0;
    //
    r.bot = 555;
    r.left = 150;
    r.center =-100;
    ggprint8b(&r,16,c, "Requirements");
    //----------------Design---------------------------

    Rect t; 
    t.bot = 473;
    t.left = 150 +90;
    t.center =-100;

    ggprint8b(&t,16,c, "Design");
    //----------------Coding--------------------------------
    Rect a; 
    a.bot = 470 -75;
    a.left = 150 +100 +105;
    a.center =-100;

    ggprint8b(&a,16,c, "Coding");

    //----------------Testing--------------------------------
    Rect b; 
    b.bot = 318;
    b.left = 448;
    b.center =-100;
    ggprint8b(&b,16,c, "Testing");

    //----------------Maintenence--------------------------------
    Rect d; 
    d.bot = 232;
    d.left = 532;
    d.center =-100;
    ggprint8b(&d,16,c, "Maintenence");


}


