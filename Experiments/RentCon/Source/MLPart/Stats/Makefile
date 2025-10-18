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

.SUFFIXES:        .o .cxx  .exe .exeP .exeS .h.gch

# ################
# Project specific variables
# ################

RNG		= RNG
RNGOBJS		= $(RNG)/hist.o $(RNG)/newran.o $(RNG)/extreal.o $(RNG)/myexcept.o
MAKE		= make

# ##################
# standard variables
# ##################

 OPTFLAGS        = -g -DABKDEBUG 
#OPTFLAGS        = -O3 # use this for optimization instead of the above
CCFLAGS          = -G -PIC +p # comment +w2 when compiling with optimization
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
# -I$(IMPORT)/STL/CC

# #########################################################
# standard variables that need modification in each project
# #########################################################

THISLIB = Stats
INCLUDES= -I. -I..  $(STL)
OBJS    = trivStats.o freqDistr.o rancor.o linRegre.o loadedDie.o keyCounter.o\
	  expMins.o multiRegre.o
SRCS        =  $(OBJS:%.o=%.cxx)
LIBPATHS    = $(SODIRKEY)$(LIBBASE)
LIBS        = -L. $(SODIRKEY). -l$(THISLIB) -L$(LIBBASE) $(LIBPATHS)\
	      -lABKCommon -lCombi
SLIBS       = $(ARNAME)      -L$(SLIBBASE) -lABKCommon -lCombi

PURIFY  = purify -cache_dir=cache -always_use_cache_dir\
	  -windows=no -log_file=purify.log

# ################
# standard targets
# ################

all:       $(RNG)/libRand.a lib

$(RNG)/libRand.a:
	cd $(RNG); $(MAKE) PWD=$(PWD)/$(RNG) lib

lib: $(ARNAME) $(SOFULL)
	ranlib *.a
	\cp -f *.a ../libs

$(SOFULL):        $(OBJS) $(RNG)/libRand.a
	/bin/rm -f $(SOFULL)
	$(LD) $(INCLUDES) -o $(SOFULL) $(OBJS) $(SOLABEL)
	\cp -f *.so ../libs

$(ARNAME):        $(OBJS) $(RNG)/libRand.a
	/bin/rm -f $(ARNAME)
	$(AR) $(ARNAME) $(OBJS) $(RNGOBJS)    

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
	$(CC) $(DEPENDOPT) $(INCLUDES) $(SRCS) >depend.mk

clean:        
	rm -rf *.o Templates.DB seeds.{in,out} *.exe* *.diff *.xy \
        $(SOFULL) $(ARNAME) libRand.* core ir.out *.notime depend.mk\
        cache SunWS_cache *~ *.h.gch RNG/*~; \
	cd RNG; $(MAKE) PWD=$(PWD)/$(RNG) clean

test:  StatsTest0.exe StatsTest1.exe StatsTest2.exe StatsTest3.exe\
       StatsTest4.exe StatsTest5.exe StatsTest6.exe StatsTest7.exe\
       StatsTest8.exe StatsTest9.exe StatsTest10.exe

testP: StatsTest0.exeP
testS: StatsTest0.exeS

# Uncomment for automatic handling of dependencies
# (use make depend to [re-]build dependencies)

# include depend.mk
