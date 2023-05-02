#include <iostream>
#include <windows.h>
#include "GL/glut.h"
#include "Camera.h"
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"
#include "Camera.h"
#include "BmpLoader.h"


using namespace std;

string dir= "C:\\Users\\EON\\Documents\\Codes\\Graphics lab\\kanon"; //change this to your own directory

GLuint ID[]={1,2,3,4,5,6,7,8};
//init values
void Camera::Init()
{
	m_yaw = 0.0;
	m_pitch = 0.0;
	SetPos(-5, 4, 12);	//to start over the floor
}

//invoke each time in the draw function
void Camera::Refresh()
{
	// Camera parameter according to Riegl's co-ordinate system
	// x/y for flat, z for height
	m_lx = cos(m_yaw) * cos(m_pitch);
	m_ly = sin(m_pitch);
	m_lz = sin(m_yaw) * cos(m_pitch);

	m_strafe_lx = cos(m_yaw - M_PI_2);
	m_strafe_lz = sin(m_yaw - M_PI_2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_x, m_y, m_z, m_x + m_lx, m_y + m_ly, m_z + m_lz, 0.0,1.0,0.0);

	//printf("Camera: %f %f %f Direction vector: %f %f %f\n", m_x, m_y, m_z, m_lx, m_ly, m_lz);
}

//to place a position of the camera
void Camera::SetPos(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z =z;

	Refresh();
}

//to obtain the position of camera (for debugging)
void Camera::GetPos(float &x, float &y, float &z)
{
    x = m_x;
    y = m_y;
    z = m_z;
}

//to obtain the direction of camera (for debugging)
void Camera::GetDirectionVector(float &x, float &y, float &z)
{
    x = m_lx;
    y = m_ly;
    z = m_lz;
}

//move the camera position
void Camera::Move(float incr)
{
    float lx = cos(m_yaw)*cos(m_pitch);
    float ly = sin(m_pitch);
    float lz = sin(m_yaw)*cos(m_pitch);

	m_x = m_x + incr*lx;
	m_y = m_y + incr*ly;
	m_z = m_z + incr*lz;

	Refresh();
}

//"strafe" the camera position
void Camera::Strafe(float incr)
{
	m_x = m_x + incr*m_strafe_lx;
	m_z = m_z + incr*m_strafe_lz;

	Refresh();
}

void Camera::Fly(float incr)
{
	m_y = m_y + incr;

	Refresh();
}

//pitch, yaw, roll functions

void Camera::RotateYaw(float angle)
{
	m_yaw += angle;

	Refresh();
}

void Camera::RotatePitch(float angle)
{
    const float limit = 89.0 * M_PI / 180.0;

	m_pitch += angle;

    if(m_pitch < -limit)
        m_pitch = -limit;

    if(m_pitch > limit)
        m_pitch = limit;

	Refresh();
}

void Camera::SetYaw(float angle)
{
	m_yaw = angle;

	Refresh();
}

void Camera::SetPitch(float angle)
{
    m_pitch = angle;

    Refresh();
}



void LoadTexture(const char*filename,GLuint ID)
{
    //cout<<filename<<endl;
    //cout<<ID<<endl;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    BmpLoader bl(filename);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, bl.iWidth, bl.iHeight, GL_RGB, GL_UNSIGNED_BYTE, bl.textureData );
}





//a struct to manage colors
struct color3f
{
	float r, g, b;
	color3f(){ r = 0; g = 0; b = 0; }
	color3f(float r, float g, float b){ this->r = r; this->g = g; this->b = b; }
};
//an struct to manage 3d points
struct point3f
{
	float x, y, z;
	point3f(){ x = 0; y = 0; z = 0; }
	point3f(float x, float y, float z){ this->x = x; this->y = y; this->z = z; }
};

int g_iWidth = 800;
int g_iHeight = 600;
const float g_fNear = 0.01;
const float g_fFar = 100.f;
color3f g_background;
//camera variables
Camera g_camera;
bool g_key[256];	//all keys 2^8
bool g_shift_down = false;
bool g_fps_mode = false;
bool g_mouse_left_down = false;
bool g_mouse_right_down = false;
// Movement settings
// const float g_translation_speed = 0.005;
const float g_translation_speed = 1;

