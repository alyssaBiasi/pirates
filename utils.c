/***********************************************************************
 *  	COSC1187 - Interactive 3D Graphics and Animation
 *  		       Assignment 2 - All functions 
 *									(excluding ship and water grid)
 *    Full Name        : Alyssa Biasi
 *    Student Number   : 3328976
 ***********************************************************************/


#include "utils.h"

/****************************************************
 Vector functions
 ****************************************************/

/* Vec3f struct constructor */
Vec3f newVec3f(float x, float y, float z) {
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	
	return v;
}


/* Vec3f struct constructor */
Vec4f newVec4f(float x, float y, float z, float w) {
	Vec4f v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	
	return v;
}


/****************************************************
 Controls and Keys functions
****************************************************/

/* Controls struct initialiser */
void initControls(Controls *controls) {
	controls->wireframe = false;
	controls->normals = false;
	controls->axes = false;
}


/* Keys struct initialiser */
void initKeys(Keys *keys) {
	keys->up = false;
	keys->down = false;
	keys->left = false;
	keys->right = false;
}


/****************************************************
 Camera functions
 ****************************************************/

/* Camera struct initialiser */
void initCamera(Camera *camera, Vec3f pos, Vec3f rot) {
	camera->fieldOfView = 75.0f;
	camera->clipNear = 0.1f;
	camera->clipFar = 200.0f;
	camera->rotating = false;
	camera->zooming = false;
	camera->zoom = 0.5f;
	camera->sensitivity = 0.4f;
	
	camera->pos = newVec3f(0,10.0f,0);
	camera->rot = newVec3f(0,0,0);
}

/* Positions the cameras */
void setupCamera(Ship *ship1, Ship *ship2) {
    float theta, x, z, dx, dz;
    Camera *cam1, *cam2;
    
    /* Calculates difference in distance between ships. */
    dx = ship1->pos.x - ship2->pos.x;
    dz = ship1->pos.z - ship2->pos.z;
    
    /* Angle of the right-angle triangle formed by two ships. */
    theta = atan(dz/dx);
    
	/* Ensures positive angle */
    if(theta <  0) {
        theta *= -1;
    }
    
    /* x & z pos of the camera - right-angle triangle formed by the camera. */
    x = cos(theta) * CAM_DISTANCE;
    z = sin(theta) * CAM_DISTANCE;
    
    cam1 = &(ship1->cam);
    cam2 = &(ship2->cam);
        
	/* Sets position of camera depending on position of both ships */
    if(ship1->pos.x < ship2->pos.x) {
        cam1->pos.x = ship1->pos.x - x;
        cam2->pos.x = ship2->pos.x + x;
    }
    else {
        cam1->pos.x = ship1->pos.x + x;
        cam2->pos.x = ship2->pos.x - x;
    }     
    
    if(ship1->pos.z < ship2->pos.z) {
        cam1->pos.z = ship1->pos.z - z;
        cam2->pos.z = ship2->pos.z + z;
    }
    else {
        cam1->pos.z = ship1->pos.z + z;
        cam2->pos.z = ship2->pos.z - z;
    }
}


/****************************************************
 Light functions
 ****************************************************/

/* Initialises the light */
void initLight(Light *light, Vec4f pos, Vec4f ambient, Vec4f diffuse, 
			   Vec4f specular, float shininess)
{
	light->pos = pos;
	light->ambient = ambient;
	light->diffuse = diffuse;
	light->specular = specular;
	light->shininess = shininess;
}


/* Applies the values stored in light to GL_LIGHT0 */
void setupLight(Light *light) {
	glLightfv(GL_LIGHT0, GL_POSITION, (const GLfloat *) &(light->pos));
	glLightfv(GL_LIGHT0, GL_AMBIENT, (const GLfloat *) &(light->ambient));
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (const GLfloat *) &(light->diffuse));
	glLightfv(GL_LIGHT0, GL_SPECULAR, (const GLfloat *) &(light->specular));
	glLightf(GL_LIGHT0, GL_SHININESS, light->shininess);
}


/****************************************************
 Projectile functions
 ****************************************************/
