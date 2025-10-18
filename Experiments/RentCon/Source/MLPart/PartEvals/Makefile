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



# 970821 Created by Max Moroz and Igor Markov,  VLSICAD ABKGroup, UCLA/CS
# 990812 ilm upgraded for use with g++

.SUFFIXES:        .o .cxx .exe .exeP .exeS .h.gch

# ##################
# standard variables
# ##################

 OPTFLAGS        = -g -DABKDEBUG 
#OPTFLAGS        = -O3 # use this for optimization instead of the above
CCFLAGS          = -G -PIC +p
DEPENDOPT        = -xM1
CC               = CC $(OPTFLAGS) $(CCFLAGS)
LD               = CC -G
LDFINAL          = CC
CC4LIBS          = CC6
# could be g++
AR               = ar -rv
SODIRKEY =  -R
SONAMEKEY        = -R # change to -h if you need "immobile"dynamic libraries.
                      # LD_LIBRARY_PATH overrides -h
                      # Make sure there is no conflict

IMPORT   = NOT_USED
#LIBBASE     = /x/xx/lib/$(CC4LIBS)/g
LIBBASE  = ../libs
SLIBBASE    = $(LIBBASE)
ARNAME      = lib$(THISLIB).a
SONAME      = lib$(THISLIB).so
SODIR       = $(PWD)
SOFULL      = $(SODIR)/$(SONAME)
SOLABEL     = $(SONAMEKEY)$(SOFULL)
TESTNAME    = $(THISLIB)Test

STL         =  

# #########################################################
# standard variables that need modification in each project
# #########################################################

THISLIB = PartEvals
INCLUDES    = -I.. -I.  $(STL)

OBJS        = partEvalXFace.o netCutWBits.o netCutWNetVec.o\
        netTallies.o talliesWCosts.o talliesWConfigIds.o\
        netVec.o evalRegistry.o \
        netTallies2way.o talliesWCosts2way.o\
        talliesWCostsWWeights2way.o \
	bbox1dim.o strayNodes.o strayNodes2way.o univPartEval.o 

SRCS        =  $(OBJS:%.o=%.cxx)
LIBPATHS    = $(SODIRKEY)$(LIBBASE)

LIBS             =-L. $(SODIRKEY). -l$(THISLIB) $(LIBPATHS) -L$(LIBBASE)\
		   -lABKCommon -lCtainers\
                   -lCombi -lStats -lPlacement -lHGraph -lGeoms -lPartitioning

SLIBS            =-L$(SLIBBASE) -L$(PWD) -l$(THISLIB)\
                  -lPartitioning -lPlacement -lHGraph -lCombi -lStats -lGeoms\
                  -lCtainers -lABKCommon

PURIFY  = purify -cache_dir=cache -always_use_cache_dir\
	  -windows=no -log_file=purify.log

# ################
# standard targets
# ################

all:    lib
	ranlib *.a
	\cp -f *.a *.so ../libs

lib: $(ARNAME) $(SOFULL)

$(SOFULL):        $(OBJS)
	/bin/rm -f $(SOFULL)
	$(LD) $(INCLUDES) -o $(SOFULL) $(OBJS) $(SOLABEL)

$(ARNAME):        $(OBJS)
	/bin/rm -f $(ARNAME)
	$(AR) $(ARNAME) $(OBJS) $(RNGOBJS)    

.cxx.o:
	$(CC) $(INCLUDES) -c $*.cxx

.h.h.gch:
	$(CC) $(INCLUDES) $(ABKSYMBOLS) $*.h

.cxx.exe: lib
	/bin/rm -f $*.exe
	$(CC) $(INCLUDES) -c $*.cxx
	$(LDFINAL) -o $*.exe $*.o $(LIBS)

.cxx.exeS: lib
	/bin/rm -f $*.exeS
	$(CC) $(INCLUDES) -c $*.cxx
	$(LDFINAL) -o $*.exeS $*.o $(SLIBS)

.cxx.exeP: lib
	/bin/rm -f $*.exeP
	$(CC) $(INCLUDES) -c $*.cxx
	$(PURIFY) $(LDFINAL) -o $*.exeP $*.o $(LIBS)

depend:
	/bin/rm -f depend.mk

depend.mk:
	$(CC) $(DEPENDOPT) $(INCLUDES) $(SRCS) *.cxx >depend.mk

clean:        
	rm -rf *.o Templates.DB *.exe *.exe? $(SOFULL) $(ARNAME) core ir.out\
	lib*pure* cache SunWS_cache *notime new.out *.h.gch

test:  PartEvalsTest0.exe   PartEvalsTest1.exe  PartEvalsTest2.exe \
       PartEvalsTest3.exe   PartEvalsTest4.exe  PartEvalsTest6.exe\
       PartEvalsTest17.exe PartEvalsTest18.exe PartEvalsTest19.exe PartEvalsTest20.exe
testP: PartEvalsTest0.exeP  PartEvalsTest1.exeP
testS: PartEvalsTest0.exeS  PartEvalsTest1.exeS

# Uncomment for automatic handling of dependencies
# (use make depend to [re-]build dependencies)

# include depend.mk
