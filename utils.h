/***********************************************************************
 *  	COSC1187 - Interactive 3D Graphics and Animation
 *  		       Assignment 2
 *    Full Name        : Alyssa Biasi
 *    Student Number   : 3328976
 ***********************************************************************/

#ifdef WIN32
#   include <windows.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <GLUT/glut.h>
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glut.h>
#endif

#if WIN32
#define bool int
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "obj.h"
#include "texture.h"
#include "png_loader.h"

#ifndef min
#define min(a, b) (a <= b ? a : b)
#endif

#ifndef max
#define max(a, b) (a >= b ? a : b)
#endif

#ifndef clamp
#define clamp(x, a, b) min(max(x, a), b)
#endif

#define myCrossPlatformMin(a, b) ((a)<(b)?(a):(b))
#define sign(val) ((val)>0?1:-1)
#define GRID_VERTEX(i, j) ((i)*cols+(j))

#define PI acos(-1)
#define CAM_DISTANCE 20.0f
#define PROJ_VEL_HORIZONTAL 10.0f
#define PROJ_INIT_VEL_VERTICAL 10.0f
#define MAX_PROJ 30
#define GRID_SIZE 200



/****************************************************
 Vector struct and functions
 ****************************************************/
/* Struct for a vector with 2 components */
typedef struct Vec2f {
	float x, y;
} Vec2f;

/* Struct for a vector with 3 components */
typedef struct Vec3f {
	float x, y, z;
} Vec3f;

/* Struct for a vector with 4 components */
typedef struct Vec4f{
	float x, y, z, w;
} Vec4f;

/* Vec3f and Vec4f constructors. */
Vec3f newVec3f(float x, float y, float z);
Vec4f newVec4f(float x, float y, float z, float w);

/* Draws 3D axes. */
void drawAxes(Vec3f pos, Vec3f length);



/****************************************************
 Controls functions
 ****************************************************/
/* Control struct */
typedef struct Controls {
	bool wireframe;
	bool normals;
	bool axes;
} Controls;

/* Initialises control states */
void initControls(Controls *controls);



/****************************************************
 Keys struct and functions
 ****************************************************/
/* Struct for movement keys */
typedef struct Keys {
	bool up;
	bool down;
	bool right;
	bool left;
} Keys;

void initKeys(Keys *keys);
void updateKeys(int key, bool state);



/****************************************************
 Camera struct and functions
 ****************************************************/
/* Struct for the camera */
typedef struct Camera {
	float fieldOfView;  /* Degree y-direction */
	float clipNear;
	float clipFar;
	
	bool rotating;
	bool zooming;
	
	float zoom;
	float sensitivity;
	
	Vec3f pos;
	Vec3f rot;
} Camera;

/* Initialiser for the camera */
void initCamera(Camera *camera, Vec3f pos, Vec3f rot);
/* Changes viewport for split screen. */
void setViewport();



/****************************************************
 Ship struct and functions
 ****************************************************/
/* Struct for the ship */
typedef struct Ship {
	Camera cam;
    Vec3f pos;
	Vec3f rot;
	Vec3f forward;
	Vec3f side;
	struct _OBJMesh *mesh;		/* The boat's mesh */
    float pitch;	/* Angle for rotate up & down in degrees */
    float roll;		/* Angle for rotate side to side in degrees */
    float heading;	/* Degrees */
    float fVel;		/* Forward speed */
    float tVel;		/* Turning speed */
	float maxSpeed;
    float acc;			/* Forward acceleration of the boat (constant) */
	float tAcc; /* Turning acceleration (constant) */
	float displacement;
	float health;
	int player;
} Ship;

/* Initialises ship */
void initShip(Ship *ship, const char *meshFile, Camera cam, int num);
/* Draws the ship */
void drawShip(Ship *ship, Controls *controls);
/* Positions cameras relative to ships */
void setupCamera(Ship *ship1, Ship *ship2);
/* Calculates the distance between the two ships */
float calcDistance();



/****************************************************
 Water grid structs and functions
 ****************************************************/
