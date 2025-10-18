#/**************************************************************************
#***    
#*** Copyright (c) 2008 Regents of the University of California,
#***               Andrew B. Kahng, Kwangok Jeong and Hailong Yao
#***
#***  Contact author(s): abk@cs.ucsd.edu, kjeong@vlsicad.ucsd.edu, hailong@cs.ucsd.edu
#***  Original Affiliation:   UCSD, Computer Science and Engineering Department,
#***                          La Jolla, CA 92093-0404 USA
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

###########################
# Created by Hailong Yao  #
# hailong@cs.ucsd.edu     #
# Date: Jul. 30, 2008     #
###########################

SUFFIX  = cpp
GCC = g++

LIBROOT=/home/memzfs_projects/artnet/d3yoon/ArtNet/Exps/RentCon/Source

GSLROOT = ../GSLSource
#GSLLIB = $(LIBROOT)/gsl/lib/libgsl.a
GSLLIB = -lgsl -lgslcblas -lm

DEFROOT = ../Def/def
LEFROOT = $(LIBROOT)/Lef/lef

SODIRKEY= -Wl,-rpath,

MLPARTROOT = $(LIBROOT)/MLPart
MLPartCFLAG = -DABKDEBUG -pedantic -pipe -I$(MLPARTROOT)

MLPartLFLAG = -L$(MLPARTROOT)/libs $(SODIRKEY)$(MLPARTROOT)/libs -lMLPart -lABKCommon -lPartitioning \
							-lFilledHier -lPartEvals -lHGraph -lClusteredHGraph -lGenHier \
							-lPartitioners -lPartLegality -lFMPart -lStats -lCombi -lPlacement \
							-lSmallPart -lCtainers -lGeoms -lMLPart

IFLAG = -I. -I$(DEFROOT) -I$(LEFROOT) -I$(GSLROOT) 
LFLAG = -fPIC -lm -lpthread -ldl $(SODIRKEY)/lib -L$(DEFROOT) -ldef -L$(LEFROOT) -llef $(GSLLIB) $(MLPartLFLAG)

BINPATH = ../bin
OBJPATH = .

SOURCES = $(wildcard *.$(SUFFIX))

OBJECTS = $(SOURCES:%.$(SUFFIX)=$(OBJPATH)/%.o)

HEADERS = $(wildcard ./*.h) 

#DISABLEASSERT="-DNDEBUG"
CFLAGS = $(IFLAG) -c -fPIC $(DEBUG) -w $(DBG) -Wunused $(DISABLEASSERT) $(MLPartCFLAG) #-Wno-deprecated

TARGET = $(BINPATH)/RentCon.exe

$(TARGET):$(OBJECTS) $(HEADERS)
	@echo "Now Generating $(TARGET)"
	$(PY) $(GCC) $(IFLAG) -o $(TARGET) $(OBJECTS) $(LFLAG)
	@echo "Done!"

# Compile the application code
$(OBJPATH)/%.o: %.$(SUFFIX) $(HEADERS)
	@echo "Now Compile $< ..."
	$(PY) $(GCC) -o $@ $(CFLAGS) $(DBG) $<
	@echo "Done!"
.KEEP_STATE:
clean:
	@echo "Now Removing ..."
	@-rm -rf $(OBJECTS)
	@-rm -rf ../obj/*.o
	@-rm -rf $(BINPATH)/$(TARGET)
	@echo "Done!"
debug:
	@$(MAKE) -f makefile DBG="-g"
explain:
	@echo "The following information represents your program:"
	@echo "Final executable name: $(TARGET)"
	@echo "Source files: $(SOURCES)"
	@echo "Object files: $(OBJECTS)"
