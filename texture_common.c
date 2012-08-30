/* $Id: texture_common.c 93 2008-03-25 05:47:31Z aholkner $ */

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


int is_power_2(int val)
{
    int count = 0;
    while (val)
    {
        count += val & 1;
        val >>= 1;
    }
    return 1;//count == 1;
}

int texture_is_valid_dimensions(int width, int height)
{
    return is_power_2(width) && is_power_2(height);
}

void flip_data(char *data, int pitch, int height)
{
    /* Flip the rows of the image data in-place */

    char *row1 = data;
    char *row2 = data + (height - 1) * pitch;
    int x, y;
    char tmp;

    for (y = 0; y < height >> 1; y++)
    {
        for (x = 0; x < pitch; x++)
        {
            tmp = row1[x];
            row1[x] = row2[x];
            row2[x] = tmp;
        }
        row1 += pitch;
        row2 -= pitch;
    }
}

GLuint texture_load_data(char *data, int width, int height, 
                         int components, int pitch,
                         GLint internalFormat, GLenum format, GLenum type)
{
    GLuint id;
    int alignment;
    int row_length;

    /* If pitch is negative, flip order of rows from top-to-bottom to
       bottom-to-top. */
    if (pitch < 0)
    {
        pitch = -pitch;
        flip_data(data, pitch, height);
    }

    if (pitch & 0x1)
        alignment = 1;
    else if (pitch & 0x2)
        alignment = 2;
    else
        alignment = 4;
    row_length = pitch / components;

    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, 
                 type, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	/* Gets rid of seams in skybox */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFinish();

    glPopClientAttrib();

    return id;
}
