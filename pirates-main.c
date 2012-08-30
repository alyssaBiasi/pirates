/***********************************************************************
*  	COSC1187 - Interactive 3D Graphics and Animation
*  		       Assignment 2
*    Full Name        : Alyssa Biasi
*    Student Number   : 3328976
***********************************************************************/

#include "utils.h"

Keys *keys;
Ship *ships;
Light light;
Camera *cams;
Seabed seabed;
Skybox skybox;
WaterGrid water;
Projectile *proj;
Controls controls;

char *text, *restart, *fps;
int projIndex, noFrames;
float width, height, time;
bool gameEnded, gameStarted;

typedef enum {
	draw, win, ground
}Results;



/**********************************************************************
 Collisions and end game
 **********************************************************************/
void endGame(int result, int loser) {
	int i;
	gameEnded = true;
	
	/* Stops ships from continuing their motion */
	for(i=0; i<2; i++) {
		ships[i].fVel = 0;
		ships[i].tVel = 0;
	}
	
	for(i=0; i<2; i++) {
		initKeys(&keys[i]);
	}
	
	/* Checking game outcome */
	switch(result) {
		case draw:
			text = "It's a draw.";
			break;
			
		case win:
			if(loser == 0) {
				text = "Player 2 wins!";
			}
			else {
				text = "Player 1 wins!";
			}
			break;
			
		case ground:
			if(loser == 0) {
				text = "Player 1 ran aground.";
			}
			else {
				text = "Player 2 ran aground.";
			}
			break;
			
		default:
			/* Exiting */
			free(ships);
			free(cams);
			free(keys);
			free(proj);
			free(fps);
			resetGrid(&water);
			exit(0);
			break;
	}
}


void detectCollision() {
	const int HIT = 10;
	float distance = 0;
	float rTotal, rShip, dx, dy, dz;
	int i,j;
	
	rShip = 2;
	
	
	/****************************************************
	 Projectile collision
	 ****************************************************/
	for(i=0; i<projIndex; i++) {
		for(j=0; j<2; j++) {
			/* Stops the projectile colliding with the ship that fired it */
			if(proj[i].player == ships[j].player) {
				continue;
			}
			
			/* Min distance is sum of radii */
			rTotal = rShip + proj[i].radius;
			
			/* Finding distance between projectile & ship. */
			dx = ships[j].pos.x - proj[i].pos.x;
			dy = ships[j].pos.y - proj[i].pos.y;
			dz = ships[j].pos.z - proj[i].pos.z;
			
			distance = sqrt(dx*dx + dy*dy + dz*dz);
			
			if(distance <= rTotal) {
				ships[j].health -= HIT;
				
				/* Checks ship's health */
				if(ships[j].health <= 0) {
					endGame(win, j);
				}
				
				removeProj(i);
			}
		}
	}
	
	
	/****************************************************
	 Ship collision
	 ****************************************************/
	
	/* Min distance between ships */
	rTotal = 2 * rShip;
	
	/* Calculating distance between the 2 ships */
	distance = calcDistance();
	
	/* A draw if they collide */
	if(distance <= rTotal) {
		endGame(draw,0);
	}
	
	
	/****************************************************
	 Seabed collision
	 ****************************************************/
	
	 /* Ignores while ship position is being reset */
	if(gameStarted) {
		rShip = 2;
		for(i=0; i<2; i++) {
			float height = findHeight(&seabed, ships[i].pos.x, ships[i].pos.z);
			dy = ships[i].pos.y - height;
			
			if(dy <= rShip) {
				endGame(ground, i);
			}
		}
	}
}


/* Removes a projectile from the array */
void removeProj(int index) {
	int i;
	
	for(i=index; i<projIndex-1; i++) {
		proj[i] = proj[i+1];
	}
	
	projIndex--;
}


/**********************************************************************
 Updating and drawing
 **********************************************************************/
void idle() {
    static int last = -1;
	static char *lastFPS = "hello";
	bool printFPS = false;
	float dt = 0;
    int now = 0;
	int i, p;
	
	if(last == -1) {
		last = glutGet(GLUT_ELAPSED_TIME);
	}
	
	now = glutGet(GLUT_ELAPSED_TIME);
    dt = (now -last)/1000.0;
    last = now;
	time += dt;
	
	/* Updates everything only if the game hasn't ended */
	if(!gameEnded) {
		updateGrid(&water, dt);
		
		for(i=0; i<2; i++) {
			calcShipPos(&ships[i], &seabed, &keys[i], dt);
		}
		
		for(i=0; i<projIndex; i++) {
			p = calcProjMotion(&proj[i], dt);
			
			if(p<0) {
				removeProj(i);
				i--;
			}
		}
		
		if(gameStarted) {
			detectCollision();
		}
	}
	
	/* Calculates the frames/second */
	printFPS = calcFPS(fps, time, noFrames);
	
	if(printFPS) {
		/* Resets if new value */
		noFrames = 0;
		time = 0;
	}
	
    glutPostRedisplay();
}


