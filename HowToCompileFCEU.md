
---

# Things you need #

### MinGW and MSYS ###
You can find them at http://www.mingw.org/. Look for the Current MSYS and Current MinGW. Everything else can be ignored unless you know what you're doing.

### libSDL for MinGW ###
You can find it at http://www.libsdl.org/.

### The DirectX 7a headers and libraries for MinGW ###
You can find it at http://alleg.sourceforge.net/index.html

### Zlib ###
You can find it at http://www.zlib.net/

### Windows API libraries ###
http://www.mingw.org/download.shtml

### FCEU source ###
Which I think you should have since you want to compile it! ;)


---

# Things to do #

### Install MinGW and MSYS. ###
Simply run the EXEs and follow the directions. Install MSYS AFTER MinGW

### Install the DirectX 7 headers and libraries and Windows API libraries ###
Simply unzip the archive to C:\MinGW.
There will be an include folder and a lib folder. Do NOT extract to C:\MinGW\dx70\_mgw.
Or you can extract to some folder and copy the include and lib folder over the C:\MinGW directory. You should do the same thing with Windows API libraries.

### Install libSDL. ###
Start MSYS and put SDL-devel-1.x.x.mingw.tar.gz in your home directory.
The path will be something like c:\msys\1.0\home\|username|

Untar the archive
> gzip -cd SDL-devel-1.x.x.mingw.tar.gz | tar x
Now build libSDL
> make install-sdl prefix=/mingw

### Install Zlib ###
Stay in MSYS, or reopen it if you closed it.
Put the zlib-1.x.x.tar.gz archive in your MSYS home dir.

Untar the archive
> tar xvfz zlib-1.x.x.tar.gz
or for bz2
> tar xvfj zlib-1.x.x.tar.bz2
Now change to zlib directory
> cd zlib-1.x.x
Configure and build zlib
> ./configure --prefix=/mingw
> make && make install

### Compile fceu ###
Stay in MSYS, or reopen it if you closed it.
Extract fceu-0.98.12-blip.src.rar to c:\msys\1.0\home\|username| .

Change to the fceu directory
> cd fceu-0.98.12-blip.src
Configure the Makefile
> ./configure --with-nativewin32
or for opengl
> ./configure --with-nativewin32 --with-opengl
Build the resource file
> windres -o src/res.o src/drivers/win/res.res
Now make
> make

### Strip the executable to make it small ###
This is optional.

Staying in MSYS, run the strip program.
> strip -s src/fceu.exe
After being "stripped", I suggest using upx, http://upx.sourceforge.net/ , to compress the executable.

The program should be in your home directory under c:\msys\1.0\home\|username|\fceu-0.98.26\src\fceu.exe