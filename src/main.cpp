// CS 465
// Final Project

#include "../shared/gltools.h"	// OpenGL toolkit
#include <ctime>
#include <iostream>
using namespace std;

#define PI 3.14159265
#define FORWARD_SPEED 50.f //per second
#define ANGULAR_VELOCITY 45.f //per second
#define MOUSE_ANGULAR_SCALE 0.1f
#define PARTICLE 4
#define MAX_SPEED 5
#define MIN_SPEED 1
#define MAX_PARTICLES 100000
#define EARTH 0
#define SPIRAL 1
#define STARDES 2
#define STONE 3
#define JUPITER 4
#define MERCURY 5
#define SMOOTH 6
#define PSTARS 7
#define PARTICLE 8
#define CIRCDES 9
#define HEXDES 10
#define NUM_TEXTURES 11

struct Planet
{
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
	float radius;
	int rot;
	bool rings;
	int texIndex;
	float red;
	float green;
	float blue;
	Planet *nextPlanet;
};
struct Input
{
	float	vertical = 0; //up = +, down = -
	float	horizontal = 0; //right = +, left = -
	float	forward = 0; //forward = +, backwards = -
	float	strafe = 0; //right = +, left = -1
};
struct Camera
{
	float	x = 30;
	float	y = 0;
	float	z = 0;
	float	rotX = 10;
	float	rotY = 10;
};
struct Particle				// Create A Structure For Particle
{
	float	life;					// Particle Life
	float   fade;				    // fade delta
	float	r;						// Red Value
	float	g;						// Green Value
	float	b;						// Blue Value
	float	x;						// X Position
	float	y;						// Y Position
	float	z;						// Z Position
	float	dx;						// X Direction
	float	dy;						// Y Direction
	float	dz;						// Z Direction
};

