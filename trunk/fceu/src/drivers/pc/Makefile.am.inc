fceud_SOURCES	=	drivers/pc/input.c drivers/pc/main.c 

fceud_SDL_SOURCES	=	drivers/pc/sdl.c drivers/pc/sdl-joystick.c drivers/pc/sdl-sound.c drivers/pc/sdl-throttle.c drivers/pc/sdl-video.c drivers/pc/unix-netplay.c

#fceud_DOS_SOURCES	=	drivers/pc/dos.c

if OPENGL
TMP_OGL	= drivers/pc/sdl-opengl.c
endif

fceud_SOURCES	+=	$(fceud_SDL_SOURCES) $(TMP_OGL)
