/***********************************************************************
 *  	COSC1187 - Interactive 3D Graphics and Animation
 *  		       Assignment 2 - Ship functions
 *    Full Name        : Alyssa Biasi
 *    Student Number   : 3328976
 ***********************************************************************/


#include "utils.h"


/* Ship struct initialiser */
void initShip(Ship *ship, const char *meshFile, Camera cam, int num) {
	ship->heading = 90;
	ship->pitch = 0;
	ship->roll = 0;
	
	ship->displacement = -0.8;
	
	ship->fVel = 0;
	ship->tVel = 0;
	ship->maxSpeed = 5.0;
	ship->acc = 3.0;
	ship->tAcc = 50.0;
	
	ship->rot.x = ship->pitch;
	ship->rot.y = ship->heading;
	ship->rot.z = ship->roll;
	
	ship->mesh = objMeshLoad(meshFile);
	initCamera(&cam, ship->pos, ship->rot);
	ship->cam = cam;
	
	ship->player = num;
}


/* Resets ship for game start */
void resetShip(Ship *ship, Seabed *seabed, Vec3f init) {
	float dy, height;
	float y = -2;		/* Account for trough of waves */
	
	/* Checks init position isn't on seabed. 
	 * Generates new (x,z) until a valid position is found. */
	do {
		init.x = 180*(rand()/(float)RAND_MAX) - 90;
		init.z = 180*(rand()/(float)RAND_MAX) - 90;
		
		height = findHeight(seabed, init.x, init.z);
		
		/* Ignores if the seabed height is higher than y */
		if(y>height) {
			dy = y-height;
		}
		else {
			dy = 0;
		}
	} while(dy < 7);
	
	ship->pos = init;
	ship->heading = 90;
	ship->fVel = 0;
	ship->tVel = 0;
	ship->health = 50;
}


/* Draws ship mesh. */
void drawShipMesh(OBJMesh *mesh) {
	int i;
	
	glBegin(GL_TRIANGLES);
		for (i = 0; i < mesh->numIndices; ++i) {
			unsigned int index = mesh->indices[i];
			float* vert = (float*)((void*)mesh->vertices + index * mesh->stride);
			float* norm = (float*)((void*)vert + mesh->normalOffset);
			
			if (mesh->hasNormals) {
				glNormal3fv(norm);
			}
		
			glVertex3fv(vert);
		}
	glEnd();
}


/* Calculates ship's position. */
void calcShipPos(Ship *ship, Seabed *seabed, Keys *keys, float dt) {
	Vec4f v;
	Vec2f headingDir;
	
	/* Forward speed of ship. */
	if(keys->up) {
		ship->fVel += ship->acc * dt;
	}
	if(keys->down) {
		ship->fVel -= ship->acc * dt;
	}
	
	ship->fVel = clamp(ship->fVel, 0, ship->maxSpeed);
	
	/* Turning speed of ship. */
	if(keys->left) {
		ship->tVel += ship->tAcc * dt;
	}
	if(keys->right) {
		ship->tVel -= ship->tAcc * dt;
	}
	
	/* Limit turning speed. 5x forward vel in degrees per second. */
	ship->tVel = clamp(ship->tVel, -fabs(ship->fVel)*5.0, fabs(ship->fVel)*5.0);
	ship->tVel -= myCrossPlatformMin(fabs(ship->tVel), 0.5) * sign(ship->tVel);
	
	/* Calculate heading vector of ship. */
	headingDir.x = sinf(ship->heading*PI/180.0);
	headingDir.y = cosf(ship->heading*PI/180.0);
	
	/* Moving the ship */
	ship->heading += ship->tVel * dt;
	ship->pos.x += headingDir.x * ship->fVel * dt;
	ship->pos.z += headingDir.y * ship->fVel * dt;
	
	
	/* Keeps ship within the playable area */
	if(ship->pos.x >= 100 || ship->pos.x <= -100 || 
			ship->pos.z >= 100 || ship->pos.z <=  -100)
	{
		resetShip(ship, seabed, newVec3f(5,0,5));
	}
	
	/* Calculate roll & pitch based on location. */
	v = calcSineValue(ship->pos.x, ship->pos.z);
	ship->pos.y = v.w - ship->displacement;
	ship->roll = asin(v.x)*180.0/PI;
	ship->pitch = asin(v.z)*180.0/PI;
	
	ship->rot.x = ship->pitch;
	ship->rot.y = ship->heading;
	ship->rot.z = ship->roll;
	
	
	/***********************************************************************
	 Forward and side vectors
	 ***********************************************************************/
	float sx, sy, sz, cx, cy, cz, theta;
	Vec3f forward, side;
	
	// rotation angle about Z-axis (roll)
    theta = ship->roll * PI/180.0;
    sz = sinf(theta);
    cz = cosf(theta);
	
	// rotation angle about Y-axis (yaw)
    theta = ship->heading * PI/180.0;
    sy = sinf(theta);
    cy = cosf(theta);
	
	// rotation angle about X-axis (pitch)
    theta = ship->pitch * PI/180.0;
    sx = sinf(theta);
    cx = cosf(theta);
	
	// determine forward vector
    forward.x = sy*cz + cy*sx*sz;
    forward.y = sy*sz - cy*sx*cz;
    forward.z = cy*cx;
	
	// determine side vector
    side.x = cy*cz - sy*sx*sz;
    side.y = cy*sz + sy*sx*cz;
    side.z = -sy*cx;
	
	ship->forward = forward;
	ship->side = side;
}


/* Draws the ship */
void drawShip(Ship *ship, Controls *controls) {
	static float diffuse[] = {0.89, 0.65, 0.41, 1.0};
	static float ambient[] = {0.89, 0.65, 0.41, 1.0};
	static float specular[] = {1.0, 1.0, 1.0, 1.0};
	static float shininess = 256.0f;

    glEnable(GL_LIGHTING);
	glPushMatrix();
		glTranslatef(ship->pos.x, ship->pos.y, ship->pos.z);
	
		if(controls->axes) {
			drawAxes(newVec3f(0,0,0), newVec3f(10,10,10));
		}
	
		glRotatef(ship->roll, 0,0,-1);
		glRotatef(ship->pitch, 1,0,0);
		glRotatef(ship->heading, 0,1,0);
	
		if(controls->axes) {
			drawAxes(newVec3f(0,0,0), newVec3f(10,10,10));
		}
	
		glScalef(0.1,0.1,0.1);
	
		glPushMatrix();
			glRotatef(-90, 0, 1, 0);
			
			/* Apply material */
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	
			drawShipMesh(ship->mesh);
		glPopMatrix();
	glPopMatrix();
	
	glPopMatrix();
}