const char *szCubeFaces[6] = { "pos_x.tga", "neg_x.tga", "pos_y.tga", "neg_y.tga", "pos_z.tga", "neg_z.tga" };
double lastTime;
int planetCount = 0;
bool warped; //flag for warping the mouse
void drawWings(int rotation);
void specialKeysDown(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void mouseMovement(int x, int y);
void updateScene(double deltaTime);
void drawOctahedron();


 // Light values and coordinates

GLfloat	 lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat  specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  specref[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat  ambientNoLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightcolor[] = { 0.0f, 1.0f, 1.0f, 1.0f }; //cyan
GLfloat	 none[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat  full[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat rot = 25, rot2 = 115, spin;
GLenum  cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,GL_TEXTURE_CUBE_MAP_NEGATIVE_X,GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,GL_TEXTURE_CUBE_MAP_POSITIVE_Z,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
GLuint textures[NUM_TEXTURES];
Particle particles[MAX_PARTICLES];	// Particle Array (Room For Particle Info)
Input input;
Camera camera;
Planet *planet;

/////////////////////MOUSE + UPDATE CAMERA////////////////////////////////////////////////////////////
void mouseMovement(int x, int y) //pos x,y
{
	if (!warped)
	{
		int mouseDiffx = x - glutGet(GLUT_WINDOW_WIDTH) / 2;
		int mouseDiffy = y - glutGet(GLUT_WINDOW_HEIGHT) / 2;
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
		camera.rotX -= ((GLfloat)mouseDiffy)*MOUSE_ANGULAR_SCALE;
		camera.rotY += ((GLfloat)mouseDiffx)*MOUSE_ANGULAR_SCALE;
		warped = true;
	}
}

void updateScene(double deltaTime) //fps
{
	warped = false;

	GLfloat horizontalMovement = 1;
	GLfloat verticalMovement = 0;

	horizontalMovement = FORWARD_SPEED;

	camera.rotX -= input.vertical * ANGULAR_VELOCITY * deltaTime; //negative due to flight-like controls
	if (camera.rotX <= -90)//if looking directly below
		camera.rotX = -90; //stop viewer from looking behind
	if (camera.rotX >= 90) //if looking directly above
		camera.rotX = 90; //stop viewer from looking behind
	camera.rotY += input.horizontal * ANGULAR_VELOCITY * deltaTime;
	horizontalMovement = cos(camera.rotX*PI / 180)*FORWARD_SPEED*deltaTime;
	verticalMovement = sin(camera.rotX*PI / 180)*FORWARD_SPEED*deltaTime;

	//moving forward or backwards
	camera.z -= cos(camera.rotY*PI / 180)*horizontalMovement * input.forward;
	camera.x += sin(camera.rotY*PI / 180)*horizontalMovement * input.forward;
	camera.y += verticalMovement * input.forward;

	//strafing left or right - do not take verticle movement into consideration
	camera.z -= cos((camera.rotY + 90)*PI / 180)*FORWARD_SPEED*deltaTime*input.strafe;
	camera.x += sin((camera.rotY + 90)*PI / 180)*FORWARD_SPEED*deltaTime*input.strafe;

	if (input.forward > 0 || input.strafe > 0 || input.horizontal > 0 || input.vertical > 0) { //rotate wings to the right if moving forwards/to right
		rot += 3;
		rot2 += 3;
	}
	else if (input.forward < 0 || input.strafe < 0 || input.horizontal < 0 || input.vertical < 0) { //rotate wings left if moving back, to left
		rot -= 3;
		rot2 -= 3;
	}

}

/////////////////////PLANETS + COLLISION////////////////////////////////////////////////////////////
void initPlanet(Planet *planet)
{
	planet->x = 0;
	planet->y = 0;
	planet->z = 0;
	planet->dx = 0;
	planet->dy = 0;
	planet->dz = 0;
	if (planet->nextPlanet)
		initPlanet(planet->nextPlanet);
}

void checkPlanetCubeCollision(Planet *planet, float radiusOfCube, float deltaTime)
{
	if (planet->x - planet->radius < -radiusOfCube)
	{
		planet->x -= (planet->x - planet->radius + radiusOfCube);
		planet->dx = -planet->dx;
	}
	else if (planet->x + planet->radius > radiusOfCube)
	{
		planet->x -= (planet->x + planet->radius - radiusOfCube);
		planet->dx = -planet->dx;
	}
	if (planet->y - planet->radius < -radiusOfCube)
	{
		planet->y -= (planet->y - planet->radius + radiusOfCube);
		planet->dy = -planet->dy;
	}
	else if (planet->y > radiusOfCube)
	{
		planet->y -= (planet->y + planet->radius - radiusOfCube);
		planet->dy = -planet->dy;
	}
	if (planet->z - planet->radius < -radiusOfCube)
	{
		planet->z -= (planet->z - planet->radius + radiusOfCube);
		planet->dz = -planet->dz;
	}
	else if (planet->z > radiusOfCube)
	{
		planet->z -= (planet->z + planet->radius - radiusOfCube);
		planet->dz = -planet->dz;
	}
}

void checkPlanetsCollision(Planet *planet, float deltaTime)
{
	Planet *first = planet;
	Planet *second = planet->nextPlanet;
	Planet *temp = new Planet();
	float distance = 0;
	while (first != 0)
	{
		while (second != 0)
		{
			temp->dx =first->x - second->x;
			temp->dy = first->y - second->y;
			temp->dz = first->z - second->z;
			distance = sqrtf(pow(temp->dx, 2) + pow(temp->dy, 2) + pow(temp->dz, 2));

			if (distance <(first->radius + second->radius))
			{
				temp->dx = second->dx;
				temp->dy = second->dy;
				temp->dz = second->dz;

				second->dx = first->dx;
				second->dy = first->dy;
				second->dz = first->dz;

				first->dx = temp->dx;
				first->dy = temp->dy;
				first->dz = temp->dz;

				second->x += second->dx * deltaTime;
				second->y += second->dy * deltaTime;
				second->z += second->dz * deltaTime;

				first->x += first->dx * deltaTime;
				first->y += first->dy * deltaTime;
				first->z += first->dz * deltaTime;
			}
			second = second->nextPlanet;
		}
		first = first->nextPlanet;
		if (first != 0)
			second = first->nextPlanet;
	}
}
	
void updatePlanet(Planet *planet, float deltaTime)
{

	planet->x += planet->dx * deltaTime;
	planet->y += planet->dy * deltaTime;
	planet->z += planet->dz * deltaTime;
	
	checkPlanetCubeCollision(planet, 100, deltaTime);

	if (planet->nextPlanet)
		updatePlanet(planet->nextPlanet, deltaTime);
	checkPlanetsCollision(planet, deltaTime);
}

void drawPlanet(Planet *planet)
{
	glPushMatrix();
		glTranslatef(planet->x, planet->y, planet->z);
		glRotatef(spin/planet->rot, 1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, textures[planet->texIndex]);
		if (planet->rings) {
			glPushMatrix();
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_CULL_FACE); //prevent weird distortion
			glColor3f(.5, .5, .5);
			gluDisk(gluNewQuadric(), planet->radius + 2, planet->radius + 4, 30, 1); //(quad, inner, outer, slices, loops)
			glRotatef(-180, 1, 0, 0); //backside of disk
			gluDisk(gluNewQuadric(), planet->radius + 2, planet->radius + 4, 30, 1);
			glDisable(GL_CULL_FACE);
			glEnable(GL_TEXTURE_2D);
			glPopMatrix();
		}
		glColor3f(planet->red, planet->green, planet->blue);
		gltDrawSphere(planet->radius,30,30);
	glPopMatrix();
	if (planet->nextPlanet !=0)
		drawPlanet(planet->nextPlanet);
}

void spawnPlanet(Planet *planet, float x, float y, float z, float dx, float dy, float dz,int r, float red, float green, float blue, int texture,bool rings,int rot)
{
	Planet *temp = new Planet();
	temp->nextPlanet = 0;
	initPlanet(temp);
	temp->dx = dx;
	temp->rings = rings;
	temp->dy = dy;
	temp->dz = dz;
	temp->x = x;
	temp->y = y;
	temp->z = z;
	temp->red = red;
	temp->green = green;
	temp->blue = blue;
	temp->texIndex = texture;
	temp->radius = r;
	temp->rot = rot;
	Planet *lastLocation = planet;
	while (planet != 0)
	{
		lastLocation = planet;
		planet = planet->nextPlanet;
	}
	lastLocation->nextPlanet = temp; 
}

/////////////////////PARTICLE EFFECTS////////////////////////////////////////////////////////////
float calculateMagnitude(Particle particle)
{
	return sqrtf(pow(particle.dx, 2) + pow(particle.dy, 2) + pow(particle.dz, 2));
}

float randomFloat(float min, float max)
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}
void normalizeParticle(int i)
{
	float magnitude = calculateMagnitude(particles[i]);
	particles[i].dx /= magnitude;
	particles[i].dy /= magnitude;
	particles[i].dz /= magnitude;
}

void initParticle(int i)
{
	particles[i].r = 1;
	particles[i].g = 0;
	particles[i].b = 1;

	particles[i].x = 0;
	particles[i].y =0;
	particles[i].z = 0;

	particles[i].dx = randomFloat(-1, 1);
	particles[i].dy =1;
	particles[i].dz = randomFloat(-1, 1);

	normalizeParticle(i);

	float speed = randomFloat(MIN_SPEED, MAX_SPEED);
	particles[i].dx *= speed;
	particles[i].dy *= speed;
	particles[i].dz *= speed;

	particles[i].life = randomFloat(1, 5);

	particles[i].fade = randomFloat(.1, 1.1);
}

void initParticles()
{
	srand(time(NULL));
	for (int i = 0; i < MAX_PARTICLES; i++)
		initParticle(i);
}

void updateParticle(int i, float deltaTime)
{
	if (particles[i].life < 0)
		initParticle(i);

	if (particles[i].life <= 1)
	{
		particles[i].x += particles[i].dx * deltaTime;
		particles[i].y += particles[i].dy * deltaTime;
		particles[i].z += particles[i].dz * deltaTime;
		particles[i].r -= 3 * particles[i].fade * deltaTime;
		particles[i].b += 1 * particles[i].fade * deltaTime; 
		particles[i].g += 1 * particles[i].fade * deltaTime;
	}
	particles[i].life -= particles[i].fade * deltaTime;
}

void drawParticles()
{
	float deltaTime = 1.f / 60.f;
	glDisable(GL_LIGHTING);

	glDepthMask(GL_FALSE); //don't detect depth between the particles, just between each particle and the rest of the scene.
	glEnable(GL_BLEND);								
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					

	glBindTexture(GL_TEXTURE_2D, textures[PARTICLE]);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE); //sets coordinates from 0 to 1 in each direction for the quad

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_TEXTURE_2D);
	glPointSize(30.0f); //primitive attribute, so must be set outside of glBegin/glEnd
	glBegin(GL_POINTS);
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		updateParticle(i, deltaTime);
		// Draw The Particle Using Our RGB Values, Fade The Particle Based On Its Life
		glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].life);
		if (particles[i].life <= 1) //only draw if it's alive
			glVertex3f(particles[i].x, particles[i].y, particles[i].z); // Top Righ
	}
	glEnd();
	glDisable(GL_BLEND);// Enable Blending
	glDisable(GL_POINT_SPRITE);
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

