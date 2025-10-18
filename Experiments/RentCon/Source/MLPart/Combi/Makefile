#/**************************************************************************
#***    
#*** Copyright (c) 1995-2000 Regents of the University of California,
#***               Andrew E. Caldwell, Andrew B. Kahng and Igor L. Markov
#*** Copyright (c) 2000-2007 Regents of the University of Michigan,
#***               Saurabh N. Adya, Jarrod A. Roy, David A. Papa and
#***               Igor L. Markov
#***
#***  Contact author(s): abk@cs.ucsd.edu, imarkov@umich.edu
#***  Original Affiliation:   UCLA, Computer Science Department,
#***                          Los Angeles, CA 90095-1596 USA
#***
#***  Permission is hereby granted, free of charge, to any person obtaining 
#***  a copy of this software and associated documentation files (the
#***  "Software"), to deal in the Software without restriction, including
#***  without limitation 
#***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
#***  and/or sell copies of the Software, and to permit persons to whom the 
#***  Software is furnished to do so, subject to the following conditions:
#***
#***  The above copyright notice and this permission notice shall be included
#***  in all copies or substantial portions of the Software.
#***
#*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
#*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
#*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
#*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
#*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#***
#***
#***************************************************************************/



.SUFFIXES:  .o .cxx  .exe .exeP .exeS .h.gch

# ##################
# standard variables
# ##################

# 990812 ilm upgraded for use with g++

OPTFLAGS    = -DABKDEBUG -g #-xwe #-D__STL_DEBUG
#OPTFLAGS    = -O3
CCFLAGS     = -G -pic +p # comment +w/+w2 when compiling with O3
DEPENDOPT   = -xM1
CC          = CC $(OPTFLAGS) $(CCFLAGS)
LD          = CC -G
LDFINAL     = CC
CC4LIBS          = CC6
# could be g++ 
AR          = ar -rv
SODIRKEY =  -R
SONAMEKEY        = -R # change to -h if you need "immobile"dynamic libraries.
                      # LD_LIBRARY_PATH overrides -h
                      # Make sure there is no conflict
        
IMPORT   = NOT_USED
#LIBBASE    = /x/xx/lib/$(CC4LIBS)/g
LIBBASE  = .
SLIBBASE    = $(LIBBASE)
ARNAME      = lib$(THISLIB).a
SONAME      = lib$(THISLIB).so
SODIR       = $(PWD)
SOFULL      = $(SODIR)/$(SONAME)
SOLABEL     = $(SONAMEKEY)$(SOFULL) 
TESTNAME    = $(THISLIB)Test

STL         = 
# -I$(IMPORT)/STL/CC

# #########################################################
# standard variables that need modification in each project
# #########################################################

THISLIB = Combi
OBJS        = mapping.o permut.o grayPermut.o grayPart.o partitionIds.o
SRCS        =  $(OBJS:%.o=%.cxx)
INCLUDES    = -I. -I.. $(STL)
LIBPATHS    = $(SODIRKEY)$(LIBBASE)
LIBS        = -L$(LIBBASE) $(SODIRKEY). $(LIBPATHS) -l$(THISLIB)\
	      -lABKCommon
SLIBS       = -L$(SLIBBASE) -l$(THISLIB) -lABKCommon 
            
PURIFY      = purify -cache_dir=cache -always_use_cache_dir -windows=no\
              -log_file=purify.log 
# -append_logfile

# ################
# standard targets
# ################

all: lib

lib: $(ARNAME) $(SOFULL)
	ranlib *.a
	\cp -f *.a *.so ../libs

$(SOFULL):        $(OBJS)
	/bin/rm -f $(SOFULL)
	$(LD) $(INCLUDES) -o $(SOFULL) $(OBJS) $(SOLABEL)

$(ARNAME):        $(OBJS)
	/bin/rm -f $(ARNAME)
	$(AR) $(ARNAME) $(OBJS)

.cxx.o: 
	$(CC) $(INCLUDES) -c $*.cxx

.h.h.gch:
	$(CC) $(INCLUDES) $(ABKSYMBOLS) $*.h

.cxx.exe:
	/bin/rm -f $*.exe
	$(CC) $(INCLUDES) -c $*.cxx
	$(LDFINAL) -o $*.exe $*.o $(LIBS)

.cxx.exeS:
	/bin/rm -f $*.exeS
	$(CC) $(INCLUDES) -c $*.cxx
	$(LDFINAL) -o $*.exeS $*.o $(SLIBS)

.cxx.exeP:
	/bin/rm -f $*.exeP
	$(CC) $(INCLUDES) -c $*.cxx
	$(PURIFY) $(LDFINAL) -o $*.exeP $*.o $(LIBS)

depend:
	/bin/rm -f depend.mk

depend.mk:
	$(CC) $(DEPENDOPT) $(INCLUDES) $(SRCS) *.cxx >depend.mk

clean:  
	rm -rf *.o Templates.DB seeds.{in,out} *.exe* *.diff\
	$(SOFULL) $(ARNAME) lib*.so core ir.out *.notime depend.mk\
        cache SunWS_cache new.out *notime *.h.gch

test: CombiTest0.exe  CombiTest1.exe CombiTest2.exe CombiTest3.exe\
      CombiTest4.exe CombiTest5.exe CombiTest6.exe

testS: CombiTest0.exeS CombiTest1.exeS
testP: CombiTest0.exeP CombiTest1.exeP

.PHONY: depend, new, clean, test, testP, lib

#include depend.mk

# ##################################################
# special variables related only the current project
# ##################################################

# ########################
# project-specific targets
# ########################