const float g_rotation_speed = M_PI / 180 * 0.05;

//definition of all functions of glut and camera (and the display)
//all these functions are defined below the main function
void display();
void reshape(int w, int h);
void init();
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void mousemotion(int x, int y);
void mouse(int button, int state, int x, int y);
void timer(int value);
void idle();
void menucreate();
void menufunction(int option);

//from this point to the main function are drawing object functions
//each function of drawing is inside a glPushMatrix/glPopmatrix

//draw the green flat/floor
void grid()
{
	glPushMatrix();
	glColor3ub(0,113,0);
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);
		glVertex3f(-50, 0, -50);
		glVertex3f(50, 0, -50);
		glVertex3f(50, 0, 50);
		glVertex3f(-50, 0, 50);
		glEnd();
	glPopMatrix();
}

//there are cones in brown color in both sides
void mountains()
{
	//from one side (symmetrical)
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
    glEnable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D,2);
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(13, 0, 40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(10, 10, 25, 25);
	glPopMatrix();
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(-13, 0, 40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(10, 10, 25, 25);
	glPopMatrix();
	//from one side (symmetrical - far away)
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(28, 0, 40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(8, 10, 25, 25);
	glPopMatrix();
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(-28, 0, 40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(8, 10, 25, 25);
	glPopMatrix();
	//from other side (symmetrical)
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(13, 0, -40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(10, 10, 25, 25);
	glPopMatrix();
	glPushMatrix();
		glColor3ub(76, 63, 25);
		glTranslatef(-13, 0, -40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(10, 10, 25, 25);
	glPopMatrix();
	//from other side (symmetrical - far away)
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(28, 0, -40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(8, 10, 25, 25);
	glPopMatrix();
	glPushMatrix();
		//glColor3ub(76, 63, 25);
		glTranslatef(-28, 0, -40);
		glRotatef(270, 1, 0, 0);
		glutSolidCone(8, 10, 25, 25);
	glPopMatrix();


	glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
}

//a road in middle of the scene
void road()
{
	//the road
	glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
        glEnable(GL_TEXTURE_GEN_T);
        glBindTexture(GL_TEXTURE_2D,7);
	glPushMatrix();
		//glColor3ub(15,15,15);
		glBegin(GL_QUADS);
			glVertex3f(-2, 0.005, 50);
			glVertex3f(2, 0.005, 50);
			glVertex3f(2, 0.005, -50);
			glVertex3f(-2, 0.005, -50);
		glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
	//thw white strips
	for (float z = -50; z < 50; z += 10)	//+=10 to space adequatly
	{
		glPushMatrix();
			glColor3ub(255, 255, 255);
				glBegin(GL_QUADS);
				glVertex3f(-0.3, 0.008, z);
				glVertex3f(0.3, 0.008, z);
				glVertex3f(0.3, 0.008, z+5);
				glVertex3f(-0.3, 0.008, z+5);
			glEnd();
		glPopMatrix();
	}
}

void light()
{
    GLfloat no_light[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_ambient[]  = {0.3, 0.3, 0.3, 1.0};
    GLfloat light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 0, 15.0, -5,1.0 };

    glEnable( GL_LIGHT1);
    glLightfv( GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv( GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv( GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv( GL_LIGHT1, GL_POSITION, light_position);

  /*  GLfloat spot_direction[] = { 0.0, -1.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
    glLightf( GL_LIGHT0, GL_SPOT_CUTOFF, 10.0); */
}


//drawing torus as trick, only the half is viewing. If check below the floor, there is the other half!
void tunnels()
{
	//arc 1
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
    glEnable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D,8);
	glPushMatrix();
		//glColor3ub(22, 75, 125);
		glTranslatef(0, 0, -40);	//-40
		glScalef(1, 1, 20);
		glutSolidTorus(0.5, 3.0, 30, 20);
	glPopMatrix();
	glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

	//arc 2
	glPushMatrix();
		glColor3ub(22, 75, 125);
		glTranslatef(0, 0, 40);		//+40
		glScalef(1, 1, 20);
		glutSolidTorus(0.5, 3.0, 30, 20);
	glPopMatrix();
}

//is the building formed by a cube and 6 windows
void house(color3f color, float x, float y, float z,int ID)
{
	glPushMatrix();
	glTranslatef(x, y, z);


		//the body
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
        glEnable(GL_TEXTURE_GEN_T);
        glBindTexture(GL_TEXTURE_2D,ID);
		glPushMatrix();
			//glColor3ub(color.r, color.g, color.b);
			glTranslatef(0,2.5,0);
			glScalef(1,2,1);
			glutSolidCube(3);
		glPopMatrix();
		glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
        glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_2D);
		//from top to bottom, left to right
		//window 0
		glPushMatrix();
			glColor3ub(250, 250, 250);
			glTranslatef(-1.55, 3.5, -1.25);
			glBegin(GL_QUADS);
				glNormal3f(-1, 0, 0);
				glVertex3f(0, 0, 0);
				glVertex3f(0, 0, 1);
				glVertex3f(0, 1, 1);
				glVertex3f(0, 1, 0);
			glEnd();
		glPopMatrix();
		//window 1
		glPushMatrix();
			glColor3ub(250,250,250);
			glTranslatef(-1.55, 3.5, 0.25);
				glBegin(GL_QUADS);
					glNormal3f(-1,0,0);
					glVertex3f(0, 0, 0);
					glVertex3f(0, 0, 1);
					glVertex3f(0, 1 ,1);
					glVertex3f(0, 1, 0);
			glEnd();
		glPopMatrix();
		//window 2
		glPushMatrix();
			glColor3ub(250, 250, 250);
			glTranslatef(-1.55, 2.0, -1.25);
			glBegin(GL_QUADS);
				glNormal3f(-1, 0, 0);
				glVertex3f(0, 0, 0);
				glVertex3f(0, 0, 1);
				glVertex3f(0, 1, 1);
				glVertex3f(0, 1, 0);
			glEnd();
		glPopMatrix();
		//window 3
		glPushMatrix();
			glColor3ub(250, 250, 250);
			glTranslatef(-1.55, 2.0, 0.25);
			glBegin(GL_QUADS);
				glNormal3f(-1, 0, 0);
				glVertex3f(0, 0, 0);
				glVertex3f(0, 0, 1);
				glVertex3f(0, 1, 1);
				glVertex3f(0, 1, 0);
			glEnd();
		glPopMatrix();
		//window 4
		glPushMatrix();
		glColor3ub(250, 250, 250);
		glTranslatef(-1.55, 0.5, -1.25);
		glBegin(GL_QUADS);
			glNormal3f(-1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 1);
			glVertex3f(0, 1, 1);
			glVertex3f(0, 1, 0);
		glEnd();
		glPopMatrix();
		//window 5
		glPushMatrix();
		glColor3ub(250, 250, 250);
		glTranslatef(-1.55, 0.5, 0.25);
		glBegin(GL_QUADS);
			glNormal3f(-1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 1);
			glVertex3f(0, 1, 1);
			glVertex3f(0, 1, 0);
		glEnd();
		glPopMatrix();
	glPopMatrix();
}

void car(float x, float y, float z)
{
	glPushMatrix();
	glTranslatef(x, y, z);
		//the load part
		glPushMatrix();
			glColor3ub(220, 10, 10);
			glTranslatef(1.0, 0.5, 10);
			glScalef(1, 0.4, 2.0);
			glutSolidCube(1);
		glPopMatrix();
		//cabin
		glPushMatrix();
			glColor3ub(10, 10, 220);
			glTranslatef(1.0, 0.8, 10.5);
			glScalef(0.5, 0.4, 0.7);
			glutSolidCube(1);
		glPopMatrix();
		//lights
		glPushMatrix();
			glColor3ub(220, 220, 10);
			glTranslatef(.6, 0.7, 9);
			//glScalef(0.5, 0.4, 0.7);
			glutSolidSphere(0.1, 12, 12);
		glPopMatrix();
		glPushMatrix();
			glColor3ub(220, 220, 10);
			glTranslatef(1.2, 0.7, 9);
			//glScalef(0.5, 0.4, 0.7);
			glutSolidSphere(0.1, 12, 12);
		glPopMatrix();

		glPushMatrix();
			glColor3ub(107, 107, 249);
			glTranslatef(0.5, 0.35, 9);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.4, 1, 15, 30);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(1.5, 0.35, 9);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.4, 1, 15, 30);
		glPopMatrix();
		//back wheels
		glPushMatrix();
			glTranslatef(0.5, 0.35, 11);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.4, 1, 15, 30);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(1.5, 0.35, 11);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.4, 1, 15, 30);
		glPopMatrix();
	glPopMatrix();
}

//a truck with load part, the front part, 4 wheels and front window
void truck(float x, float y, float z)
{
	glPushMatrix();
	glTranslatef(x, y, z);
		//the load part
		glPushMatrix();
			glColor3ub(100, 100, 125);
			glTranslatef(1.0, 0.75, 10);
			glScalef(1, 1, 3);
			glutSolidCube(1);
		glPopMatrix();
		//the front part
		glPushMatrix();
			glColor3ub(255, 10, 12);
			glTranslatef(1.0, 0.75, 8.25);
			glScalef(1, 0.75, 0.5);
			glutSolidCube(1.0);
		glPopMatrix();
		//window
		glPushMatrix();
			glColor3ub(200,200,200);
			glBegin(GL_QUADS);
			glVertex3f(0.6, 1.0, 7.99);
			glVertex3f(1.4, 1.0, 7.99);
			glVertex3f(1.4, 0.65, 7.99);
			glVertex3f(0.6, 0.65, 7.99);
			glEnd();
		glPopMatrix();
		//front wheels
		glPushMatrix();
			glColor3ub(20, 20, 20);
			glTranslatef(0.5, 0.35, 9);
			glRotatef(90,0,1,0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.85, 1, 15, 30);
		glPopMatrix();
		glPushMatrix();
			glColor3ub(20, 20, 20);
			glTranslatef(1.5, 0.35, 9);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.85, 1, 15, 30);
		glPopMatrix();
		//back wheels
		glPushMatrix();
			glColor3ub(20, 20, 20);
			glTranslatef(0.5, 0.35, 11);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.85, 1, 15, 30);
		glPopMatrix();
		glPushMatrix();
			glColor3ub(20, 20, 20);
			glTranslatef(1.5, 0.35, 11);
			glRotatef(90, 0, 1, 0);
			glScalef(0.2, 0.2, 0.2);
			glutSolidTorus(0.85, 1, 15, 30);
		glPopMatrix();
	glPopMatrix();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(g_iWidth, g_iHeight);
	glutCreateWindow("Program");

	glEnable(GL_TEXTURE_2D);
    for(int i=0;i<sizeof(ID)/sizeof(ID[0]);i++)
    {
        string tex= dir + "\\textures\\" + to_string(ID[i]) + ".bmp";
        LoadTexture(tex.c_str(),ID[i]);
    }
    cout<<"done"<<endl;

	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemotion);
	glutPassiveMotionFunc(mousemotion);
	glutKeyboardUpFunc(keyboardup);

	glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);

    light();

	glutTimerFunc(1, timer, 0);
	glutMainLoop();
}

