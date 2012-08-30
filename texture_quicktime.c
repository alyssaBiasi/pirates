/* Load a texture from an image using OS X QuickTime framework. 
 * Not tested on little-endian yet.
 */

#include <Carbon/Carbon.h>
#include <QuickTime/Movies.h>
#include <QuickTime/QuickTimeComponents.h>
#include <OpenGL/GL.h>

#include "texture.h"

/* GL_UNSIGNED_INT_... is a hack to convert ARGB format into RGBA.  The
 * BGRA extension is supported on all Apple hardware, so we don't bother
 * checking for it.
 */
#if TARGET_RT_BIG_ENDIAN
#  define ARGB_FORMAT GL_BGRA
#  define ARGB_TYPE GL_UNSIGNED_INT_8_8_8_8_REV
#elif TARGET_RT_LITTLE_ENDIAN
#  define ARGB_FORMAT GL_BGRA
#  define ARGB_TYPE GL_UNSIGNED_INT_8_8_8_8
#else
#  error Target endianness not specified, please use Apple gcc.
#endif

GLuint texture_load(const char *filename)
{
    FSSpec fss;
    GraphicsImportComponent gi;
    Rect rect;
    GWorldPtr world;

    int width;
    int height;
    int pitch;
    unsigned char *buffer;

    GLuint id;

    NativePathNameToFSSpec(filename, &fss);
    GetGraphicsImporterForFile(&fss, &gi);
    GraphicsImportGetNaturalBounds(gi, &rect);

    width = rect.right;
    height = rect.bottom;
    pitch = width * 4;
    buffer = malloc(pitch * height);
    QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &rect, NULL, NULL, 0,
                       buffer, pitch);

    GraphicsImportSetGWorld(gi, world, NULL);
    GraphicsImportDraw(gi);
    DisposeGWorld(world);
    CloseComponent(gi);

    id = texture_load_data(buffer, width, height,
                           4, -pitch,
                           GL_RGBA, ARGB_FORMAT, ARGB_TYPE);
    free(buffer);

    return id;
}