/* Struct for the water */
typedef struct WaterGrid {
	int rows;		/* No. of vertices per row (tessellation) */
	int cols;		/* No. of vertices per col (tessellation) */
	float size;		/* Size of the grid in GL coords */
	int nVertices;
	int nIndices;
	
	Vec3f *vertices;
	Vec3f *normals;
	int *indices;
	
	GLuint tex;
} WaterGrid;


/* Struct for wave sine function */
typedef struct SineFunc {
	float A;	/* Amplitude */
	float k;	/* Wave period (k*2PI) */
	float w;	/* Angular frequency */
} SineFunc;

/* Initialises 2D water grid */
bool initGrid(WaterGrid *grid, int rows, int cols, 
						float size, const char *texFile);
/* Deletes memory allocated by initGrid */
void resetGrid(WaterGrid *grid);
/* Draws the water grid */
void drawWater(WaterGrid *grid);
/* Applies wave effect */
void updateGrid(WaterGrid *grid, float dt);
/* Draw normals */
void drawNormals(WaterGrid *grid, float size);
/* Returns a Vec4f: Vec3f of normals + w = height at (x,z) */
Vec4f calcSine(float x, float z);
/* Calculates y value at a given position */
float calcHeight(SineFunc *func, float x, float t);
/* Returns Vec4f = normal & heigh at (x,z) */
Vec4f calcSineValue(float x, float z);



/****************************************************
 Light struct and functions
 ****************************************************/
/* Struct for light info. */
typedef struct Light {
	Vec4f pos;
	Vec4f ambient;
	Vec4f diffuse;
	Vec4f specular;
	float shininess;
} Light;

/* Initialises light struct */
void initLight(Light *light, Vec4f pos, Vec4f ambient, 
				Vec4f diffuse, Vec4f specular, float shininess);
/* Applies the values stored in light to GL_LIGHT0 */
void setupLight(Light *light);



/****************************************************
 Projectile struct and functions
 ****************************************************/
/* Struct for cannon ball projectile */
typedef struct Projectile {
	Vec3f pos;
	Vec3f vel;
	Vec3f forward;
	Vec3f gravity;
	float radius;
	int player;
} Projectile;
/* Intialiser for projectiles*/
void initProj(Projectile *proj, Ship *ship, bool left);
/* Updates projectile's motion. */
int calcProjMotion(Projectile *proj, float dt);
/* Draws the projectile's */
void drawProj(Projectile *proj);
void removeProj(int index);

/* Collision detection */
void detectCollision();



/****************************************************
 Skybox functions
 ****************************************************/
typedef struct Skybox {
	GLuint *tex;
	float size;
} Skybox;
/* Initialises skybox */
void initSkybox(Skybox *box, float size);
/* Draws the skybox */
void drawSkybox(Skybox *box, Camera *cam);



/****************************************************
 Seabed functions
 ****************************************************/
typedef struct Seabed {
	float scale;
	float height;
	float displacement;
	GLuint tex;
	Image *image;
} Seabed;

/* Initialises values for drawing seabed */
bool initSeabed(Seabed *terrain, float size, float displace, 
				float height, const char *texFile,const char *heightFile);
/* Loads heightmap & draws seabed */
void drawSeabed(Seabed *terrain);



/****************************************************
 Misc functions
 ****************************************************/
/* Determines game outcome */
void endGame(int result, int loser);

/* Resets game to replay */
void resetGame();

/* Writes text to the screen */
void writeText(Vec3f pos, void* font, char* s, int w, int h);

/* Finds height of seabed at specified point */
float findHeight(Seabed *terrain, float x, float z);

/* Resets ship for game start */
void resetShip(Ship *ship, Seabed *seabed, Vec3f pos);

/* Calculates ships position */
void calcShipPos(Ship *ship, Seabed *seabed, Keys *keys, float dt);

/* Reshapes the scene */
void reshape(int w, int h);

/* Calculates the frames per second */
bool calcFPS(char *FPS, float time, int noFrames);