/////////////////////SPACE SHIP////////////////////////////////////////////////////////////
void drawWings(int rotation) { //ship wing space
	glPushMatrix();
		glColor3f(.3, 0, 1);
		glRotatef(rotation, 1, 0, 0);
		glColor3f(.3, 0, 1);
		glTranslatef(-10, 0, 0);
		gluPartialDisk(gluNewQuadric(), 5, 20, 5, 5, 0, 180);
		glRotatef(-180, 1, 0, 0);
		gluPartialDisk(gluNewQuadric(), 5, 20, 5, 5, 0, 180);
	glPopMatrix();
}

void drawShip(void) {
	glPushMatrix(); //ship space
		glMaterialfv(GL_FRONT, GL_SPECULAR, lightcolor);
		glMateriali(GL_FRONT, GL_SHININESS, 20);
		if ((input.horizontal > 0 && input.vertical > 0) || (input.strafe > 0 && input.forward > 0)) //ship angle when using keys
			glRotatef(-20, 1, 1, 0);
		else if ((input.horizontal < 0 && input.vertical > 0) || (input.strafe < 0 && input.forward > 0)) {
			glRotatef(-20, 1, 0, 0);
			glRotatef(20, 0, 1, 0);
		}
		else if (input.strafe > 0 && input.forward < 0)
			glRotatef(20, 0, 1, 0);
		else if (input.strafe < 0 && input.forward < 0)
			glRotatef(-20, 0, 1, 0);
		else if (input.horizontal > 0 || input.strafe > 0)
			glRotatef(-20, 0, 1, 0);
		else if (input.horizontal < 0 || input.strafe < 0)
			glRotatef(20, 0, 1, 0);
		else if (input.vertical > 0 || input.forward > 0)
			glRotatef(-20, 1, 0, 0);
		else if (input.vertical < 0)
			glRotatef(20, 1, 0, 0);

		glRotatef(90, 0, 1, 0);
		glRotatef(30, 0, 0, 1);
		glScalef(.5, .5, .5);
		drawWings(rot); //wings 1 and 2
		drawWings(rot2);
		glPushMatrix(); //top of ship cylinder/cone space
			glColor3f(.3, 0, 1);
			glTranslatef(18, 0, 0);
			glRotatef(90, 0, 1, 0);
			gluCylinder(gluNewQuadric(), 5.0f, .1f, 15, 32, 3);
		glPopMatrix();

		glPushMatrix(); //ship body cylinder space
			glColor3f(.5, 0, 1);
			glTranslatef(18, 0, 0);
			glRotatef(90, 0, 1, 0);
			glRotatef(180, 1, 0, 0);
			gluCylinder(gluNewQuadric(), 5.0f, 10, 30, 30, 30);
		glPopMatrix();

		glPushMatrix();//ship back space
			glColor3f(.3, 0, 1);
			glRotatef(-90, 0, 1, 0);
			glTranslatef(0, 0, 11);
			gluDisk(gluNewQuadric(), 3, 10, 30, 1);
			glTranslatef(0, 0, -3);
			glColor3f(1, 0, .1);
			gluDisk(gluNewQuadric(), 0, 7, 30, 1);
		glPopMatrix();

		if (input.forward != 0 || input.strafe != 0 || input.horizontal != 0 || input.vertical != 0) { //make flamey stuff appear when moving
			glDisable(GL_LIGHTING);
			glPushMatrix(); 
				glTranslatef(-12, 0, 0);
				glRotatef(90, 0, 0, 1);
				glRotatef(90, 0, 1, 0);
				glScalef(.5, .5, .5);
				drawParticles();
			glPopMatrix();
			glEnable(GL_LIGHTING);
		}
	glPopMatrix();
}

