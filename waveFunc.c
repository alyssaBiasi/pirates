/***********************************************************************
 *  	COSC1187 - Interactive 3D Graphics and Animation
 *  		       Assignment 2 - Water grid functions
 *    Full Name        : Alyssa Biasi
 *    Student Number   : 3328976
 ***********************************************************************/


#include "utils.h"

SineFunc sineWaveX = {1.0f, 0.5f, 1.0f};
SineFunc sineWaveZ = {1.0f, 0.5f, 1.0f};

/* An absolute measure of time passed (in seconds) */
static float animationTime = 0;


/* Initialises 2D grid */
bool initGrid(WaterGrid *grid, int rows, int cols, float size, const char *texFile) {
	int i,j,index;
	float x,z;
	float start = -size/2.0f;
	
	/* Sets min size */
	if(rows<2) {
		rows = 2;
	}
	if(cols<2) {
		cols = 2;
	}
	
	int nVertices = (rows) * (cols);
	int nIndices = (rows-1) * (cols-1) * 6;
	
	/* Vertex/normal & index arrays */
	Vec3f *vertices = calloc(nVertices, sizeof(Vec3f));
	Vec3f *normals = calloc(nVertices, sizeof(Vec3f));
	int *indices = calloc(nIndices, sizeof(int));
	
	GLuint water_tex = texture_load(texFile);
	
	if(!water_tex) {
		printf("Water texture not created.\n");
		return false;
	}
	
	/* Populating vertex array. (x,z) static. */
	index = 0;
	for(i=0; i<rows; i++) {
		x = i/(float)(rows-1);
		x = (x-0.5)*size;
		
		for(j=0; j<cols; j++) {
			z = j/(float)(cols-1);
			z = (z-0.5)*size;
			
			vertices[index].x = x;
			vertices[index].z = z;
			index++;
		}
	}
	
	/* Creating an array of indices to form triangles */
	
	index = 0;
	for(i=0; i<(rows-1); i++) {
		for(j=0; j<(cols-1); j++) {
			indices[index++] = GRID_VERTEX(i, j);
			indices[index++] = GRID_VERTEX(i, j + 1);
			indices[index++] = GRID_VERTEX(i + 1, j);
			indices[index++] = GRID_VERTEX(i + 1, j);
			indices[index++] = GRID_VERTEX(i, j + 1);
			indices[index++] = GRID_VERTEX(i + 1, j + 1);
		}
	}
	
	/* Assigning variables */
	grid->rows = rows;
	grid->cols = cols;
	grid->size = size;
	grid->nVertices = nVertices;
	grid->nIndices = nIndices;
	grid->vertices = vertices;
	grid->normals = normals;
	grid->indices = indices;
	grid->tex = water_tex;
	
	/* Determining y values */
	updateGrid(grid, 0.0f);
	
	return true;
}


/* Deletes memory allocated by initGrid */
void resetGrid(WaterGrid *grid) {
	free(grid->vertices);
	free(grid->normals);
	free(grid->indices);
	
	grid->nVertices = 0;
	grid->nIndices = 0;
	grid->normals = 0;
	grid->indices = 0;
}


/* Draws the given grid */
void drawWater(WaterGrid *grid) {
	/* Blue water white specular highlight */
	static float diffuse[] = {0, 0.5, 1, 0.8};
	static float ambient[] = {0, 0.5, 1, 1};
	static float specular[] = {1, 1, 1, 1};
	static float shininess = 100.0f;
	
	int i;
	GLfloat u;
	GLfloat v;
	
	/* Applying material */
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	
	/* Draw tirangle grid */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, grid->tex);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1,1,1,0.8);
	
	glBegin(GL_TRIANGLES);
		for(i=0; i<(grid->nIndices); i++) {
			/* Prevent out of bounds error */
			assert(grid->indices[i] >= 0 && grid->indices[i] < grid->nVertices);
			
			Vec3f vec = grid->vertices[grid->indices[i]];
			Vec3f n = grid->normals[grid->indices[i]];
			
			u = vec.x;
			v = vec.z;
			
			if(u < 0) {
			    u *= -1;
			}
			if(v < 0) {
			    v *= -1;
			}
			
			/* 0 <= u, v <=  1 */
			u /= 100;
			v /= 100;
			
			glNormal3f(n.x, n.y, n.z);
			glTexCoord2f(u,v);
			glVertex3f(vec.x, vec.y, vec.z);
		}
	glEnd();
	
	glDisable(GL_BLEND);
	
	/* Draws flat plane so the water edge doesn't look so much 
	 * like the end of the world */
	float size = grid->size / 2 +50;
	
	glBindTexture(GL_TEXTURE_2D, grid->tex);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f( -size, -2, -size );
	
		glTexCoord2f(0, 1); glVertex3f( -size, -2,  size );
	
		glTexCoord2f(1, 1); glVertex3f(  size, -2,  size );
	
		glTexCoord2f(1, 0); glVertex3f(  size, -2, -size );
    glEnd();
	
	
}


/* Calculates y value at a given position */
float calcHeight(SineFunc *func, float x, float t) {
	float height = (func->A)*sinf((func->k)*x + (func->w)*t);
	
	return height;
}


/* Calculates un-normalized normal vector at a given position */
Vec2f calcNormal(SineFunc *func, float x, float t) {
	Vec2f n;
	n.x = -(func->A)*(func->k)*cosf((func->k)*x + (func->w)*t);
	n.y = 1.0;
	
	return n;
}


/* Returns Vec4f = normal & heigh at (x,z) */
Vec4f calcSineValue(float x, float z) {
	float mag;
	Vec4f v;
	float t = animationTime;
	
	/* Sum heights of both waves at (x,z) */
	v.w = calcHeight(&sineWaveX, x, t) + calcHeight(&sineWaveZ, z, t);
	
	/* Normal for each sine wave */
	Vec2f normX = calcNormal(&sineWaveX, x, t);
	Vec2f normZ = calcNormal(&sineWaveZ, z, t);
	
	/* Combining normals */
	v.x = normX.x;
	v.y = 1.0f;
	v.z = normZ.x;
	
	/* Normalizing */
	mag = sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
	v.x = v.x/mag;
	v.y = v.y/mag;
	v.z = v.z/mag;
	
	return v;
}


/* Applies wave effect, animating using dt */
void updateGrid(WaterGrid *grid, float dt) {
	int i;
	
	/* Anim.Time used in calcSineValue */
	animationTime  += dt;
	
	for(i=0; i<(grid->nVertices); i++) {
		Vec4f v = calcSineValue(grid->vertices[i].x, grid->vertices[i].z);
		
		grid->vertices[i].y = v.w;
		grid->normals[i].x = v.x;
		grid->normals[i].y = v.y;
		grid->normals[i].z = v.z;
	}
}


/* Draws normal vectors */
void drawNormals(WaterGrid *grid, float size) {
	int i;
	
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
	
	glDisable(GL_LIGHTING);
	glColor3f(1,1,0);
	
	glBegin(GL_LINES);
		for(i=0; i<(grid->nVertices); i++) {
			glVertex3f(grid->vertices[i].x, grid->vertices[i].y, grid->vertices[i].z);
			glVertex3f(grid->vertices[i].x + grid->normals[i].x * size,
					   grid->vertices[i].y + grid->normals[i].y * size,
					   grid->vertices[i].z + grid->normals[i].z * size);
		}
	glEnd();
	
	glPopAttrib();
}