/* Calculates distance between the ships */
float calcDistance() {
	float dx, dy, dz, distance;
	
	dx = ships[0].pos.x - ships[1].pos.x;
	dy = ships[0].pos.y - ships[1].pos.y;
	dz = ships[0].pos.z - ships[1].pos.z;
	
	distance = sqrt(dx*dx + dy*dy + dz*dz);
	
	return distance;
}


void display() {
	int i,j,k;
	Vec3f v;
	
	noFrames++;
	
    glLoadIdentity();
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	/* Wireframe or filled */
    if(controls.wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); /* wireframe */
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); /* filled */
    }
	
    /* Sets the positions of the camera */
	setupCamera(&ships[0], &ships[1]);	
	
	/* End screen - Stops the scene from drawing */
	if(gameEnded) {
		v = newVec3f(width-width/2, height*2/4, 0);
		writeText(v, GLUT_BITMAP_TIMES_ROMAN_24, text, width*2, height);
		
		v.y -= 50.0f;
		writeText(v, GLUT_BITMAP_TIMES_ROMAN_24,restart, width*2, height);
		
		glutSwapBuffers();
		return;
	}
	
	/**********************************************************************
	 Drawing the scene
	 **********************************************************************/
	for(i=0, k=1; i<2; i++, k--) {
		/* Switches viewports & draws twice. */
		setViewport();
		
		/* Makes camera always look at the other ship. */
		gluLookAt(ships[i].cam.pos.x, ships[i].cam.pos.y, ships[i].cam.pos.z,
							ships[k].pos.x, ships[k].pos.y, ships[k].pos.z, 0,1,0);
			
		/* Draws skybox around camera */
		drawSkybox(&skybox, &(ships[i].cam));
			        
		if (controls.axes) {
			drawAxes(newVec3f(0, 0, 0), newVec3f(10, 10, 10));
		}
		
		setupLight(&light);
		
		drawSeabed(&seabed);
		
		drawWater(&water);
		
		/* Cannon balls */
		for(j=0; j<projIndex; j++) {
			drawProj(&proj[j]);
		}
		
		/* Water grid normals */
		if(controls.normals) {
			drawNormals(&water, 1);
		}
		
		/* Ships */
		for(j=0; j<2; j++) {
			drawShip(&ships[j], &controls);
			
		}		 
	}
	
	/* Prints frames per second */
	v = newVec3f(0,0,0);
	writeText(v, GLUT_BITMAP_HELVETICA_18, fps, width*2, height);
	
    glutSwapBuffers();
}


/* Creates viewport of half the window. 
 * Switches between left & right side */
void setViewport() {
	static bool left = true;
	
	glLoadIdentity();
	
	if(left) {
		/* Left viewport */
		glViewport(0,0,width,height);
	}
	else {
		/* Right viewport */
		glViewport(width,0,width,height);
	}
	
	left = !left;
}


/**********************************************************************
 Keyboard - update Controls and Keys
 **********************************************************************/
void keyboardDown(unsigned char key, int x, int y) {
	/* Checks for esc */
	if(key == 27) {
		endGame(-1,0);
	}
	
	/* Ignores switch statement if game has finished */
	if(gameEnded) {
		if(key == 13) {
			/* Enter key */
			gameEnded = false;
			resetGame();
		}
		
		return;
	}
	
	switch(key) {
		case 'b':
			/* Toggle wireframe */
			controls.wireframe = !controls.wireframe;
			break;
			
		case 'n':
			/* Toggle normals */
			controls.normals = !controls.normals;
			break;
			
		case 'v':
			/* Toggle axes */
			controls.axes = !controls.axes;
			break;
			
		/* Water tesselation */
		case '-':
		case '_':
			resetGrid(&water);
			initGrid(&water, water.rows/2, water.cols/2, water.size, "water.jpg");
			break;
			
		case '+':
		case '=':
			resetGrid(&water);
			initGrid(&water, water.rows*2, water.cols*2, water.size, "water.jpg");
			break;
			
		/* Movement for player 1 ship. */
		case 'w':
			/* Up */
			keys[0].up = !keys[0].up;
			break;
			
		case 's':
			/* Down */
			keys[0].down = !keys[0].down;
			break;
			
		case 'a':
			/* Left */
			keys[0].left = !keys[0].left;
			break;
			
		case 'd':
			/* Right */
			keys[0].right = !keys[0].right;
			break;
			
		/* Player 1 cannons */
		case 'q':
			/* Left cannon */
			if(projIndex < MAX_PROJ) {
			    initProj(&proj[projIndex], &ships[0], true);
			    projIndex++;
			}
			break;
			
		case 'e':
			/* Right cannon */
			if(projIndex < MAX_PROJ) {
		    	initProj(&proj[projIndex], &ships[0], false);
		    	projIndex++;
			}
			break;
			
		/* Player 2 cannons */
		case 127: /* delete key */
		case 'k':
			/* Left cannon */
			if(projIndex < MAX_PROJ) {
			    initProj(&proj[projIndex], &ships[1], true);
			    projIndex++;
			}
			break;
			
		case 'l':
			/* Right cannon */
			if(projIndex < MAX_PROJ) {
		    	initProj(&proj[projIndex], &ships[1], false);
			    projIndex++;
			}
			break;
			
		default:
			break;
	}
}