/* Initialises projectiles */
void initProj(Projectile *proj, Ship *ship, bool left) {
	Vec3f side;
	
	proj->gravity = newVec3f(0,-9.8,0);
	proj->pos = ship->pos;
	
	side = ship->side;
	
	proj->vel.x = PROJ_VEL_HORIZONTAL;
	proj->vel.y = PROJ_INIT_VEL_VERTICAL;
	proj->vel.z = PROJ_VEL_HORIZONTAL;
	
	proj->radius = 0.5f;
	proj->player = ship->player;
	
	/* Ship's side vector = Projectile's forward vector */
	if(left) {
		/* Fire from left side */
		proj->forward = ship->side;
	}
	else {
		/* Fire from right side */
		proj->forward.x = -1*(ship->side.x);
		proj->forward.y = -1*(ship->side.y);
		proj->forward.z = -1*(ship->side.z);
	}
}


/* Updates projectile's motion */
int calcProjMotion(Projectile *proj, float dt) {
	float dy, dvy;
	
	Vec3f newPos = proj->pos;
	Vec3f newVel = proj->vel;
	
	/* dvy = change in vertical velocity. horizontal is constant */
	dy = 0.5 * dt * newVel.y;
	
	/* dy = change in y direction */
	dvy = 0.5 * dt * proj->gravity.y;
	newVel.y += dvy;
	
	/* Adjusting position. 
	 * (x,z) found using forward vector of projectile */
	newPos.y += dy;
	newPos.x += proj->forward.x * PROJ_VEL_HORIZONTAL * dt;
	newPos.z += proj->forward.z * PROJ_VEL_HORIZONTAL * dt;
	
	proj->pos = newPos;
	proj->vel = newVel;
	
	/* Checks that the projectile is above the water level */
	if(proj->pos.y <= -4) {
		return -1;
	}
	else {
		return 1;
	}
}


/* Draws the projectile */
void drawProj(Projectile *proj) {
	static float diffuse[] = {0.3, 0.3, 0.3, 1.0};
	static float ambient[] = {0.3, 0.3, 0.3, 1.0};
	static float specular[] = {1, 1, 1, 1};
	static float shininess = 100.0f;
	
	glPushMatrix();
		/* Applying material */
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	
		glTranslatef(proj->pos.x, proj->pos.y, proj->pos.z);
		glutSolidSphere(proj->radius,8,4);
	glPopMatrix();
}



/****************************************************
 Seabed functions
 ****************************************************/
bool initSeabed(Seabed *terrain, float size, float height,
			float displace, const char *texFile, const char *heightFile) {
	Image *image = load_png(heightFile);
    GLuint tex = texture_load(texFile);
	
	if(!image || !tex) {
		printf("Error: seabed file not loaded. \n");
		return false;
	}
	
	terrain->image = image;
	terrain->tex = tex;
	
	terrain->scale = size;
	terrain->displacement = displace;
	terrain->height = height;
	
	return true;
}