/////////////////////SHAPES////////////////////////////////////////////////////////////
void drawTP(int xLen, int yLen, int zLen) { //triangular prism
	glPushMatrix();
		glDisable(GL_CULL_FACE);
		glBegin(GL_QUADS);

			glNormal3f(1, 0, 0);
			glTexCoord2f(1.0f, 1.0f);
			glVertex4d(0.0, 0.0, 0.0, 1.0);   
			glTexCoord2f(0.0f, 1.0f);
			glVertex4d(0.0, yLen, 0.0, 1.0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex4d(xLen, yLen, 0.0, 1.0);
			glTexCoord2f(1.0f, 0.0f);
			glVertex4d(xLen, 0.0, 0.0, 1.0);

			glNormal3f(0, 0, -1);
			glTexCoord2f(1.0f, 1.0f);
			glVertex4d(0.0, 0.0, 0.0, 1.0);
			glTexCoord2f(0.0f, 1.0f);
			glVertex4d(0.0, 0.0, zLen, 1.0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex4d(0.0, yLen, zLen, 1.0);
			glTexCoord2f(1.0f, 0.0f);
			glVertex4d(0.0, yLen, 0.0, 1.0);

			glTexCoord2f(1.0f, 1.0f);
			glVertex4d(0.0, 0.0, zLen, 1.0);
			glTexCoord2f(0.0f, 1.0f);
			glVertex4d(xLen, 0.0, 0.0, 1.0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex4d(xLen, yLen, 0.0, 1.0);
			glTexCoord2f(1.0f, 0.0f);
			glVertex4d(0.0, yLen, zLen, 1.0);
			glEnd();
			for (int i = 0; i <= 1; i++) {
				glBegin(GL_TRIANGLES);
					glNormal3f(0, 1, 0);
					glTexCoord2f(1.0f,1.0f);
					glVertex3f(0, 0, 0);
					glTexCoord2f(1.0f,0.0f);
					glVertex3f(xLen, 0, 0);
					glTexCoord2f(0.0f, 1.0f);
					glVertex3f(0, 0, zLen);
				glEnd();
				glTranslatef(0, yLen, 0);
			}
		glEnable(GL_CULL_FACE);
	glPopMatrix();
}

void drawCube(float xLen, float yLen, float zLen)
{
	glBegin(GL_QUADS);
		glNormal3f(0, 0, -1);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-xLen, yLen, -zLen);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xLen, yLen, -zLen);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(xLen, 0, -zLen);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-xLen, 0, -zLen);

		glNormal3f(0, 0, 1);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(xLen, yLen, zLen);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-xLen, yLen, zLen);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-xLen, 0, zLen);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(xLen, 0, zLen);

		glNormal3f(-1, 0, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-xLen, yLen, zLen);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-xLen, yLen, -zLen);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-xLen, 0, -zLen);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-xLen, 0, zLen);

		glNormal3f(1, 0, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(xLen, yLen, -zLen);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xLen, yLen, zLen);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(xLen, 0, zLen);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(xLen, 0, -zLen);

		glNormal3f(0, 1, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(xLen, yLen, -zLen);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-xLen, yLen, -zLen);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-xLen, yLen, zLen);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(xLen, yLen, zLen);

		glNormal3f(0, -1, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-xLen, 0, -zLen);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xLen, 0, -zLen);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(xLen, 0, zLen);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-xLen, 0, zLen);
	glEnd();
}