void keyboardUp(unsigned char key, int x, int y) {
	if(gameEnded) {
		return;
	}
	
	switch(key) {
		/* Movement for player 1 ship. */
		case 'w':
			/* Up */
			keys[0].up = !keys[0].up;
			break;
			
		case 's':
			/* Down */
			keys[0].down = !keys[0].down;
			break;
			
		case 'a':
			/* Left */
			keys[0].left = !keys[0].left;
			break;
			
		case 'd':
			/* Right */
			keys[0].right = !keys[0].right;
			break;
			
		default:
			break;
	}
}


void updateKeys(int key, bool state) {
    static bool fired = false;
	if(gameEnded) {
		return;
	}
	
	switch(key) {
		/* Player 2 movement */
		case GLUT_KEY_UP:
			keys[1].up = state;
			break;
			
		case GLUT_KEY_DOWN:
			keys[1].down = state;
			break;
			
		case GLUT_KEY_LEFT:
			keys[1].left = state;
			break;
			
		case GLUT_KEY_RIGHT:
			keys[1].right = state;
			break;
			
		/* Player one cannons */
		case GLUT_KEY_PAGE_DOWN:
			/* Right cannon */
			if(!fired && projIndex < MAX_PROJ) {
			    initProj(&proj[projIndex], &ships[1], false);
			    projIndex++;
			}
			fired = !fired;
			break;
			
		default:
			break;
	}
}


void arrowDown(int key, int x, int y) {
    updateKeys(key, true);
}


void arrowUp(int key, int x, int y) {
    updateKeys(key, false);
}



/**********************************************************************
 Initialising
 **********************************************************************/
void reshape(int w, int h) {
    float aspect = 0;
	
	w = w/2;
	aspect = w/(float)h;

	width = w;
	height = h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0f, aspect, 0.1f, 210.0f);
    glMatrixMode(GL_MODELVIEW);
}


void resetGame() {
	int i;
	
	/* Resets control values */
	initControls(&controls);
	
	/* Resets the ships - sets initial position
	 * Ensures that there's a reasonable distance between ships */
	do {
		for(i=0; i<2; i++) {
			resetShip(&ships[i], &seabed, newVec3f(5,0,5));
		}	
	} while(calcDistance() < 25);
	
	reshape(width*2, height);
	
	gameStarted = true;
}


bool init() {
	int i;
	bool returned;
	
	/* Cameras */
	cams = malloc(sizeof(Camera)*2);
	
	/* Ships */
	ships = malloc(sizeof(Ship)*2);
	for(i=0; i<2; i++) {
		initShip(&ships[i], "galleon.obj", cams[i], (i+1));
	}	
	
	/* Ship keys */
	keys = malloc(sizeof(Keys)*2);
	for(i=0; i<2; i++) {
		initKeys(&keys[i]);
	}
	
	/* Projectiles - memory for specified number of cannon balls */
	proj = malloc(sizeof(Projectile)*MAX_PROJ);
	
	initControls(&controls);
	
	/* Water */
	returned = initGrid(&water, GRID_SIZE, GRID_SIZE, GRID_SIZE, "water.jpg");
	returned = initSeabed(&seabed, GRID_SIZE, 25.0, 20.0, "sand.jpg", "out.png");
	
	if(!returned) {
		return false;
	}
	
	/* Light */
	initLight(&light, newVec4f(1.2, 1, -1.5, 0), newVec4f(0.4, 0.3, 0.2, 1), 
			  newVec4f(0.5, 0.5, 0.5, 1), newVec4f(1, 1, 1, 0), 128);
	
	initSkybox(&skybox, GRID_SIZE/2);
	
	/* Initialising global variables */
	time = 0;
	projIndex = 0;
	noFrames = 0;
	gameEnded = false;
	gameStarted = false;
	restart = "Restart? Press enter.\0";
	fps = malloc(sizeof(char)*15);
	
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0,0,0,0);
	
	/* Enable lighting and lights */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	
	/* Sets game initial state */
	resetGame();
	
	return true;
}


int main(int argc, char **argv) {
	bool returned;
	
    glutInit(&argc, argv);
	
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("s3328976 - Assignment 2");
	
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
	
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(arrowDown);
    glutSpecialUpFunc(arrowUp);
	
	returned = init();
	
	/* Checks texture loads */
	if(!returned) {
		return 0;
	}
	
    glutMainLoop();
}