/* Loads heightmap & draws seabed */
void drawSeabed(Seabed *terrain) {
	static float diffuse[] = {0.93, 0.91, 0.80, 1.0};
	static float ambient[] = {0.93, 0.91, 0.80, 1.0};
	static float specular[] = {1, 1, 1, 1};
	static float shininess = 100.0f;
	
    int width, height, i, j, index;
    float x, y, z, x2, z2, u, v;
    float scale, scaleY, sizeW, sizeH;
	Image *image = terrain->image;
	Vec3f A,B,C,AB,AC,n;
    
    width = image->width;
    height = image->height;
	
	/* scales : -0.5 < x,z, < 05 */
    sizeW = 1.0/(float)width;
    sizeH = 1.0/(float)height;
	
	/* Scales to match water grid */
	scale = terrain->scale;
	scaleY = terrain->height;
	
    
	/* Applying material */
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	
	/* Draws seabed */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, terrain->tex);
    for(i=0; i<width-1; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for(j=0; j<height; j++) {
            x = (j*sizeH-0.5)*scale;
            z = (i*sizeW-0.5)*scale;
		
            index = (j*width + i) * image->channels;
            y = (scaleY * image->data[index]/255.0f) - terrain->displacement;
            
			/* Calculating texturing coordinates */
			u = x/100;
			v = z/100;
			
			if(u<0) {
				u *= -1;
			}
			if(v<0) {
				v *= -1;
			}
			
			/* Calculating normals */
			A = newVec3f(x, y, z);	/* This pos */
            
			x2 = ((j+1)*sizeH-0.5)*scale;
			index = ((j+1) * image->width + i) * image->channels;
			B = newVec3f(x2, image->data[index]/255.0f, z);		/* x direction */
            
            z2 = ((i+1)*sizeW-0.5)*scale;
            index = (j * image->width + (i+1)) * image->channels;
            C = newVec3f(x, image->data[index]/255.0f, z2);		/* z direction */
			
			AB = newVec3f(B.x-A.x, B.y-A.y, B.z-A.z);
			AC = newVec3f(C.x-A.x, B.y-A.y, B.z-A.z);
			
			float magAB = sqrt(pow(AB.x,2) + pow(AB.y,2) + pow(AB.z,2));
			float magAC = sqrt(pow(AC.x,2) + pow(AC.y,2) + pow(AC.z,2));
			float scalar = AB.x*AC.x + AB.y*AC.y + AB.z*AC.z;
			
			float theta = acos(scalar/(magAB*magAC));
			
			if(theta > 0) {
				/* Cross product */
				n = newVec3f(AC.y*AB.z-AC.z*AB.y, AC.z*AB.x-AC.x*AB.z, AC.x*AB.y-AC.y*AB.x);
			}
			else {
				/* Cross product */
				n = newVec3f(AB.y*AC.z-AB.z*AC.y, AB.z*AC.x-AB.x*AC.z, AB.x*AC.y-AB.y*AC.x);
			}
			
			/* Noramlizing */
            float mag = sqrt(pow(n.x,2) + pow(n.y,2) + pow(n.z,2));
            n.x = n.x/mag;
            n.y = n.y/mag;
            n.z = n.z/mag;
			
			/* COORDINATES */
			glNormal3f(n.x, n.y, n.z);
			glTexCoord2f(u,v);
            glVertex3f(x, y, z);
            
            index = (j*width + (i+1)) * image->channels;
            y = (scaleY * image->data[index]/255.0f) - terrain->displacement;
			
			/* Incrementing z & updating v texture coordinate */
			z = z+sizeW*scale;
			
			v = z/100;
			
			if(v<0) {
				v *= -1;
			}
			
			/* COORDINATES */
			glNormal3f(n.x, n.y, n.z);
			glTexCoord2f(u,v);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}


/* Finds height at given position */
float findHeight(Seabed *terrain, float x, float z) {
	Image *image = terrain->image;
	float sizeW = 1.0/(float)(image->width);
    float sizeH = 1.0/(float)(image->height);
	int i, j, index;
	float y;
	
	/* Convert (x,z) to (i,j) */
	i = (int)(((z/terrain->scale) + 0.5)/sizeW);
	j = (int)(((x/terrain->scale) + 0.5)/sizeH);
	
	/* Use (i,j) to determine y */
	index = (j*image->width + i) * image->channels;
	y = (terrain->height *image->data[index]/255.0f) - terrain->displacement;
	
	return y;
}



/****************************************************
 Skybox functions
 ****************************************************/
void initSkybox(Skybox *box, float size) {
	box->size = size;
	
	box->tex = malloc(sizeof(GLuint)*5);
	
	box->tex [0] = texture_load("skyFront.jpg");
	box->tex [1] = texture_load("skyLeft.jpg");
	box->tex [2] = texture_load("skyBack.jpg");
	box->tex [3] = texture_load("skyRight.jpg");
	box->tex [4] = texture_load("skyTop.jpg");
}


/* Draws the skybox */
void drawSkybox(Skybox *box, Camera *cam) {
	float size = 1.0;
	Vec3f pos = cam->pos;
	
	glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	
	// Front quad
	glBindTexture(GL_TEXTURE_2D, box->tex[0]);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex3f( pos.x+size, pos.y-size, pos.z-size);
	
		glTexCoord2f(1,0); glVertex3f( pos.x-size, pos.y-size, pos.z-size);
	
		glTexCoord2f(1,1); glVertex3f( pos.x-size, pos.y+size, pos.z-size);
	
		glTexCoord2f(0,1); glVertex3f( pos.x+size, pos.y+size, pos.z-size);
	glEnd();
	
	// Left quad
	glBindTexture(GL_TEXTURE_2D, box->tex[1]);
    glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex3f( pos.x+size, pos.y-size, pos.z+size );
	
		glTexCoord2f(1,0); glVertex3f( pos.x+size, pos.y-size, pos.z-size );
	
		glTexCoord2f(1,1); glVertex3f( pos.x+size, pos.y+size, pos.z-size );
	
		glTexCoord2f(0,1); glVertex3f( pos.x+size, pos.y+size, pos.z+size );
    glEnd();
	
	// Back quad
	glBindTexture(GL_TEXTURE_2D, box->tex[2]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f( pos.x-size, pos.y-size, pos.z+size );
		
		glTexCoord2f(1, 0); glVertex3f( pos.x+size, pos.y-size, pos.z+size );
		
		glTexCoord2f(1, 1); glVertex3f( pos.x+size, pos.y+size, pos.z+size );
		
		glTexCoord2f(0, 1); glVertex3f( pos.x-size, pos.y+size, pos.z+size );
    glEnd();
	
	// Right quad
	glBindTexture(GL_TEXTURE_2D, box->tex[3]);
    glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex3f( pos.x-size, pos.y-size, pos.z-size );
	
		glTexCoord2f(1,0); glVertex3f( pos.x-size, pos.y-size, pos.z+size );
	
		glTexCoord2f(1,1); glVertex3f( pos.x-size, pos.y+size, pos.z+size );
	
		glTexCoord2f(0,1); glVertex3f( pos.x-size, pos.y+size, pos.z-size );
    glEnd();
	
	// Top quad
	glBindTexture(GL_TEXTURE_2D, box->tex[4]);
    glBegin(GL_QUADS);
		glTexCoord2f(0,1); glVertex3f( pos.x-size, pos.y+size, pos.z-size );
	
		glTexCoord2f(0,0); glVertex3f( pos.x-size, pos.y+size, pos.z+size );
	
		glTexCoord2f(0,1); glVertex3f( pos.x+size, pos.y+size, pos.z+size );
	
		glTexCoord2f(1,1); glVertex3f( pos.x+size, pos.y+size, pos.z-size );
    glEnd();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}



/****************************************************
 Misc functions
 ****************************************************/

/* Draws a 3d axis */
void drawAxes(Vec3f pos, Vec3f length) {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
	
	glDisable(GL_LIGHTING);
	
	/* Draw the axes, red for x, green for y and blue for z */
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(pos.x, 0, 0); glVertex3f(pos.x + length.x, 0, 0);
	
	glColor3f(0, 1, 0);
	glVertex3f(0, pos.y, 0); glVertex3f(0, pos.y + length.y, 0);
	
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, pos.z); glVertex3f(0, 0, pos.z + length.z);
	glEnd();
	
	glPopAttrib();
}


/* Writes message to screen */
void writeText(Vec3f pos, void* font, char* s, int w, int h) {
	/* Sets window for text writing */
	glLoadIdentity();
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho (0.0, w, 0.0, h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
	
	glDisable(GL_LIGHTING);
	glColor3f(1,0,0);
	glRasterPos3f(pos.x, pos.y, pos.z);
	
	/* Writing text */
	while (*s) {
		glutBitmapCharacter(font, *s);
		s++;
	}
	
	glPopAttrib();
	
	/* Resets window to draw scene */
	reshape(w, h);
}


/* Calculates the frames/second - returns whether it was updated */
bool calcFPS(char *FPS, float time, int noFrames) {
	static float lastPrint = 0;
	float fps;
	
	/* Calculates frames/sec */
	if(time > lastPrint+0.1) {
		fps = noFrames/time;
		
		/* Stores string in specified char array */
		sprintf(FPS, "FPS: %0.5f", fps);
		
		lastPrint = time;
		return true;
	}
	else {
		return false;
	}
}







