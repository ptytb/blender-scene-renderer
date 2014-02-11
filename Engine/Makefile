

CC=icc
CPP=icpc
LD=icc

INTEL_PATH=/media/usb3/prog/composerxe
INTEL_LIB=$(INTEL_PATH)/lib/ia32

CCFILES=\
render.c \
../mylib/targa.c \
../mylib/matrix.c \
../mylib/objldr.c \
../mylib/filemap.c \
../mylib/slabvec.c \
../mylib/mtlldr.c \
../mylib/texldr.c \
../mylib/strfeed.c \
../mylib/objvbo.c \
../mylib/bvhldr.c \
../mylib/numparse.c

#../mylib/gaussj.c

CPPFILES=\
engine.cpp \
scene.cpp \
entity.cpp \
inst.cpp \
controller.cpp \
scorp.cpp

LDFILES=\
engine.o \
render.o \
filemap.o \
matrix.o \
mtlldr.o \
objldr.o \
objvbo.o \
slabvec.o \
strfeed.o \
targa.o \
texldr.o \
bvhldr.o \
numparse.o \
scene.o \
entity.o \
inst.o \
controller.o \
scorp.o

#gaussj.o

CCFLAGS_COMMON= -c -std=gnu99 -pedantic -Wall -Wfatal-errors -march=i686 \
-I/media/usb3/ode-0.12/include 

CCFLAGS_RELEASE= -O3 -xHOST -no-prec-div -fno-exceptions -mfpmath=sse
# -mmmx -msse2

CCFLAGS_DEBUG= -g -O0 -Wextra -Wno-implicit-function-declaration \
-Wuninitialized -Wunused

CPPFLAGS_COMMON= -c -std=c++0x -pedantic -Wall -Wfatal-errors -march=i686

CPPFLAGS_RELEASE= -O3 -xHOST -no-prec-div

CPPFLAGS_DEBUG= -g -O0

LDFLAGS_COMMON= -o ../engine -L/media/usb3/ode-0.12/ode/src/.libs \
-L$(INTEL_LIB) -lirc -limf

LDFLAGS_RELEASE= -strip-all 

LDLIBS= -lGLEW -lGLU -lGL -lglut 
#-lode

release:
	@$(CC) $(CCFLAGS_COMMON) $(CCFLAGS_RELEASE) $(CCFILES)
	@$(CPP) $(CPPFLAGS_COMMON) $(CPPFLAGS_RELEASE) $(CPPFILES)
	@$(LD) $(LDFLAGS_COMMON) $(LDFLAGS_RELEASE) $(LDFILES) $(LDLIBS)

debug:
	@$(CC) $(CCFLAGS_COMMON) $(CCFLAGS_DEBUG) $(CCFILES)
	@$(CPP) $(CPPFLAGS_COMMON) $(CPPFLAGS_DEBUG) $(CPPFILES)
	@$(LD) $(LDFLAGS_COMMON) $(LDFILES) $(LDLIBS)

clean:
	@-rm *.o
	@-rm *.bc

.PHONY: release debug clean

