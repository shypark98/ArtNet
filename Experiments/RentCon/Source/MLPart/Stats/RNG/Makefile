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


#/**************************************************************************
#***    
#*** Copyright (c) 1995-2000 Regents of the University of California,
#***               Andrew E. Caldwell, Andrew B. Kahng and Igor L. Markov
#*** Copyright (c) 2000-2004 Regents of the University of Michigan,
#***               Saurabh N. Adya, Jarrod A. Roy and Igor L. Markov
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

# 000430 Created by Sachin Bansal,  VLSICAD ABKGroup, UCLA/CS

.SUFFIXES:       .a .o .cxx .c .cpp


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
CC4LIBS          = CC5
# could be g++
AR               = ar cr
SODIRKEY =  -R
SONAMEKEY        = -R # change to -h if you need "immobile"dynamic libraries.
                      # LD_LIBRARY_PATH overrides -h
                      # Make sure there is no conflict

IMPORT   = NOT_USED
#LIBBASE     = /x/xx/lib/$(CC4LIBS)/g
LIBBASE  = ../../libs
SLIBBASE    = $(LIBBASE)
ARNAME      = lib$(THISLIB).a
SONAME      = lib$(THISLIB).so
SODIR       = $(PWD)
SOFULL      = $(SODIR)/$(SONAME)
SOLABEL     = $(SONAMEKEY)$(SOFULL)
TESTNAME    = $(THISLIB)Test

STL         = 
# -I$(IMPORT)/STL/CC
COMMON      = ../../ABKCommon

# #########################################################
# standard variables that need modification in each project
# #########################################################

THISLIB = Rand
INCLUDES= -I$(COMMON) 
OBJS    =  hist.o newran.o extreal.o myexcept.o
SRCS                =  $(OBJS:%.o=%.cpp)
LIBPATHS    = $(SODIRKEY)$(LIBBASE)
LIBS        = -L. -l$(THISLIB) $(LIBPATHS) -L$(LIBBASE)  -lcommon 
SLIBS       = $(ARNAME)                    -L$(SLIBBASE) -lcommon 

PURIFY  = purify -cache_dir=cache -always_use_cache_dir\
	  -windows=no -log_file=purify.log

# ################
# pakcage specific variables
# ################

OBJ_TR = tryrand.o tryrand1.o tryrand2.o tryrand3.o tryrand4.o hist.o \
      newran.o extreal.o myexcept.o

newranxx = include.h newran.h boolean.h myexcept.h


# ################
# standard targets
# ################

all:        lib

lib: $(SOFULL) $(ARNAME)
	\cp -f *.a *.so ../../libs

$(SOFULL):        $(OBJS)
	/bin/rm -f $(SOFULL)
	$(LD) $(INCLUDES) -o $(SOFULL) $(OBJS) $(SOLABEL)

$(ARNAME):        $(OBJS)
	/bin/rm -f $(ARNAME)
	$(AR) $(ARNAME) $(OBJS)     

.cxx.o:
	$(CC) $(INCLUDES) -c $*.cxx

depend:
	/bin/rm -f depend.mk

depend.mk:
	$(CC) $(DEPENDOPT) $(INCLUDES) $(SRCS) >depend.mk

clean:        
	rm -rf *.o Templates.DB $(TESTNAME)* $(SOFULL) $(SONAME) $(ARNAME)\
	core ir.out lib*pure* plot?.xy cache *.a tryrand SunWS_cache

test:  test0   test1  test2  test3   test4 test5 test6 test8
testP: testP0  testP1 testP2 testP3 testP4 testP5  # tests with purify
testS: testS0  testS1 testS2 testS3 testS4 testS5  # static tests 

tryrand: $(OBJ_TR) $(newranxx)
	$(LDFINAL) -o $@ $(OBJ_TR) -L. -lm

test0: main0.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)0
	$(LDFINAL) -o $(TESTNAME)0 main0.o  $(LIBS) 

test1: main1.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)1
	$(LDFINAL) -o $(TESTNAME)1 main1.o  $(LIBS)

test2: main2.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)2
	$(LDFINAL) -o $(TESTNAME)2 main2.o  $(LIBS)

test3: main3.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)3
	$(LDFINAL) -o $(TESTNAME)3 main3.o  $(LIBS)

test4: main4.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)4
	$(LDFINAL) -o $(TESTNAME)4 main4.o  $(LIBS)

test5: main5.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)5
	$(LDFINAL) -o $(TESTNAME)5 main5.o  $(LIBS)

test6: main6.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)6
	$(LDFINAL) -o $(TESTNAME)6 main6.o  $(LIBS)

test7: main7.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)7
	$(LDFINAL) -o $(TESTNAME)7 main7.o  $(LIBS)

test8: main8.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)8
	$(LDFINAL) -o $(TESTNAME)8 main8.o  $(LIBS)

test9: main9.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)9
	$(LDFINAL) -o $(TESTNAME)9 main9.o  $(LIBS)

testP0: main0.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)0 $(SODIR)/$(SONAME)
	$(PURIFY) $(LDFINAL) -o $(TESTNAME)0 main0.o  $(LIBS)

testP8: main8.o $(SOFULL)
	/bin/rm -rf $(TESTNAME)8 $(SODIR)/$(SONAME)
	$(PURIFY) $(LDFINAL) -o $(TESTNAME)8 main8.o  $(LIBS)

testS0: main0.o $(ARNAME)
	/bin/rm -rf $(TESTNAME)0
	$(LDFINAL) -o $(TESTNAME)0 main0.o  $(SLIBS)

testS10: main10.o $(ARNAME)
	/bin/rm -rf $(TESTNAME)10
	$(LDFINAL) -o $(TESTNAME)10 main10.o  $(LIBS)

# Uncomment for automatic handling of dependencies
# (use make depend to [re-]build dependencies)

# include depend.mk
