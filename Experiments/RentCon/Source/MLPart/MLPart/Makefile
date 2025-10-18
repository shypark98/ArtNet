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

.SUFFIXES:        .o .cxx .c .exeS .exeSol .exeP .exe .h.gch

# ##################
# standard variables
# ##################

 OPTFLAGS        = -g -DABKDEBUG 
#OPTFLAGS        = -O3 # use this for optimization instead of the above
CCFLAGS          = -G -PIC
DEPENDOPT        = -xM1
CC               = CC $(OPTFLAGS) $(CCFLAGS)
C                = CC
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

THISLIB = MLPart
INCLUDES    = -I. -I..
							
OBJS             = mlParams.o baseMLPart.o doML.o mlPart.o callPartitioner.o

SRCS        =  $(OBJS:%.o=%.cxx)

LIBPATHS    = $(SODIRKEY)$(LIBBASE)

LIBS             =-L$(LIBBASE) -L. $(SODIRKEY). $(LIBPATHS) -l$(THISLIB) -lABKCommon \
									-lPartitioning -lFilledHier -lPartEvals \
									-lHGraph -lClusteredHGraph -lGenHier \
									-lPartitioners -lPartLegality -lFMPart \
									-lStats -lCombi -lPlacement -lSmallPart \
									-lCtainers -lGeoms

SLIBS            =-L$(SLIBBASE) -l$(THISLIB) \
                  -lFMPart -lSmallPart -lPartitioners -lPartEvals\
                  -lPartLegality -lClusteredHGraph -lFilledHier -lGenHier\
                  -lPartitioning -lHGraph -lGeoms -lPlacement -lStats\
                  -lCtainers -lCombi -lABKCommon

PURIFY  = purify -cache_dir=cache -always_use_cache_dir\
	  -windows=no -log_file=purify.log

# ################
# standard targets
# ################

all:    lib

lib: $(ARNAME) $(SOFULL)
	ranlib *.a
	\cp -f *.a *.so ../libs

$(SOFULL):        $(OBJS)
	/bin/rm -f $(SOFULL)
	$(LD) $(INCLUDES) -o $(SOFULL) $(OBJS) $(SOLABEL)

$(ARNAME):        $(OBJS)
	/bin/rm -f $(ARNAME)
	$(AR) $(ARNAME) $(OBJS) $(RNGOBJS)    

.cxx.o:
	$(CC) $(INCLUDES) -c $*.cxx

.c.o:
	$(C) $(INCLUDES) -c $*.c

.h.h.gch:
	$(CC) $(INCLUDES) $(ABKSYMBOLS) $*.h

%.exe: %.cxx %.o
	/bin/rm -f $*.exe
	$(LDFINAL) -o $*.exe $*.o $(LIBS)

%.exeS: %.cxx %.o
	/bin/rm -f $*.exeS
	$(LDFINAL) -o $*.exeS $*.o -static $(SLIBS)

%.exeSPseudo: %.cxx %.o
	/bin/rm -f $*.exeSPseudo
	$(LDFINAL) -o $*.exeSPseudo $*.o $(SLIBS)

%.exeSol: %.cxx %.o dlstubs.o
	/bin/rm -f $*.exeSol
	$(LDFINAL) -o $*.exeSol dlstubs.o $*.o -static $(SLIBS)

%.exeSolPseudo: %.cxx %.o
	/bin/rm -f $*.exeSolPseudo
	$(LDFINAL) -o $*.exeSolPseudo $*.o $(SLIBS)

%.exeP: %.cxx %.o
	/bin/rm -f $*.exeP
	$(PURIFY) $(LDFINAL) -o $*.exeP $*.o $(LIBS)

depend:
	/bin/rm -f depend.mk

depend.mk:
	$(CC) $(DEPENDOPT) $(INCLUDES) $(SRCS) *.cxx >depend.mk

clean:        
	rm -rf *.o Templates.DB *.exe* $(SOFULL) $(ARNAME) core ir.out\
	lib*pure* cache SunWS_cache *notime new.out *.h.gch

test:  MLPartTest0.exe  MLPartTest1.exe  MLPartTest2.exe  MLPartTest6.exe\
       MLPartTest7.exe
testP: MLPartTest0.exeP  MLPartTest1.exeP  MLPartTest2.exeP  MLPartTest6.exeP\
       MLPartTest7.exeP
testSol: MLPartTest0.exeSol MLPartTest1.exeSol MLPartTest2.exeSol\
         MLPartTest6.exeSol MLPartTest7.exeSol
testSolPseudo: MLPartTest0.exeSolPseudo MLPartTest1.exeSolPseudo MLPartTest2.exeSolPseudo\
               MLPartTest6.exeSolPseudo MLPartTest7.exeSolPseudo
testS: MLPartTest0.exeS  MLPartTest1.exeS  MLPartTest2.exeS  MLPartTest6.exeS\
       MLPartTest7.exeS
testSPseudo: MLPartTest0.exeSPseudo  MLPartTest1.exeSPseudo  MLPartTest2.exeSPseudo\
             MLPartTest6.exeSPseudo  MLPartTest7.exeSPseudo

# Uncomment for automatic handling of dependencies
# (use make depend to [re-]build dependencies)

# include depend.mk
