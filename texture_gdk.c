/* Texture loader for Linux using gdk-pixbuf-2.0 (a component of GTK).
 * Can load PNG, JPEG, GIF, BMP, TIFF, ..., etc.  
 *
 * Make sure the image has dimensions that are powers of two! (e.g., 32x64 is
 * ok, but 30x70 is not).
 *
 * Compile this file on Linux with 
 *  
 *   gcc -c texture_gdk.c `pkg-config gdk-pixbuf-2.0 --cflags --libs`
 *
 * or use the Makefile from the I3D website.
 */

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "texture.h"

GLuint texture_load(const char *filename)
{
    GdkPixbuf *pixbuf;
    int width, height, rowstride, channels;
    guchar *pixels;

    GLuint id;
    GLint internalformat;
    GLenum format;

    g_type_init();
    pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

    if (!pixbuf)
    {
        fprintf(stderr, "Could not load file %s\n", filename);
        return 0;
    }

    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);

    if (!texture_is_valid_dimensions(width, height))
    {
        fprintf(stderr, "Image %s does not have dimensions of power 2\n", 
                filename);
        gdk_pixbuf_unref(pixbuf);
        return 0;
    }

    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    channels = gdk_pixbuf_get_n_channels(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    switch (channels) 
    {
        case 1:
            internalformat = format = GL_LUMINANCE;
            break;
        case 2:
            internalformat = format = GL_LUMINANCE_ALPHA;
            break;
        case 3:
            internalformat = format = GL_RGB;
            break;
        case 4:
            internalformat = format = GL_RGBA;
            break;
    }

    id = texture_load_data(pixels, width, height, 
                           channels, -rowstride, internalformat, 
                           format, GL_UNSIGNED_BYTE);

    gdk_pixbuf_unref(pixbuf);

    return id;
}
