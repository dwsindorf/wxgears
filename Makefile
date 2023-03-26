#################
# MODULE    $Id: Makefile,v 1.1 2010-12-03 21:21:46 dean Exp $
#################
MAKEFILE       = Makefile
SHELL          = /bin/sh
#################
# Compiler parameters
#################
CC            = g++
#################
#CFILES        = wxgears.cpp glew.cpp
CFILES        = wxgears.cpp

OBJS          = $(CFILES:%.cpp=%.o)
#################
PROGRAM       = wxgears
OPTLVL        = -g
#DEFINES	      = -DWXWIN -DGLEW_STATIC -w -DGLEW -DGLEW_STATIC -Wall -Wno-non-virtual-dtor -fno-strict-aliasing 
DEFINES	      = -DWXWIN -DGLEW_STATIC -w -Wall -Wno-non-virtual-dtor -fno-strict-aliasing 
CFLAGS        = $(OPTLVL) $(DEFINES) $(GCCFLAGS) -I./
LFLAGS        = -lX11
#CFLAGS = -pthread -Wall -Wundef -O2 -fno-strict-aliasing $(DEFINES)

#VERSION = --version=2.8
#################
# .SUFFIXES   - implicit dependency rules
#################
.SUFFIXES:
.SUFFIXES: .cpp .o
#################

all:  		$(OBJS) program
			    
program:;   $(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS) `wx-config $(VERSION) --libs --gl_libs` $(LFLAGS);
		    @echo "######## done ##########"

$(OBJS):;   $(CC) $(CFLAGS) `wx-config $(VERSION) --cxxflags` $(GCCFLAGS) -c -o $(@) $(@F:%.o=%.cpp)
		
test:;     ./$(PROGRAM)

clean:;     rm -f *.o $(PROGRAM)

showenv:;      @echo LIBS  = `wx-config $(VERSION) --libs --gl_libs`;
			   @echo FLAGS = `wx-config $(VERSION) --cxxflags`;
			   @echo GCCFLAGS = $(GCCFLAGS)
			   @echo "PATH  = $(PATH)"
               
### DEPENDENCIES ###
wxgears.o: wxgears.cpp wxgears.h
glew.o: glew.cpp