//is invoke by init function and created the menus options
void menucreate()
{
	int backgroundmenu = glutCreateMenu(menufunction);
	glutAddMenuEntry("Red", 0);
	glutAddMenuEntry("Blue", 1);
	glutAddMenuEntry("Green", 2);
	glutAddMenuEntry("Yellow", 3);
	glutAddMenuEntry("Default", 4);

	int mainmenu = glutCreateMenu(menufunction);
	glutAddSubMenu("Color", backgroundmenu);

	glutAddMenuEntry("Light On", 5);
	glutAddMenuEntry("Light Off", 6);
	glutAddMenuEntry("Exit", 7);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//used when click on menu
void menufunction(int option)
{
	if (option == 0)	//red color
	{
		g_background.r = 1;
		g_background.g = 0;
		g_background.b = 0;
	}
	else if (option == 1)	//blue color
	{
		g_background.r = 0;
		g_background.g = 0;
		g_background.b = 1;
	}
	else if (option == 2)	//green color
	{
		g_background.r = 0;
		g_background.g = 1;
		g_background.b = 0;
	}
	else if (option == 3)	//orange color
	{
		g_background.r = 255;
		g_background.g = 153;
		g_background.b = 0;
	}
	else if (option == 4)	//default color (black)
	{
		g_background.r = 0;
		g_background.g = 0;
		g_background.b = 0;
	}
	else if (option == 5)	//light on
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_COLOR_MATERIAL);
	}
	else if (option == 6)	//light off
	{
		glDisable(GL_LIGHTING);
	}
	else if (option == 7)	//exit of application
		exit(0);
	glutPostRedisplay();
}

