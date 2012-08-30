################################################################################################
#
# Alyssa Biasi
#
################################################################################################
UNAME := $(shell uname)

# Linux (default)
CFLAGS = `pkg-config gdk-pixbuf-2.0 --cflags` -Wall
LDFLAGS = -lGL -lGLU -lglut -lm -lz `pkg-config gdk-pixbuf-2.0 --libs` -std=c99
TEXT_PLATFORM_SOURCES = texture_gdk.c

# Windows (cygwin)
ifeq "$(UNAME)" "Windows_NT"
	CFLAGS = `pkg-config gdk-pixbuf-2.0 --cflags` -Wall
	LDFLAGS = -lopengl32 -lglu32 -lglut32 `pkg-config gdk-pixbuf-2.0 --libs`
	TEXT_PLATFORM_SOURCES = texture_gdk.c
endif
 
# OS X
ifeq "$(UNAME)" "Darwin"
	CFLAGS = 
	LDFLAGS = -framework Carbon -framework QuickTime \
			  -framework OpenGL -framework GLUT \
			  -std=c99 -m32 -lz
	TEXT_PLATFORM_SOURCES = texture_quicktime.c
endif


PIRATES = pirates
PIRATES_SOURCES = obj.c pirates-main.c shipFunc.c waveFunc.c \
		utils.c texture_common.c png_loader.c \
		$(TEXT_PLATFORM_SOURCES)
PIRATES_HEADERS = obj.h texture.h uthash.h utils.h png_loader.h

all : $(PIRATES)

$(PIRATES) : $(PIRATES_SOURCES) $(PIRATES_HEADERS)
	gcc -o $@ $(PIRATES_SOURCES) $(CFLAGS) $(LDFLAGS)

clean : 
	rm -f $(PIRATES)
