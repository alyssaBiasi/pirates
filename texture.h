#ifdef WIN32
#  include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

GLuint texture_load(const char *filename);
GLuint texture_load_data(unsigned char *data, int width, int height, 
                         int components, int pitch,
                         GLint internalFormat, GLenum format, GLenum type);

int texture_is_valid_dimensions(int width, int height);

#ifdef __cplusplus
}
#endif