//the entry point to display objects
void display()
{
    glClearColor(0.0f, 0.1f, 0.1f,0.7);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(g_background.r, g_background.g, g_background.b, 1.0);
	glLoadIdentity();
	g_camera.Refresh();	//to update the camera position
	mountains();
	car(0, 0, 0);
	car(0, 0, 10);
	truck(-2, 0, 8);
	truck(-2, 0, -10);
	house(color3f(15, 20, 200), 9, 0, 2,4);
	house(color3f(15, 150, 150), 9, 0, 12,5);
	house(color3f(20, 150, 200), 9, 0, 6,6);
	house(color3f(150, 20, 200), 9, 0, -6,4);
	house(color3f(15, 200, 20), 9, 0, -10,5);
	house(color3f(200, 15, 20), 9, 0, 19,6);
	road();
	tunnels();
	grid();
	glutSwapBuffers();
}

//initialize some variables
void init()
{
	glEnable(GL_DEPTH_TEST);	//zbuffer
	g_background.r = 0;
	g_background.g = 0;
	g_background.b = 0;
	glClearColor(g_background.r, g_background.g, g_background.b, 1.0);
	menucreate();
}

//function to call when the window is resized
void reshape(int w, int h)
{
	g_iHeight = h;
	g_iWidth = w;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.f, (float)w / (float)h, g_fNear, g_fFar);
	glMatrixMode(GL_MODELVIEW);
}

