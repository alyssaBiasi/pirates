#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <stdio.h>

#include "texture.h"

using namespace Gdiplus;
using namespace std;

/*
 * This version of load_texture is intended to be used with Visual C++ 2008,
 * compile it using the vcproj file provided, following the instructions
 * provided on the I3D webpage for compiling using Visual C++. This version
 * uses GDI+ to load images and is written in C++.
 * More details: http://msdn2.microsoft.com/en-us/library/ms533798(VS.85).aspx
 */
GLuint texture_load(const char *filename)
{
    // Init gdi+
    GdiplusStartupInput input;
    ULONG_PTR token;
    GdiplusStartup(&token, &input, 0);

    // Bitmap constructor expects wide char, convert filename to that
    int needed = MultiByteToWideChar(CP_UTF8, 0, filename, -1, 0, 0);
    wchar_t* wfilename = new wchar_t[sizeof(wchar_t) * needed];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, needed);

    // Create the bitmap and make sure it worked, if it didn't, it's your fault
    Bitmap* bitmap = new Bitmap(wfilename);
    Status status = bitmap->GetLastStatus();
    if (status != Ok)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 0;
    }

    delete [] wfilename;

    // We don't like standard data types 'round these parts
    UINT height = bitmap->GetHeight();
    UINT width = bitmap->GetWidth();
    PixelFormat bitmapPixelFormat = bitmap->GetPixelFormat();
    INT channels;
    GLuint internalFormat, format;

    switch (bitmapPixelFormat)
    {
        case PixelFormat32bppRGB:
            channels = 4;
            break;

        case PixelFormat32bppARGB:
        case PixelFormat16bppARGB1555:
        case PixelFormat32bppPARGB:
        case PixelFormat64bppARGB:
        case PixelFormat64bppPARGB:
            bitmapPixelFormat = PixelFormat32bppARGB;
            channels = 4;
            break;

        default:
            bitmapPixelFormat = PixelFormat24bppRGB;
            channels = 3;
            break;
    }

    // Sometimes you'll want the internal format to be different, but for a
    // basic texture loader this does the job
    switch (channels)
    {
        case 1:
            format = internalFormat = GL_LUMINANCE;
            break;

        case 2:
            format = internalFormat = GL_LUMINANCE_ALPHA;
            break;

        case 3:
            format = internalFormat = GL_RGB;
            break;

        default:
            format = internalFormat = GL_RGBA;
            break;
    }

    Rect rect;
    rect.X = 0; rect.Y = 0;
    rect.Width = (INT)width;
    rect.Height = (INT)height;
    BitmapData bitmapData;
    bitmap->LockBits(&rect, ImageLockModeRead, bitmapPixelFormat, &bitmapData);
    size_t size = bitmapData.Height * bitmapData.Stride;
    unsigned char* data = new unsigned char[size];
    memcpy(data, bitmapData.Scan0, size);
    bitmap->UnlockBits(&bitmapData);

    // Convert BGR to RGB
    for (size_t i = 0; i < size; i += channels)
    {
        char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    }

    GLuint id = texture_load_data(data, width, height, 
                                  channels, -bitmapData.Stride, 
                                  internalFormat, format, GL_UNSIGNED_BYTE); 

    delete [] data;
    delete bitmap;

    return id;
}