void drawOctahedron() { 
	glPushMatrix();
		glRotatef(45, 0, 1, 0);
		glDisable(GL_CULL_FACE);
		glBindTexture(GL_TEXTURE_2D, textures[STONE]);
		glDisable(GL_LIGHTING);
		glBegin(GL_TRIANGLES);
			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, 15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(10, 0, -10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(-10, 0, -10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, 15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-10, 0, 10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(10, 0, 10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, 15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-10, 0, -10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(-10, 0, 10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, 15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(10, 0, 10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(10, 0, -10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, -15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(10, 0, -10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(-10, 0, -10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, -15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-10, 0, 10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(10, 0, 10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, -15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-10, 0, -10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(-10, 0, 10);

			glTexCoord2f(1.5f, 1.0f);
			glVertex3f(0, -15, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(10, 0, 10);
			glTexCoord2f(3.0f, 0.0f);
			glVertex3f(10, 0, -10);
		glEnd();
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
	glPopMatrix();
}

void drawCircle(const GLfloat radius, const GLuint num_vertex)
{
	GLfloat vertex[4];
	GLfloat texcoord[2];

	const GLfloat delta_angle = 2.0*PI / num_vertex;

	glBegin(GL_TRIANGLE_FAN);
		//draw the vertex at the center of the circle
		texcoord[0] = 0.5;
		texcoord[1] = 0.5;
		glTexCoord2fv(texcoord);

		vertex[0] = vertex[1] = vertex[2] = 0.0;
		vertex[3] = 1.0;
		glVertex4fv(vertex);

		for (int i = 0; i < num_vertex; i++)
		{
			texcoord[0] = (cos(delta_angle*i) + 1.0)*0.5;
			texcoord[1] = (sin(delta_angle*i) + 1.0)*0.5;
			glTexCoord2fv(texcoord);

			vertex[0] = cos(delta_angle*i) * radius;
			vertex[1] = sin(delta_angle*i) * radius;
			vertex[2] = 0.0;
			vertex[3] = 1.0;
			glVertex4fv(vertex);
		}

		texcoord[0] = (1.0 + 1.0)*0.5;
		texcoord[1] = (0.0 + 1.0)*0.5;
		glTexCoord2fv(texcoord);

		vertex[0] = 1.0 * radius;
		vertex[1] = 0.0 * radius;
		vertex[2] = 0.0;
		vertex[3] = 1.0;
		glVertex4fv(vertex);
	glEnd();
}

void drawPlane(int size, int increment)
{
	glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);
		float textureQuadSize = (float)increment / ((float)size * 2.0f);
		for (int i = -size; i<size; i += increment)
		{
			for (int j = -size; j<size; j += increment)
			{
				float startSTexture = (float)(i) / ((float)size*2.f) + 0.5f;
				float startTTexture = 1 - ((float)(j + increment) / ((float)size*2.f) + 0.5f); //need to flip the T and offset by one increment
				glTexCoord2f(startSTexture + textureQuadSize, startTTexture + textureQuadSize);
				glVertex3f(i + increment, 0, j);
				glTexCoord2f(startSTexture + 0.0f, startTTexture + textureQuadSize);
				glVertex3f(i, 0, j);
				glTexCoord2f(startSTexture + 0.0f, startTTexture + 0.0f);
				glVertex3f(i, 0, j + increment);
				glTexCoord2f(startSTexture + textureQuadSize, startTTexture + 0.0f);
				glVertex3f(i + increment, 0, j + increment);
			}
		}
	glEnd();
}

/////////////////////SKYBOX////////////////////////////////////////////////////////////
void DrawSkyBox(void)
{
	GLfloat fExtent = 500.0f;
	glEnable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_DEPTH_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBegin(GL_QUADS);
		//////////////////////////////////////////////
		// Negative X
		glTexCoord3f(-1.0f, -1.0f, 1.0f);
		glVertex3f(-fExtent, -fExtent, fExtent);

		glTexCoord3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-fExtent, -fExtent, -fExtent);

		glTexCoord3f(-1.0f, 1.0f, -1.0f);
		glVertex3f(-fExtent, fExtent, -fExtent);

		glTexCoord3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-fExtent, fExtent, fExtent);
		///////////////////////////////////////////////
		//  Postive X
		glTexCoord3f(1.0f, -1.0f, -1.0f);
		glVertex3f(fExtent, -fExtent, -fExtent);

		glTexCoord3f(1.0f, -1.0f, 1.0f);
		glVertex3f(fExtent, -fExtent, fExtent);

		glTexCoord3f(1.0f, 1.0f, 1.0f);
		glVertex3f(fExtent, fExtent, fExtent);

		glTexCoord3f(1.0f, 1.0f, -1.0f);
		glVertex3f(fExtent, fExtent, -fExtent);
		////////////////////////////////////////////////
		// Negative Z 
		glTexCoord3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-fExtent, -fExtent, -fExtent);

		glTexCoord3f(1.0f, -1.0f, -1.0f);
		glVertex3f(fExtent, -fExtent, -fExtent);

		glTexCoord3f(1.0f, 1.0f, -1.0f);
		glVertex3f(fExtent, fExtent, -fExtent);

		glTexCoord3f(-1.0f, 1.0f, -1.0f);
		glVertex3f(-fExtent, fExtent, -fExtent);
		////////////////////////////////////////////////
		// Positive Z 
		glTexCoord3f(1.0f, -1.0f, 1.0f);
		glVertex3f(fExtent, -fExtent, fExtent);

		glTexCoord3f(-1.0f, -1.0f, 1.0f);
		glVertex3f(-fExtent, -fExtent, fExtent);

		glTexCoord3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-fExtent, fExtent, fExtent);

		glTexCoord3f(1.0f, 1.0f, 1.0f);
		glVertex3f(fExtent, fExtent, fExtent);
		//////////////////////////////////////////////////
		// Positive Y
		glTexCoord3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-fExtent, fExtent, fExtent);

		glTexCoord3f(-1.0f, 1.0f, -1.0f);
		glVertex3f(-fExtent, fExtent, -fExtent);

		glTexCoord3f(1.0f, 1.0f, -1.0f);
		glVertex3f(fExtent, fExtent, -fExtent);

		glTexCoord3f(1.0f, 1.0f, 1.0f);
		glVertex3f(fExtent, fExtent, fExtent);
		///////////////////////////////////////////////////
		// Negative Y
		glTexCoord3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-fExtent, -fExtent, -fExtent);

		glTexCoord3f(-1.0f, -1.0f, 1.0f);
		glVertex3f(-fExtent, -fExtent, fExtent);

		glTexCoord3f(1.0f, -1.0f, 1.0f);
		glVertex3f(fExtent, -fExtent, fExtent);

		glTexCoord3f(1.0f, -1.0f, -1.0f);
		glVertex3f(fExtent, -fExtent, -fExtent);
	glEnd();
	glDisable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_DEPTH_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

}

/////////////////////MAIN SCENE STUFF////////////////////////////////////////////////////////////
void drawBase() {
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslatef(0, 100, 0);
		glBindTexture(GL_TEXTURE_2D, textures[SMOOTH]); //stone base texture
		glScalef(2, 2, 2);
		glPushMatrix();
			for (int i = 0; i <= 1; i++) { //draw corners of base
				glPushMatrix();
				glTranslatef(56, -300, 0);
				glRotatef(90, 0, 1, 0);
				drawTP(100, 100, 60);
				glRotatef(180, 0, 1, 0);
				glTranslatef(0, 100, 0);
				drawTP(100, -100, -60);
				glPopMatrix();
				glRotatef(180, 0, 1, 0);
			}
		glPopMatrix();
		glTranslatef(0, -300, -50);
		drawCube(56, 100, 50); //draw part of center
		glTranslatef(0, 0, 100);
		drawCube(56, 100, 50); //draw second part of center
	glPopMatrix();	
}
void drawStar() { //center star
	glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);
		glDisable(GL_LIGHTING);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		gltDrawSphere(14, 30, 30);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

void drawScene() {

	drawStar();
	drawBase();

	glMaterialfv(GL_FRONT, GL_SPECULAR, lightcolor);
	glMateriali(GL_FRONT, GL_SHININESS, 128);

	glPushMatrix(); //flat hexagon on base + borders
		glEnable(GL_TEXTURE_2D);
		glColor3f(.3,.3,.3); //gray
		glTranslatef(0, -299, 0);
		glRotatef(-90, 1, 0, 0);
		glBindTexture(GL_TEXTURE_2D, textures[SPIRAL]); //spiral texture
		glRotatef(spin, 0, 0, 1); //make it all spin
		gltDrawTorus(100, 6, 6, 6); //draw hexagon border
		drawCircle(100, 6); //draw flat hexagon
		glRotatef(-spin, 0, 0, 1); //stop spinning!!1!!
		gltDrawTorus(225, 10, 6, 6); //draw upper base borders
		glTranslatef(0, 0, -200);
		gltDrawTorus(230, 10, 6, 6); //draw base of the base borders
	glPopMatrix();

	glPushMatrix(); //flat yellow star design 
		glRotatef(-spin, 0, 1, 0); //rotate backwards
		glTranslatef(0, -295, 0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, textures[STARDES]); //star design
		glDisable(GL_CULL_FACE);
		glColor4f(1.0, .811, .25, 0.7);
		drawPlane(70, 50); //textured plane
		glDisable(GL_BLEND);
	glPopMatrix();

	glPushMatrix(); //pointy part of sphere
		glColor3f(0, 1, 1);
		glTranslatef(0, -255, 0);
		glRotatef(spin, 0, 1, 0);
		drawOctahedron();
	glPopMatrix();

	glPushMatrix(); //purple sphere on base
		glDisable(GL_CULL_FACE);
		glRotatef(spin, 0, 1, 0);
		glColor4f(1.0, 1, 1, .6);
		glTranslatef(0, -295, 0);
		glBindTexture(GL_TEXTURE_2D, textures[PSTARS]); //purple starry texture
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gltDrawSphere(50, 30, 30);
	glPopMatrix();

	glPushMatrix(); //3 small rings above sphere
		glRotatef(spin, 0, 1, 0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glTranslatef(0, -225, 0); //top ring
		glRotatef(90, 1, 0, 0);
		glColor3f(0, 1, 1);
		glutWireTorus(1, 20, 10, 20); //second
		glTranslatef(0, 0, -25);
		glutWireTorus(1, 40, 10, 20); //third
		glTranslatef(0, 0, -25);
		glutWireTorus(1, 60, 10, 20);
	glPopMatrix();

	glPushMatrix(); //connects star to base sphere
		glColor3f(.8, 1, 1);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glRotatef(90, 1, 0, 0);
		gluCylinder(gluNewQuadric(), .75, .75, 250, 10, 10);
	glPopMatrix();

	glPushMatrix(); //everything inside this will rotate
		glRotatef(spin, 1, 1, 1);
		glPushMatrix(); //gyroscope ring space
			glColor3f(1, 0, 1); //magenta ring
			glutWireTorus(8, 150, 10, 20);
			glColor3f(1.0, .811, .25); //gold ring
			glRotatef(90, 0, 1, 0);
			glutWireTorus(8, 150, 10, 20);
			glColor3f(0, 1, 1); //cyan ring
			glRotatef(90, 1, 0, 0);
			glutWireTorus(8, 150, 10, 20);

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_LIGHTING);
			glBindTexture(GL_TEXTURE_2D, textures[SMOOTH]); //dark inner rings
			glColor3f(1, 1, 1);
			gltDrawTorus(150, 5, 100, 100);
			glRotatef(90, 0, 1, 0);
			gltDrawTorus(150, 5, 100, 100);
			glRotatef(90, 1, 0, 0);
			gltDrawTorus(150, 5, 100, 100);
		glPopMatrix();

		//updating + drawing planets
		double currentTime = std::clock() / (double)CLOCKS_PER_SEC;
		double timeSinceLastTime = currentTime - lastTime;
		lastTime = currentTime;
		updatePlanet(planet, timeSinceLastTime);
		drawPlanet(planet);

		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, lightcolor);
		glMateriali(GL_FRONT, GL_SHININESS, 128);
		glColor3f(0, 1, 1); 
		glDisable(GL_LIGHTING);
		glutWireSphere(15, 15, 15); //wire sphere for sun

		glColor3f(1, 0, 1);
		glPushMatrix(); //octahedrons attached to gyro rings
			glTranslatef(142, 0, 0);
			drawOctahedron();
			glTranslatef(-285, 0, 0);
			drawOctahedron();
			glTranslatef(142, 142, 0);
			glColor3f(1.0, .811, .25);
			drawOctahedron();
			glTranslatef(0, -285, 0);
			drawOctahedron();
			glTranslatef(0, 142, 142);
			glColor3f(0, 1, 1);
			drawOctahedron();
			glTranslatef(0, 0, -285);
			drawOctahedron();
		glPopMatrix();
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		//connectors from sun to octahedrons
		gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 150, 32, 3);
		glRotatef(180, 1, 0, 0);
		gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 150, 32, 3);
		glRotatef(90, 1, 0, 0);
		gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 150, 32, 3);
		glRotatef(180, 1, 0, 0);
		gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 150, 32, 3);
		glRotatef(90, 0, 1, 0);
		gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 150, 32, 3);
		glRotatef(180, 1, 0, 0);
		gluCylinder(gluNewQuadric(), 0.2f, 0.2f, 150, 32, 3);
	
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	glPopMatrix();
}