//key press method. When ESC is pressed (exit) and when space is pressed (camera)
void keyboard(unsigned char key, int x, int y)
{
	if (key == 27)	//esc
	{
		exit(0);
	}
	if (key == ' ') {	//space
		g_fps_mode = !g_fps_mode;

		if (g_fps_mode) {
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(g_iWidth / 2, g_iHeight / 2);
		}
		else {
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
	}

	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
		g_shift_down = true;
	}
	else {
		g_shift_down = false;
	}

	g_key[key] = true;
}

//inactive keys
void keyboardup(unsigned char key, int x, int y)
{
	g_key[key] = false;
}

//call the display function on idle time
void idle()
{
	display();
}

//camera related function: timer, mouse and mouse motion
void timer(int value)
{
	if (g_fps_mode) {
		if (g_key['w'] || g_key['W']) {
			g_camera.Move(g_translation_speed);
		}
		else if (g_key['s'] || g_key['S']) {
			g_camera.Move(-g_translation_speed);
		}
		else if (g_key['a'] || g_key['A']) {
			g_camera.Strafe(g_translation_speed);
		}
		else if (g_key['d'] || g_key['D']) {
			g_camera.Strafe(-g_translation_speed);
		}
		else if (g_mouse_left_down) {
			g_camera.Fly(-g_translation_speed);
		}
		else if (g_mouse_right_down) {
			g_camera.Fly(g_translation_speed);
		}
	}

	glutTimerFunc(1, timer, 0);	//call the timer again each 1 millisecond
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			g_mouse_left_down = true;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			g_mouse_right_down = true;
		}
	}
	else if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON) {
			g_mouse_left_down = false;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			g_mouse_right_down = false;
		}
	}
}

void mousemotion(int x, int y)
{
	// This variable is hack to stop glutWarpPointer from triggering an event callback to Mouse(...)
	// This avoids it being called recursively and hanging up the event loop
	static bool just_warped = false;

	if (just_warped) {
		just_warped = false;
		return;
	}

	if (g_fps_mode) {
		int dx = x - g_iWidth / 2;
		int dy = y - g_iHeight / 2;

		if (dx) {
			g_camera.RotateYaw(g_rotation_speed*dx);
		}

		if (dy) {
			g_camera.RotatePitch(g_rotation_speed*dy);
		}

		glutWarpPointer(g_iWidth / 2, g_iHeight / 2);

		just_warped = true;
	}
}