///////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	spin += .5; //rotation

	//view space
	glClearColor(0, 0, .2, .1);
	glTranslatef(0, -20, -70);
	//world space
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawSkyBox();

	glPushMatrix(); //all inside relies on cam controls
		glRotatef(-camera.rotX, 1, 0, 0); //use camera 
		glRotatef(camera.rotY, 0, 1, 0);
		glTranslatef(-camera.x, -camera.y, -camera.z);
		glTranslatef(0, 20, 70);
	
		glEnable(GL_CULL_FACE);

		//reflect objects and light source
		glPushMatrix();
			glTranslatef(0, -1000, 0); //move down
			glScalef(1, -1, 1); //invert over y axis
			glFrontFace(GL_CW);
			drawScene();	//draw reflection
			glPushMatrix(); //reflection of ship
				glRotatef(180, 0, 0, 1);
				glTranslatef(-camera.x, -camera.y + 20, camera.z - 70); //undo camera controls
				glRotatef(camera.rotY, 0, 1, 0);
				//glRotatef(-camera.rotX, 1, 0, 0); //angle is a little off with this
				glDisable(GL_TEXTURE_2D);
				drawShip();
				glEnable(GL_TEXTURE_2D);
			glPopMatrix();
			glFrontFace(GL_CCW);
		glPopMatrix();

		glPushMatrix(); //drawing plane to reflect
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindTexture(GL_TEXTURE_2D, textures[STONE]); //stone surface
			glDisable(GL_CULL_FACE);
			glTranslatef(0, -500, 0);
			glColor4f(1, 1, 1, 0.7);
			glScalef(2, 2, 2); //biggererer
			glRotatef(90, 1, 0, 0);
			drawCircle(500, 500); //main surface
			glRotatef(-90, 1, 0, 0);


			glDisable(GL_LIGHTING); //cyan design on stone
			glBindTexture(GL_TEXTURE_2D, textures[CIRCDES]);
			glColor4f(0, 1, 1, .8);
			glTranslatef(0, 2, 0);
			drawPlane(475, 50); //circular ring design

			glBindTexture(GL_TEXTURE_2D, textures[HEXDES]);
			glColor4f(0, 1, 1, .7);
			glTranslatef(0,1,0);
			drawPlane(125, 50); //hexagon design

			glBindTexture(GL_TEXTURE_2D, textures[SPIRAL]);
			glRotatef(90, 1, 0, 0);
			glColor3f(.1,.1, .1);
			gltDrawTorus(502,3,100,100); //edge/border of circlular plane
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
		glPopMatrix();

		drawScene(); //actually draw the non-reflected stuff

		//drawing particles for sun, and rotating them to cover it (in a 'V' shape alone)
		glScalef(.5, .5, .5);
		drawParticles();
		glRotatef(90, 1, 0, 0);
		drawParticles();
		glRotatef(90, 1, 0, 0);
		drawParticles();
		glRotatef(90, 1, 0, 0);
		drawParticles();
		glRotatef(90, 0, 0, 1);
		drawParticles();
		glRotatef(-180, 0, 0, 1);
		drawParticles();

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	drawShip(); //draw ship as normal
	glutSwapBuffers();
}
///////////////SET UP STUFF////////////////////////////////////////////
void TimerFunction(int value)
{
	updateScene(16 / 1000.0);
	// Redraw the scene with new coordinates
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}

///////////////////////////////////////////////////////////
// Setup the rendering context
void SetupRC(void)
{
	GLbyte *pBytes;
	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;

	// White background
	glClearColor(0.8f, 0.9f, 1.f, 1.0f);

	glShadeModel(GL_SMOOTH);
	//culling turned on (off by default)
	glEnable(GL_CULL_FACE);

	//depth test turned on (on be default)
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);

	// Setup and enable light 0
	// Supply a slight ambient light so the objects can be seen

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	// The light is composed of just a diffuse and specular components
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHT0);

	// Enable color tracking
	glEnable(GL_COLOR_MATERIAL);

	// Set Material properties to follow glColor values
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_NORMALIZE);
	glGenTextures(NUM_TEXTURES, textures);

	glBindTexture(GL_TEXTURE_2D, textures[EARTH]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("earth.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[SPIRAL]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("spiral.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[STARDES]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("design-star.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[STONE]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("stone.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[JUPITER]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("jupitermap.tga", &iWidth, &iHeight, &iComponents, &eFormat); 
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[MERCURY]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("mercurymap.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[SMOOTH]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("smooth-stone.tga", &iWidth, &iHeight, &iComponents, &eFormat); 
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[PSTARS]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("purple-stars.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[PARTICLE]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("particle.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[CIRCDES]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("design-circle.tga", &iWidth, &iHeight, &iComponents, &eFormat); //custom texture
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat,GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glBindTexture(GL_TEXTURE_2D, textures[HEXDES]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("design-hex.tga", &iWidth, &iHeight, &iComponents, &eFormat); //custom texture
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
	initParticles();

	planet = new Planet();
	planet->nextPlanet = 0;
	initPlanet(planet);

	//initialize clock
	double clocksPerSec = CLOCKS_PER_SEC;
	double currentTime = std::clock();
	lastTime = currentTime / clocksPerSec;

	// Load Cube Map images
	for (int i = 0; i < 6; i++)
	{
		// Load this texture map
		pBytes = gltLoadTGA(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);
		glTexImage2D(cube[i], 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
		free(pBytes);
	}
}

void ChangeSize(int w, int h)
{
	//GLfloat nRange = 100.0f;

	// Prevent a divide by zero
	if (h == 0)
		h = 1;

	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);

	// Reset projection matrix stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	GLfloat fAspect;
	fAspect = (GLfloat)w / (GLfloat)h;

	//frustum that is 45 degrees fovy, aspect ratio of w/h, 1 unit away from the camera, and will clip anything beyond 1000 units
	gluPerspective(45, fAspect, 0.1, 2000);

	// Reset Model view matrix stack
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

///////////////////////////////////////////////////////////
// Respond to arrow keys by moving the camera frame of reference
void specialKeysDown(int key, int x, int y)
{

	switch (key)
	{
	case GLUT_KEY_UP:
		input.vertical = 1;
		break;
	case GLUT_KEY_DOWN:
		input.vertical = -1;
		break;
	case GLUT_KEY_LEFT:
		input.horizontal = -1;
		break;
	case GLUT_KEY_RIGHT:
		input.horizontal = 1;
		break;
	default:
		break;
	}
}
///////////////////////////////////////////////////////////
// Respond when arrow keys are depressed 
void specialKeysUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		input.vertical = 0;
		break;
	case GLUT_KEY_DOWN:
		input.vertical = 0;
		break;
	case GLUT_KEY_LEFT:
		input.horizontal = 0;
		break;
	case GLUT_KEY_RIGHT:
		input.horizontal = 0;
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////
// Respond to keyboard keys associated with ASCII (pressed) WASD
void keyboardDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		input.forward = 5;
		break;
	case 's':
		input.forward = -5;
		break;
	case 'a':
		input.strafe = -5;
		break;
	case 'd':
		input.strafe = 5;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////
// Respond to keyboard keys associated with ASCII (depressed)  WASD
void keyboardUp(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'w':
		input.forward = 0;
		break;
	case 's':
		input.forward = 0;
		break;
	case 'a':
		input.strafe = 0;
		break;
	case 'd':
		input.strafe = 0;
		break;
	case ' ':
		if (planetCount <= 156) {
			spawnPlanet(planet, -80, -40, 10, 25, 50, 25, randomFloat(1,20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), MERCURY, true, -2);
			spawnPlanet(planet, 125, 50, 10, 10, 5, 10, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), JUPITER, true, 4);
			spawnPlanet(planet, -120, -20, 0, 2, 3, 4, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), STONE, true, 1);
			spawnPlanet(planet, -65, 10, 5, 10, 3, 4, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), EARTH, false, 4);
			spawnPlanet(planet, -30, -30, 0, 2, 3, 4, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), SMOOTH, false, 2);
			spawnPlanet(planet, 80, 20, 14, 12, 3, 4, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), MERCURY, true, 1);
			spawnPlanet(planet, -60, 40, 0, 15, 20, 15, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), EARTH, false, 5);
			spawnPlanet(planet, -55, -35, 0, 10, 20, 30, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), PSTARS, false, -2);
			spawnPlanet(planet, -110, 35, 0, 5, 5, 10, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), JUPITER, false, 1);
			spawnPlanet(planet, 25, 100, 50, 15, 15, 20, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), SMOOTH, false, -4);
			spawnPlanet(planet, 125, 20, 0, 1, 5, 0, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), STONE, false, 2);
			spawnPlanet(planet, -110, -30, 0, 10, 10, 5, randomFloat(1, 20), randomFloat(0,1), randomFloat(0,1), randomFloat(0,1), EARTH, false, 6);
			planetCount += 12;
			break;
		}
		//skip down and clear planets if go over limit
	case 'c':
		planetCount = 0;
		planet = new Planet();
		planet->nextPlanet = 0;
		initPlanet(planet);
	default:
		break;
	}
}
///////////////////////////////////////////////////////////
// Main program entry point
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Final Project");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutSpecialFunc(specialKeysDown);
	glutSpecialUpFunc(specialKeysUp);
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
	glutPassiveMotionFunc(mouseMovement);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutTimerFunc(33, TimerFunction, 1);
	SetupRC();
	glutMainLoop();

	return 0;
}






