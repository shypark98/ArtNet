/**************************************************************************
***    
*** Copyright (c) 2008 Regents of the University of California,
***               Andrew B. Kahng, Kwangok Jeong and Hailong Yao
***
***  Contact author(s): abk@cs.ucsd.edu, kjeong@vlsicad.ucsd.edu, hailong@cs.ucsd.edu
***  Original Affiliation:   UCSD, Computer Science and Engineering Department,
***                          La Jolla, CA 92093-0404 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining 
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation 
***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
***  and/or sell copies of the Software, and to permit persons to whom the 
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/

/***************************
*** Created by Hailong Yao
*** hailong@cs.ucsd.edu
*** Date: Jul. 30, 2008
***************************/

#include <stdio.h>
#include <string.h>
// Mateus@180515
#include <iostream>
//--------------
#include <malloc.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <limits.h>

#include "LDIO.h"
#include "Design.h"
#include "typedefs.h"
#include "Pad.h"
#include "Pin.h"
#include "CellMaster.h"

#include "lefrReader.hpp"
#include "lefwWriter.hpp"
#include "lefiDebug.hpp"
#include "lefiUtil.hpp"
#include "defrReader.hpp"
#include "defiAlias.hpp"
#include "ABKCommon/abkcommon.h"
#include "ABKCommon/abkassert.h"

using namespace std;

namespace DESIGN {
	int numObjs;
	int isSumSet;      // to keep track if within SUM
	int isProp = 0;    // for PROPERTYDEFINITIONS
	int begOperand;    // to keep track for constraint, to print - as the 1st char
	static double curVer = 0;
	static int setSNetWireCbk = 0;
	
	//DEF functions
	void LEFCheckType(lefrCallbackType_e c) {
	  if (c >= 0 && c <= lefrLibraryEndCbkType) {
	    // OK
	  }
	  else
	  {
	    fprintf(stdout, "ERROR: callback type is out of bounds!\n");
	  }
	}
	
	int LEFMacroBeginCB(lefrCallbackType_e c, const char* macroName, lefiUserData ud) {
	  LEFCheckType(c);
	  Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
			fprintf(stdout, "MACRO %s\n",  macroName);
		}
	  
	  //here is a new cellMaster
	  CellMaster * cm = new CellMaster(macroName);
	  design->addCellMaster(cm);
	  
	  return 0;
	}
	
	int LEFMacroClassTypeCB(lefrCallbackType_e c, const char* macroClassType, lefiUserData ud) {
	  LEFCheckType(c);
	  Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "MACRO CLASS %s\n",  macroClassType);
		}
		
	  return 0;
	}
	
	int LEFMacroOriginCB(lefrCallbackType_e c, lefiNum macroNum, lefiUserData ud) {
	  LEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
			fprintf(stdout, "  ORIGIN ( %g %g ) ;\n", macroNum.x, macroNum.y);
		}
		CellMaster *cm = design->getCurrentCellMaster();
    if (cm == NULL)
    {
    	fprintf(stdout, "\nError: no cell master read in the LEF file.\n");
      exit(-1);
    }
		
		cm->setOriginXY(macroNum.x, macroNum.y);
	  return 0;
	}
	
	int LEFMacroSizeCB(lefrCallbackType_e c, lefiNum macroNum, lefiUserData ud) {
	  LEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
	  	fprintf(stdout, "  SIZE %g BY %g ;\n", macroNum.x, macroNum.y);
	  }

		CellMaster *cm = design->getCurrentCellMaster();
    if (cm == NULL)
    {
    	fprintf(stdout, "\nError: no cell master read in the LEF file.\n");
      exit(-1);
    }
		cm->setWidth(macroNum.x);
		cm->setHeight(macroNum.y);
	  
	  return 0;
	}
	
	void LEFGetGeometryCenter(lefiGeometries* geometry, double &centerX, double &centerY)
	{
	  int                   numItems = geometry->lefiGeometries::numItems();
	  int                   i, j;
	  lefiGeomPath*         path;
	  lefiGeomPathIter*     pathIter;
	  lefiGeomRect*         rect;
	  lefiGeomRectIter*     rectIter;
	  lefiGeomPolygon*      polygon;
	  lefiGeomPolygonIter*  polygonIter;
	  lefiGeomVia*          via;
	  lefiGeomViaIter*      viaIter;
	  double lX = 1e12, rX = -1, bY = 1e12, tY = -1;
	  
	  for (i = 0; i < numItems; i++) {
	     switch (geometry->lefiGeometries::itemType(i)) {
	        case  lefiGeomClassE:
	             //fprintf(stdout, "CLASS %s ", geometry->lefiGeometries::getClass(i));
	             break;
	        case lefiGeomLayerE:
	             //fprintf(stdout, "      LAYER %s ;\n", geometry->lefiGeometries::getLayer(i));
	             break;
	        case lefiGeomLayerExceptPgNetE:
	             //fprintf(stdout, "      EXCEPTPGNET ;\n");
	             break;
	        case lefiGeomLayerMinSpacingE:
	             //fprintf(stdout, "      SPACING %g ;\n", geometry->lefiGeometries::getLayerMinSpacing(i));
	             break;
	        case lefiGeomLayerRuleWidthE:
	             //fprintf(stdout, "      DESIGNRULEWIDTH %g ;\n", geometry->lefiGeometries::getLayerRuleWidth(i));
	             break;
	        case lefiGeomWidthE:
	             //fprintf(stdout, "      WIDTH %g ;\n", geometry->lefiGeometries::getWidth(i));
	             break;
	        case lefiGeomPathE:
	             path = geometry->lefiGeometries::getPath(i);
	             //fprintf(stdout, "      PATH ");
					    if (path == NULL)
					    {
					    	fprintf(stdout, "\nError in reading PATH in the LEF file.\n");
					      exit(-1);
					    }
	             for (j = 0; j < path->numPoints; j++)
	             {
	             		if (lX > path->x[j])
	             			lX = path->x[j];
	             		if (rX < path->x[j])
	             			rX = path->x[j];
	             		if (bY > path->y[j])
	             			bY = path->y[j];
	             		if (tY < path->y[j])
	             			tY = path->y[j];
	             		
//	                if (j+1 == path->numPoints) // last one on the list
	                   //fprintf(stdout, "      ( %g %g ) ;\n", path->x[j], path->y[j]);
//	                else
	                   //fprintf(stdout, "      ( %g %g )\n", path->x[j], path->y[j]);
	             }
	             break;
	        case lefiGeomPathIterE:
	             pathIter = geometry->lefiGeometries::getPathIter(i);
	             //fprintf(stdout, "      PATH ITERATED ");
						    if (pathIter == NULL)
						    {
						    	fprintf(stdout, "\nError in reading PATH in the LEF file.\n");
						      exit(-1);
						    }
	             for (j = 0; j < pathIter->numPoints; j++)
	             {
	             		if (lX > pathIter->x[j])
	             			lX = pathIter->x[j];
	             		if (rX < pathIter->x[j])
	             			rX = pathIter->x[j];
	             		if (bY > pathIter->y[j])
	             			bY = pathIter->y[j];
	             		if (tY < pathIter->y[j])
	             			tY = pathIter->y[j];
	             		
	                //fprintf(stdout, "      ( %g %g )\n", pathIter->x[j], pathIter->y[j]);
	             }
	             //fprintf(stdout, "      DO %g BY %g STEP %g %g ;\n", pathIter->xStart, pathIter->yStart, pathIter->xStep, pathIter->yStep);
	             break;
	        case lefiGeomRectE:
	             rect = geometry->lefiGeometries::getRect(i);
						    if (rect == NULL)
						    {
						    	fprintf(stdout, "\nError in reading Geometry Rect in the LEF file.\n");
						      exit(-1);
						    }
							if (lX > rect->xl)
								lX = rect->xl;
							if (rX < rect->xh)
								rX = rect->xh;
							if (bY > rect->yl)
								bY = rect->yl;
							if (tY < rect->yh)
								tY = rect->yh;
	             	
	             //fprintf(stdout, "      RECT ( %f %f ) ( %f %f ) ;\n", rect->xl, rect->yl, rect->xh, rect->yh);
	             break;
	        case lefiGeomRectIterE:
	             rectIter = geometry->lefiGeometries::getRectIter(i);
						    if (rectIter == NULL)
						    {
						    	fprintf(stdout, "\nError in reading Geometry Rect in the LEF file.\n");
						      exit(-1);
						    }
							if (lX > rectIter->xl)
								lX = rectIter->xl;
							if (rX < rectIter->xh)
								rX = rectIter->xh;
							if (bY > rectIter->yl)
								bY = rectIter->yl;
							if (tY < rectIter->yh)
								tY = rectIter->yh;
	             	
	             //fprintf(stdout, "      RECT ITERATE ( %f %f ) ( %f %f )\n", rectIter->xl, rectIter->yl, rectIter->xh, rectIter->yh);
	             //fprintf(stdout, "      DO %g BY %g STEP %g %g ;\n", rectIter->xStart, rectIter->yStart, rectIter->xStep, rectIter->yStep);
	             break;
	        case lefiGeomPolygonE:
	             polygon = geometry->lefiGeometries::getPolygon(i);
	             //fprintf(stdout, "      POLYGON ");
						    if (polygon == NULL)
						    {
						    	fprintf(stdout, "\nError in reading Geometry Polygon in the LEF file.\n");
						      exit(-1);
						    }
	             for (j = 0; j < polygon->numPoints; j++)
	             {
									if (lX > polygon->x[j])
										lX = polygon->x[j];
									if (rX < polygon->x[j])
										rX = polygon->x[j];
									if (bY > polygon->y[j])
										bY = polygon->y[j];
									if (tY < polygon->y[j])
										tY = polygon->y[j];
	             	
//	                if (j+1 == polygon->numPoints) // last one on the list
	                   //fprintf(stdout, "      ( %g %g ) ;\n", polygon->x[j], polygon->y[j]);
//	                else
	                   //fprintf(stdout, "      ( %g %g )\n", polygon->x[j], polygon->y[j]);
	             }
	             break;
	        case lefiGeomPolygonIterE:
	             polygonIter = geometry->lefiGeometries::getPolygonIter(i);
						    if (polygonIter == NULL)
						    {
						    	fprintf(stdout, "\nError in reading Geometry Polygon in the LEF file.\n");
						      exit(-1);
						    }
	             //fprintf(stdout, "      POLYGON ITERATE");
	             for (j = 0; j < polygonIter->numPoints; j++)
	             {
										if (lX > polygonIter->x[j])
											lX = polygonIter->x[j];
										if (rX < polygonIter->x[j])
											rX = polygonIter->x[j];
										if (bY > polygonIter->y[j])
											bY = polygonIter->y[j];
										if (tY < polygonIter->y[j])
											tY = polygonIter->y[j];
	                   //fprintf(stdout, "      ( %g %g )\n", polygonIter->x[j], polygonIter->y[j]);
	             }
	             //fprintf(stdout, "      DO %g BY %g STEP %g %g ;\n",
//	                     polygonIter->xStart, polygonIter->yStart,
//	                     polygonIter->xStep, polygonIter->yStep);
	             break;
	        case lefiGeomViaE:
	             via = geometry->lefiGeometries::getVia(i);
						    if (via == NULL)
						    {
						    	fprintf(stdout, "\nError in reading Via in the LEF file.\n");
						      exit(-1);
						    }
							if (lX > via->x)
								lX = via->x;
							if (rX < via->x)
								rX = via->x;
							if (bY > via->y)
								bY = via->y;
							if (tY < via->y)
								tY = via->y;
//	             fprintf(stdout, "      VIA ( %g %g ) %s ;\n", via->x, via->y, via->name);
	             break;
	        case lefiGeomViaIterE:
	             viaIter = geometry->lefiGeometries::getViaIter(i);
						    if (viaIter == NULL)
						    {
						    	fprintf(stdout, "\nError in reading Via in the LEF file.\n");
						      exit(-1);
						    }
								if (lX > viaIter->x)
									lX = viaIter->x;
								if (rX < viaIter->x)
									rX = viaIter->x;
								if (bY > viaIter->y)
									bY = viaIter->y;
								if (tY < viaIter->y)
									tY = viaIter->y;

//	             fprintf(stdout, "      VIA ITERATE ( %g %g ) %s\n", viaIter->x, viaIter->y, viaIter->name);
//	             fprintf(stdout, "      DO %g BY %g STEP %g %g ;\n", viaIter->xStart, viaIter->yStart, viaIter->xStep, viaIter->yStep);
	             break;
	        default:
//	             fprintf(stdout, "BOGUS geometries type.\n");
	             break;
	     }
	  }
	  if (lX > 1e10)
	  	lX = 0;
	  if (rX < 0)
	  	rX = 0;
	  if (bY > 1e10)
	  	bY = 0; 
	  if (tY < 0)
	  	tY = 0;
	  
	  centerX = (lX + rX)/2.0;
	  centerY = (bY + tY)/2.0;
	}
	
	int LEFPinCB(lefrCallbackType_e c, lefiPin* pin, lefiUserData ud)
	{
		LEFCheckType(c);
		double centerX = 0, centerY = 0;
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
			fprintf(stdout, "  PIN %s DIRECTION: %s;\n", pin->name(), pin->direction());
		}
		for (int i = 0; i < pin->numPorts(); ++ i)
		{
			lefiGeometries* g = pin->port(i);
			LEFGetGeometryCenter(g, centerX, centerY);
//			if (v.getForMajStats() > 2)
//			{
//				fprintf(stdout, "    Center(%f, %f)\n", centerX, centerY);
//			}
		}

		CellMaster *cm = design->getCurrentCellMaster();
    if (cm == NULL)
    {
    	fprintf(stdout, "\nError: no cell master read in the LEF file.\n");
      exit(-1);
    }
		
		PinType pinType;
		const char *dir = pin->direction();
		if (strcmp(dir, "INPUT") == 0)
		{
			pinType = INPUT;
		}
		else if (strcmp(dir, "OUTPUT") == 0)
		{
			pinType = OUTPUT;
		}
		else if (strcmp(dir, "INOUT") == 0)
		{
			pinType = INOUT;
		}
		else
		{
			pinType = UNKNOWN;
		}
		
		PinMaster * pm = new PinMaster(pin->name(), pinType, centerX, centerY);
		cm->addPinMaster(pm);
		
	  return 0;
	}

	char* LEFOrientStr(int orient) {
		switch (orient) {
		    case 0: return ((char*)"N");
		    case 1: return ((char*)"W");
		    case 2: return ((char*)"S");
		    case 3: return ((char*)"E");
		    case 4: return ((char*)"FN");
		    case 5: return ((char*)"FW");
		    case 6: return ((char*)"FS");
		    case 7: return ((char*)"FE");
		};
		return ((char*)"BOGUS");
	}

	int LEFMacroCB(lefrCallbackType_e c, lefiMacro* macro, lefiUserData ud) {
//	  lefiSitePattern* pattern;
//	  int              propNum, i;
			int hasPrtSym = 0;
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		
	  LEFCheckType(c);
	  if (macro->lefiMacro::hasClass())
			if (v.getForMajStats() > 2)
			{
				fprintf(stdout, "  CLASS %s ;\n", macro->lefiMacro::macroClass());
			}
	  if (macro->lefiMacro::hasEEQ())
			if (v.getForMajStats() > 2)
			{
				fprintf(stdout, "  EEQ %s ;\n", macro->lefiMacro::EEQ());
			}
	  if (macro->lefiMacro::hasLEQ())
			if (v.getForMajStats() > 2)
			{
	     fprintf(stdout, "  LEQ %s ;\n", macro->lefiMacro::LEQ());
	    }
	  if (macro->lefiMacro::hasSource())
			if (v.getForMajStats() > 2)
			{
	     fprintf(stdout, "  SOURCE %s ;\n", macro->lefiMacro::source());
	    }
	  if (macro->lefiMacro::hasXSymmetry()) {
			if (v.getForMajStats() > 2)
			{
	     fprintf(stdout, "  SYMMETRY X ");
	    }
	     hasPrtSym = 1;
	  }
	  if (macro->lefiMacro::hasYSymmetry()) {   // print X Y & R90 in one line
	     if (!hasPrtSym) {
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout, "  SYMMETRY Y ");
	      }
	        hasPrtSym = 1;
	     }
	     else
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout, "Y ");
	      }
	  }
	  if (macro->lefiMacro::has90Symmetry()) {
	     if (!hasPrtSym) {
					if (v.getForMajStats() > 2)
					{
		        fprintf(stdout, "  SYMMETRY R90 ");
		      }
	        hasPrtSym = 1;
	     }
	     else
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout, "R90 ");
	      }
	  }
	  if (hasPrtSym) {
				if (v.getForMajStats() > 2)
				{
		     fprintf(stdout, ";\n");
			  }
	     hasPrtSym = 0;
	  }
//	  if (macro->lefiMacro::hasSiteName())
//	     fprintf(stdout, "  SITE %s ;\n", macro->lefiMacro::siteName());
//	  if (macro->lefiMacro::hasSitePattern()) {
//	     for (i = 0; i < macro->lefiMacro::numSitePattern(); i++ ) {
//	       pattern = macro->lefiMacro::sitePattern(i);
//	       if (pattern->lefiSitePattern::hasStepPattern()) {
//	          fprintf(stdout, "  SITE %s %g %g %s DO %g BY %g STEP %g %g ;\n",
//	                pattern->lefiSitePattern::name(), pattern->lefiSitePattern::x(),
//	                pattern->lefiSitePattern::y(),
//	                LEFOrientStr(pattern->lefiSitePattern::orient()),
//	                pattern->lefiSitePattern::xStart(),
//	                pattern->lefiSitePattern::yStart(),
//	                pattern->lefiSitePattern::xStep(),
//	                pattern->lefiSitePattern::yStep());
//	       } else {
//	          fprintf(stdout, "  SITE %s %g %g %s ;\n",
//	                pattern->lefiSitePattern::name(), pattern->lefiSitePattern::x(),
//	                pattern->lefiSitePattern::y(),
//	                LEFOrientStr(pattern->lefiSitePattern::orient()));
//	       }
//	     }
//	  }
//	  if (macro->lefiMacro::hasSize())
//	     fprintf(stdout, "  SIZE %g BY %g ;\n", macro->lefiMacro::sizeX(),
//	             macro->lefiMacro::sizeY());
	
//	  if (macro->lefiMacro::hasForeign()) {
//	     for (i = 0; i < macro->lefiMacro::numForeigns(); i++) {
//	        fprintf(stdout, "  FOREIGN %s ", macro->lefiMacro::foreignName(i));
//	        if (macro->lefiMacro::hasForeignPoint(i)) {
//	           fprintf(stdout, "( %g %g ) ", macro->lefiMacro::foreignX(i),
//	                   macro->lefiMacro::foreignY(i));
//	           if (macro->lefiMacro::hasForeignOrient(i))
//	              fprintf(stdout, "%s ", macro->lefiMacro::foreignOrientStr(i));
//	        }
//	        fprintf(stdout, ";\n");
//	     }
//	  }
//	  if (macro->lefiMacro::hasOrigin())
//	     fprintf(stdout, "  ORIGIN ( %g %g ) ;\n", macro->lefiMacro::originX(),
//	             macro->lefiMacro::originY());
//	  if (macro->lefiMacro::hasPower())
//	     fprintf(stdout, "  POWER %g ;\n", macro->lefiMacro::power());
//	  propNum = macro->lefiMacro::numProperties();
//	  if (propNum > 0) {
//	     fprintf(stdout, "  PROPERTY ");
//	     for (i = 0; i < propNum; i++) {
//	        // value can either be a string or number
//	        if (macro->lefiMacro::propValue(i)) {
//	           fprintf(stdout, "%s %s ", macro->lefiMacro::propName(i),
//	                   macro->lefiMacro::propValue(i));
//	        }
//	        else
//	           fprintf(stdout, "%s %g ", macro->lefiMacro::propName(i),
//	                   macro->lefiMacro::propNum(i));
	
//	        switch (macro->lefiMacro::propType(i)) {
//	           case 'R': fprintf(stdout, "REAL ");
//	                     break;
//	           case 'I': fprintf(stdout, "INTEGER ");
//	                     break;
//	           case 'S': fprintf(stdout, "STRING ");
//	                     break;
//	           case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                     break;
//	           case 'N': fprintf(stdout, "NUMBER ");
//	                     break;
//	        } 
//	     }
//	     fprintf(stdout, ";\n");
//	  }
	  //fprintf(stdout, "END %s\n", macro->lefiMacro::name());
	  return 0;
	}
	
	int LEFMacroEndCB(lefrCallbackType_e c, const char* macroName, lefiUserData ud) {
	  LEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "END %s\n", macroName);
		}
	  return 0;
	}
	
	void* LEFMallocCB(int size) {
	  return malloc(size);
	}
	
	void* LEFReallocCB(void* name, int size) {
	  return realloc(name, size);
	}
	
	void LEFFreeCB(void* name) {
	  free(name);
	  return;
	}
	
	void LEFLineNumberCB(int lineNo) {
	  fprintf(stdout, "Parsed %d number of lines!!\n", lineNo);
	  
	  return;
	}
	
	void LDReader::readLef(Design *design)
	{
	  FILE* f;
		Verbosity & v = design->getVerb();
    //use the callback function
		lefrSetMacroBeginCbk(LEFMacroBeginCB);
		lefrSetMacroCbk(LEFMacroCB);
		lefrSetMacroClassTypeCbk(LEFMacroClassTypeCB);
		lefrSetMacroOriginCbk(LEFMacroOriginCB);
		lefrSetMacroSizeCbk(LEFMacroSizeCB);
		lefrSetMacroEndCbk(LEFMacroEndCB);
		lefrSetPinCbk(LEFPinCB);
		lefrSetUserData((void*)3);
		lefrSetMallocFunction(LEFMallocCB);
		lefrSetReallocFunction(LEFReallocCB);
		lefrSetFreeFunction(LEFFreeCB);
//    lefrSetRegisterUnusedCallbacks();
		lefrSetMacroWarnings(30);
		if (v.getForActions() > 0)
		{
			lefrSetLineNumberFunction(LEFLineNumberCB);
			lefrSetDeltaNumberLines(1000);
		}
		
		lefrInit();
		
		for (int i = 0; i < design->getLefFileNum(); ++ i)
		{
			if(v.getForSysRes() > 0)
			{
				MemUsage mu;
				
				double availMem = VMemUsage::getPhysTotal();
				availMem = min(availMem, static_cast<double>(1UL << (sizeof(void*)*8 - 20)));
				
				cout << "==================Mem info=================="<<endl
							<<"Current memory usage is " << mu.getEstimate() << "MB" << endl
							<< "Available physical memory is " << availMem << "MB" << endl
							<<"============================================"<<endl;
	    }
			string lefName = design->getLefName(i);
	    if ((f = fopen(lefName.c_str(),"r")) == 0)
	    {
	      fprintf(stderr, "Cannot open input file '%s'\n", lefName.c_str());
	      return;
	    }
	    if (v.getForActions() > 0)
	    {
		    fprintf(stdout, "\nStart to read in the lef file %s ...\n", lefName.c_str());
		  }

			lefrReset();
			
	    int res = lefrRead(f, lefName.c_str(), (void*)design);
	    if (res)
	    {
	    	fprintf(stderr, "Reader returns bad status.\n");
	    	exit(-1);
	    }
			
	    (void)lefrReleaseNResetMemory();
			
			fclose(f);
		}

    if (v.getForActions() > 0)
    {
    	CellMasterMap &CMM = design->getCellMasterMap();
    	for (CellMasterMapItr itr = CMM.begin(); itr != CMM.end(); ++ itr)
    	{
    		CellMaster * cm = (*itr).second;
    		cm->check();
    	}
	    fprintf(stdout, "Finished reading the lef files\n");
	  }
	}
	
	//DEF functions
	void DEFDataError() {
	  fprintf(stdout, "ERROR: returned user data is not correct!\n");
	}

	void DEFCheckType(defrCallbackType_e c) {
	  if (c >= 0 && c <= defrDesignEndCbkType) {
	    // OK
	  } else {
	    fprintf(stdout, "ERROR: callback type is out of bounds!\n");
	  }
	}

	int DEFDone(defrCallbackType_e c, void* dummy, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "END DESIGN\n");
		}
	  return 0;
	}

	int DEFEndfunc(defrCallbackType_e c, void* dummy, defiUserData ud) {
	  DEFCheckType(c);
	  return 0;
	}

	char* DEFOrientStr(int orient) {
	  switch (orient) {
	      case 0: return ((char*)"N");
	      case 1: return ((char*)"W");
	      case 2: return ((char*)"S");
	      case 3: return ((char*)"E");
	      case 4: return ((char*)"FN");
	      case 5: return ((char*)"FW");
	      case 6: return ((char*)"FS");
	      case 7: return ((char*)"FE");
	  };
	  return ((char*)"BOGUS");
	}

	int DEFCompf(defrCallbackType_e c, defiComponent* co, defiUserData ud)
	{
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		double lefDefFactor = design->getLefDefFactor();
		const char *gateName = co->defiComponent::id();
		const char *cellName = co->defiComponent::name();
		CellMaster *cm = design->getCellMaster(cellName);
    if (cm == NULL)
    {
    	fprintf(stdout, "\nError: no cell master read in the LEF file.\n");
      exit(-1);
    }
    PinMasterMap &pinMasterMap = cm->getPinMasterMap();
    int width = (int)(cm->getWidth()*lefDefFactor);
    int height = (int)(cm->getHeight()*lefDefFactor);
		if (v.getForMajStats() > 2)
		{
			cout << "Width: " << width << "\tHeight: " << height << endl;
		}
		int originX, originY;
		const char *orientStr;
	  static int gateIndex = 0;
		static int pinIndex = 0;
		
		Gate *g = new Gate(gateIndex++, gateName, cellName);
		
	  int i;
	//  missing GENERATE, FOREIGN
//	  fprintf(stdout, "- %s %s ", co->defiComponent::id(), co->defiComponent::name());
	  
//    co->defiComponent::changeIdAndName("idName", "modelName");
//    fprintf(stdout, "%s %s ", co->defiComponent::id(), co->defiComponent::name());
    
//	    if (co->defiComponent::hasNets()) {
//	        for (i = 0; i < co->defiComponent::numNets(); i++)
//	             fprintf(stdout, "%s ", co->defiComponent::net(i));
//	    }
	    
	    if (co->defiComponent::isFixed())
	    {
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout, "+ FIXED %d %d %s ",
	                co->defiComponent::placementX(),
	                co->defiComponent::placementY(),
	                //DEFOrientStr(co->defiComponent::placementOrient()));
	                co->defiComponent::placementOrientStr());
	      }
				originX = co->defiComponent::placementX();
				originY = co->defiComponent::placementY();
				orientStr = co->defiComponent::placementOrientStr();
			}
	    if (co->defiComponent::isCover())
	    {
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout, "+ COVER %d %d %s ",
	                co->defiComponent::placementX(),
	                co->defiComponent::placementY(),
	                DEFOrientStr(co->defiComponent::placementOrient()));
	      }
				originX = co->defiComponent::placementX();
				originY = co->defiComponent::placementY();
				orientStr = DEFOrientStr(co->defiComponent::placementOrient());
	    }
	    if (co->defiComponent::isPlaced())
	    {
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout,"+ PLACED %d %d %s ",
	                co->defiComponent::placementX(),
	                co->defiComponent::placementY(),
	                DEFOrientStr(co->defiComponent::placementOrient()));
	      }
				originX = co->defiComponent::placementX();
				originY = co->defiComponent::placementY();
				orientStr = DEFOrientStr(co->defiComponent::placementOrient());
	    }
	    if (co->defiComponent::isUnplaced()) {
				if (v.getForMajStats() > 2)
				{
	        fprintf(stdout,"+ UNPLACED ");
	        if ((co->defiComponent::placementX() != -1) ||
	            (co->defiComponent::placementY() != -1))
	           fprintf(stdout,"%d %d %s ",
	                   co->defiComponent::placementX(),
	                   co->defiComponent::placementY(),
	                   DEFOrientStr(co->defiComponent::placementOrient()));
	      }
				originX = co->defiComponent::placementX();
				originY = co->defiComponent::placementY();
				orientStr = DEFOrientStr(co->defiComponent::placementOrient());
	    }
//	    if (co->defiComponent::hasSource())
//	        fprintf(stdout, "+ SOURCE %s ", co->defiComponent::source);
//	    if (co->defiComponent::hasGenerate()) {
//	        fprintf(stdout, "+ GENERATE %s ", co->defiComponent::generateName());
//	        if (co->defiComponent::macroName() &&
//	            *(co->defiComponent::macroName()))
//	           fprintf(stdout, "%s ", co->defiComponent::macroName());
//	    }
//	    if (co->defiComponent::hasWeight())
//	        fprintf(stdout, "+ WEIGHT %d ", co->defiComponent::weight());
//	    if (co->defiComponent::hasEEQ())
//	        fprintf(stdout, "+ EEQMASTER %s ", co->defiComponent::EEQ());
//	    if (co->defiComponent::hasRegionName())
//	        fprintf(stdout, "+ REGION %s ", co->defiComponent::regionName());
//	    if (co->defiComponent::hasRegionBounds()) {
//	        int *xl, *yl, *xh, *yh;
//	        int size;
//	        co->defiComponent::regionBounds(&size, &xl, &yl, &xh, &yh);
//	        for (i = 0; i < size; i++) { 
//	            fprintf(stdout, "+ REGION %d %d %d %d \n",
//	                    xl[i], yl[i], xh[i], yh[i]);
//	        }
//	    }
//	    if (co->defiComponent::hasHalo()) {
//	        int left, bottom, right, top;
//	        (void) co->defiComponent::haloEdges(&left, &bottom, &right, &top);
//	        fprintf(stdout, "+ HALO ");
//	        if (co->defiComponent::hasHaloSoft())
//	           fprintf(stdout, "SOFT ");
//	        fprintf(stdout, "%d %d %d %d\n", left, bottom, right, top);
//	    }
//	    if (co->defiComponent::hasRouteHalo()) {
//	        fprintf(stdout, "+ ROUTEHALO %d %s %s\n", co->defiComponent::haloDist(),
//	                co->defiComponent::minLayer(), co->defiComponent::maxLayer());
//	    }
//	    if (co->defiComponent::hasForeignName()) {
//	        fprintf(stdout, "+ FOREIGN %s %d %d %s %d ",
//	                co->defiComponent::foreignName(), co->defiComponent::foreignX(),
//	                co->defiComponent::foreignY(), co->defiComponent::foreignOri(),
//	                co->defiComponent::foreignOrient());
//	    }
//	    if (co->defiComponent::numProps()) {
//	        for (i = 0; i < co->defiComponent::numProps(); i++) {
//	            fprintf(stdout, "+ PROPERTY %s %s ", co->defiComponent::propName(i),
//	                    co->defiComponent::propValue(i));
//	            switch (co->defiComponent::propType(i)) {
//	               case 'R': fprintf(stdout, "REAL ");
//	                         break;
//	               case 'I': fprintf(stdout, "INTEGER ");
//	                         break;
//	               case 'S': fprintf(stdout, "STRING ");
//	                         break;
//	               case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                         break;
//	               case 'N': fprintf(stdout, "NUMBER ");
//	                         break;
//	            }
//	        }
//	    }
//	    fprintf(stdout, ";\n");
//	    --numObjs;
//	    if (numObjs <= 0)
//	        fprintf(stdout, "END COMPONENTS\n");
				Chip &chip = design->getChip();
				int chipLeft = chip.getLX();
				int chipBottom = chip.getBY();
				int x, y;
				// Get and print the origin of the instance.
				if (strcmp(orientStr, "N") == 0)
				{
					x = originX - chipLeft;
					y = originY - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+pm->x()*lefDefFactor);
						int centerY = (int)(y+pm->y()*lefDefFactor);
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "S") == 0)
				{
					x = originX - width - chipLeft;
					y = originY - height - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+width-pm->x()*lefDefFactor);
						int centerY = (int)(y+height-pm->y()*lefDefFactor);
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
						
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "W") == 0)
				{
					int tmp = width;
					width = height;
					height = tmp;
					x = originX - width - chipLeft;
					y = originY - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+width-pm->y()*lefDefFactor);
						int centerY = (int)(y+pm->x()*lefDefFactor);
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
						
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "E") == 0)
				{
					int tmp = width;
					width = height;
					height = tmp;
					x = originX - chipLeft;
					y = originY - height - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+pm->y()*lefDefFactor);
						int centerY = (int)(y+height-pm->x()*lefDefFactor);
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
						
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "FN") == 0)
				{
					x = originX - width - chipLeft;
					y = originY - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+width-pm->x()*lefDefFactor);
						int centerY = (int)(y+pm->y()*lefDefFactor);
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
						
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "FS") == 0)
				{
					x = originX - chipLeft;
					y = originY - height - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+pm->x()*lefDefFactor);
						int centerY = (int)(y+height-pm->y()*lefDefFactor);
						
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "FW") == 0)
				{
					int tmp = width;
					width = height;
					height = tmp;
					x = originX - chipLeft;
					y = originY - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+pm->y()*lefDefFactor);
						int centerY = (int)(y+pm->x()*lefDefFactor);
												string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;

				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else if (strcmp(orientStr, "FE") == 0)
				{
					int tmp = width;
					width = height;
					height = tmp;
					x = originX - width - chipLeft;
					y = originY - height - chipBottom;
    			g->setCoord(x, y, width, height);
					for (PinMasterMapItr itr = pinMasterMap.begin(); itr != pinMasterMap.end(); ++ itr)
					{
						PinMaster *pm = (*itr).second;
						int centerX = (int)(x+width-pm->y()*lefDefFactor);
						int centerY = (int)(y+height-pm->x()*lefDefFactor);
						
						string pmName = pm->getName();
						if (pmName == "VDD" || pmName == "VSS")
							continue;
				    Pin *pin = new Pin(pinIndex ++, pm->getName(), pm->getType(), centerX, centerY);
		        pin->setGate(g);
		        g->addPin(pin);
					}
				}
				else
				{
					fprintf(stderr, "Error: unknown orient of cell instance: %s\n", orientStr);
					exit(-1);
				}
				design->addGate(g);
				
	  return 0;
	}

	int DEFNetpath(defrCallbackType_e c, defiNet* ppath, defiUserData ud) {
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "\n");
	  	fprintf (stdout, "Callback of partial path for net\n");
		}
	
	  return 0;
	}

	int DEFNetNamef(defrCallbackType_e c, const char* netName, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
	    fprintf(stdout, "- %s ", netName);
  	}
	  return 0;
	}

	int DEFSubnetNamef(defrCallbackType_e c, const char* subnetName, defiUserData ud) {
	  DEFCheckType(c);
	  
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
	    if (curVer >= 5.6)
	      fprintf(stdout, "   + SUBNET CBK %s ", subnetName);
	  }
	  return 0;
	}

	int DEFNondefRulef(defrCallbackType_e c, const char* ruleName, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
	    if (curVer >= 5.6)
	      fprintf(stdout, "   + NONDEFAULTRULE CBK %s ", ruleName);
	  }
	  return 0;
	}

//do not consider P/G nets
	int DEFNetf(defrCallbackType_e c, defiNet* net, defiUserData ud) {
	  // For net and special net.
	  int        i, j, k, x, y, z, count, newLayer;
	  defiPath*  p;
	  defiSubnet *s;
	  int        path;
	  defiVpin   *vpin;
	  // defiShield *noShield;
	  defiWire   *wire;
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		
	  DEFCheckType(c);
//	  if (c != defrNetCbkType)
//	      fprintf(stdout, "BOGUS NET TYPE  ");
//	  if (net->defiNet::pinIsMustJoin(0))
//	      fprintf(stdout, "- MUSTJOIN ");
	// 5/6/2004 - don'pin need since I have a callback for the name
	//  else
	//      fprintf(stdout, "- %s ", net->defiNet::name());
	 
	//  net->defiNet::changeNetName("newNetName");
	//  fprintf(stdout, "%s ", net->defiNet::name());
		static int netIndex = 0;
		const char *netName = net->defiNet::name();
		Net *n = new Net(netIndex, netName);
		
	  if (net->defiNet::hasUse())
	  {
			const char *usage = net->defiNet::use();
			if (strcmp(usage, "ANALOG") == 0)
			{
				n->setType(SIGNAL);
			}
			else if (strcmp(usage, "CLOCK") == 0)
			{
				n->setType(SIGNAL);
			}
			else if (strcmp(usage, "GROUND") == 0)
			{
//				n->setType(PG);
				delete n;
				return 0;
			}
			else if (strcmp(usage, "POWER") == 0)
			{
//				n->setType(PG);
				delete n;
				return 0;
			}
			else if (strcmp(usage, "RESET") == 0)
			{
				n->setType(SIGNAL);
			}
			else if (strcmp(usage, "SCAN") == 0)
			{
				n->setType(SIGNAL);
			}
			else if (strcmp(usage, "SIGNAL") == 0)
			{
				n->setType(SIGNAL);
			}
			else if (strcmp(usage, "TIEOFF") == 0)
			{
				n->setType(SIGNAL);
			}
			else
			{
				fprintf(stdout, "\nWarning: unknown net type %s of net %s. Assume it is SIGNAL.\n", usage, n->getName().c_str());
				n->setType(SIGNAL);
			}
	  }
	  else
	  {
//			fprintf(stdout, "\nWarning: net %s does not have a type. Assume it is SIGNAL.\n", n->getName().c_str());
			n->setType(SIGNAL);
	  }
		
		if (v.getForMajStats() > 2)
		{
			fprintf(stdout, "Net %s\n", netName);
		}
	  // compName & pinName
		StrVector &keyNames = design->getCellKeyNames();
		StrVector &fullNames = design->getCellFullNames();
	  for (i = 0; i < net->defiNet::numConnections(); i++)
	  {
	      const char *gateName = net->defiNet::instance(i);
	      const char *pinName = net->defiNet::pin(i);
				if (v.getForMajStats() > 2)
				{
		      fprintf(stdout, "Gate: %s, Pin: %s\n", gateName, pinName);
		    }
	      
	      if (strcmp(gateName, "PIN") == 0)
	      {
	      	abkassert(design->isPadName(pinName), "Error in DEFNetf()");
	      	Pad *pad = design->getPadByName(pinName);
		      if (pad == NULL)
			    {
			    	fprintf(stdout, "\nWarning: no pad named %s in the design.\n", pinName);
			    	continue;
			    }
	      	if (pad->getPadType() != PrimiaryOutput)
		      	n->addSourcePad(pad);
		      else
		      	n->addSinkPad(pad);
	      }
	      else
	      {
	      	abkassert(design->isGateName(gateName), "Error in DefNetf()");
		      Gate *gate = design->getGateByName(gateName);
		      if (gate == NULL)
			    {
			    	fprintf(stdout, "\nWarning: no cell instance named %s in the design.\n", gateName);
			    	continue;
			    }
		      string cellName = gate->getCellName();
//		      fprintf(stdout, "cell name: %s\n", cellName.c_str());
		      bool match = false;
		      for (int j = 0; j < keyNames.size(); ++ j)
		      {
		      	if (cellName.find(keyNames[j]) != string::npos)
		      	{
//		      		fprintf(stdout, "Cell instance %s -> cell name %s matched key %s\n", gateName, cellName.c_str(), keyNames[j].c_str());
		      		match = true;
		      		break;
		      	}
		      }
		      if (match)
		      	continue;
		      for (int j = 0; j < fullNames.size(); ++ j)
		      {
		      	if (cellName.compare(fullNames[j]) == 0)
		      	{
//		      		fprintf(stdout, "Cell instance %s -> cell name %s matched full name %s\n", gateName, cellName.c_str(), fullNames[j].c_str());
		      		match = true;
		      		break;
		      	}
		      }
		      if (match)
		      	continue;
		      
		      Pin *pin = gate->getPinByName(pinName);
		      if (pin == NULL)
			    {
			    	fprintf(stdout, "\nWarning: cell instance %s does not have a pin of name %s.\n", gateName, pinName);
			    	continue;
			    }
		      if (pin->getType() == OUTPUT)
		      {
		      	n->addSourcePin(pin);
		      }
		      else
		      {
		      	n->addSinkPin(pin);
		      }
		      gate->addNet(n);
		    }
	  }
	  
    if (n->getPinNum()+n->getPadNum() <= 1)
    {
    	fprintf(stdout, "\nWarning: net %s has %d pin and %d pad\n", n->getName().c_str(), n->getPinNum(), n->getPadNum());
    }
    
		netIndex++;
	  design->addNet(n);
		
//	  for (i = 0; i < net->defiNet::numVpins(); i++) {
//	      vpin = net->defiNet::vpin(i);
//	      fprintf(stdout, "  + %s", vpin->name());
//	      if (vpin->layer()) 
//	          fprintf(stdout, " %s", vpin->layer());
//	      fprintf(stdout, " %d %d %d %d", vpin->xl(), vpin->yl(), vpin->xh(),
//	              vpin->yh());
//	      if (vpin->status() != ' ') {
//	          fprintf(stdout, " %c", vpin->status());
//	          fprintf(stdout, " %d %d", vpin->xLoc(), vpin->yLoc());
//	          if (vpin->orient() != -1)
//	              fprintf(stdout, " %s", DEFOrientStr(vpin->orient()));
//	      }
//	      fprintf(stdout, "\n");
//	  }
	
//	  // regularWiring
//	  if (net->defiNet::numWires())
//	  {
//	     for (i = 0; i < net->defiNet::numWires(); i++) {
//	        newLayer = 0;
//	        wire = net->defiNet::wire(i);
//	        fprintf(stdout, "\n  + %s ", wire->wireType());
//	        count = 0;
//	        for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	           p = wire->defiWire::path(j);
//	           p->initTraverse();
//	           while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	              count++;
//	              // Don'pin want the line to be too long
//	              if (count >= 5) {
//	                  fprintf(stdout, "\n");
//	                  count = 0;
//	              } 
//	              switch (path) {
//	                case DEFIPATH_LAYER:
//	                     if (newLayer == 0) {
//	                         fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                         newLayer = 1;
//	                     } else
//	                         fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                     break;
//	                case DEFIPATH_VIA:
//	                     fprintf(stdout, "%s ", p->defiPath::getVia());
//	                     break;
//	                case DEFIPATH_VIAROTATION:
//	                     fprintf(stdout, "%s ", 
//	                             DEFOrientStr(p->defiPath::getViaRotation()));
//	                     break;
//	                case DEFIPATH_WIDTH:
//	                     fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                     break;
//	                case DEFIPATH_POINT:
//	                     p->defiPath::getPoint(&x, &y);
//	                     fprintf(stdout, "( %d %d ) ", x, y);
//	                     break;
//	                case DEFIPATH_FLUSHPOINT:
//	                     p->defiPath::getFlushPoint(&x, &y, &z);
//	                     fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                     break;
//	                case DEFIPATH_TAPER:
//	                     fprintf(stdout, "TAPER ");
//	                     break;
//	                case DEFIPATH_TAPERRULE:
//	                     fprintf(stdout, "TAPERRULE %s ",p->defiPath::getTaperRule());
//	                     break;
//	                case DEFIPATH_STYLE:
//	                     fprintf(stdout, "STYLE %d ",p->defiPath::getStyle());
//	                     break;
//	              }
//	           }
//	        }
//	        fprintf(stdout, "\n");
//	        count = 0;
//	     }
//	  }
	
//	  // SHIELDNET
//	  if (net->defiNet::numShieldNets())
//	  {
//	     for (i = 0; i < net->defiNet::numShieldNets(); i++) 
//	         fprintf(stdout, "\n  + SHIELDNET %s", net->defiNet::shieldNet(i));
//	  }
	/* obsolete in 5.4
	  if (net->defiNet::numNoShields()) {
	     for (i = 0; i < net->defiNet::numNoShields(); i++) { 
	         noShield = net->defiNet::noShield(i); 
	         fprintf(stdout, "\n  + NOSHIELD ");
	         newLayer = 0;
	         for (j = 0; j < noShield->defiShield::numPaths(); j++) {
	            p = noShield->defiShield::path(j);
	            p->initTraverse();
	            while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
	               count++;
	               // Don'pin want the line to be too long
	               if (count >= 5) {
	                   fprintf(stdout, "\n");
	                   count = 0;
	               }
	               switch (path) {
	                 case DEFIPATH_LAYER:
	                      if (newLayer == 0) {
	                          fprintf(stdout, "%s ", p->defiPath::getLayer());
	                          newLayer = 1;
	                      } else
	                          fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
	                      break;
	                 case DEFIPATH_VIA:
	                      fprintf(stdout, "%s ", p->defiPath::getVia());
	                      break;
	                 case DEFIPATH_VIAROTATION:
	                      fprintf(stdout, "%s ", 
	                             DEFOrientStr(p->defiPath::getViaRotation()));
	                      break;
	                 case DEFIPATH_WIDTH:
	                      fprintf(stdout, "%d ", p->defiPath::getWidth());
	                      break;
	                 case DEFIPATH_POINT:
	                      p->defiPath::getPoint(&x, &y);
	                      fprintf(stdout, "( %d %d ) ", x, y);
	                      break;
	                 case DEFIPATH_FLUSHPOINT:
	                      p->defiPath::getFlushPoint(&x, &y, &z);
	                      fprintf(stdout, "( %d %d %d ) ", x, y, z);
	                      break;
	                 case DEFIPATH_TAPER:
	                      fprintf(stdout, "TAPER ");
	                      break;
	                 case DEFIPATH_TAPERRULE:
	                      fprintf(stdout, "TAPERRULE %s ",
	                              p->defiPath::getTaperRule());
	                      break;
	               }
	            }
	         }
	     }
	  }
	*/
	
//	  if (net->defiNet::hasSubnets())
//	  {
//	     for (i = 0; i < net->defiNet::numSubnets(); i++) {
//	        s = net->defiNet::subnet(i);
//	        fprintf(stdout, "\n");
//	 
//	        if (s->defiSubnet::numConnections()) {
//	           if (s->defiSubnet::pinIsMustJoin(0))
//	              fprintf(stdout, "- MUSTJOIN ");
//	           else
//	              fprintf(stdout, "  + SUBNET %s ", s->defiSubnet::name());
//	           for (j = 0; j < s->defiSubnet::numConnections(); j++)
//	              fprintf(stdout, " ( %s %s )\n", s->defiSubnet::instance(j), s->defiSubnet::pin(j));
//	
//	           // regularWiring
//	           if (s->defiSubnet::numWires()) {
//	              for (k = 0; k < s->defiSubnet::numWires(); k++) {
//	                 newLayer = 0;
//	                 wire = s->defiSubnet::wire(k);
//	                 fprintf(stdout, "  %s ", wire->wireType());
//	                 count = 0;
//	                 for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	                    p = wire->defiWire::path(j);
//	                    p->initTraverse();
//	                    while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	                       count++;
//	                       // Don'pin want the line to be too long
//	                       if (count >= 5) {
//	                           fprintf(stdout, "\n");
//	                           count = 0;
//	                       } 
//	                       switch (path) {
//	                         case DEFIPATH_LAYER:
//	                              if (newLayer == 0) {
//	                                  fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                                  newLayer = 1;
//	                              } else
//	                                  fprintf(stdout, "NEW %s ",
//	                                          p->defiPath::getLayer());
//	                              break;
//	                         case DEFIPATH_VIA:
//	                              fprintf(stdout, "%s ", p->defiPath::getVia());
//	                              break;
//	                         case DEFIPATH_VIAROTATION:
//	                              fprintf(stdout, "%s ",
//	                                      p->defiPath::getViaRotationStr());
//	                              break;
//	                         case DEFIPATH_WIDTH:
//	                              fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                              break;
//	                         case DEFIPATH_POINT:
//	                              p->defiPath::getPoint(&x, &y);
//	                              fprintf(stdout, "( %d %d ) ", x, y);
//	                              break;
//	                         case DEFIPATH_FLUSHPOINT:
//	                              p->defiPath::getFlushPoint(&x, &y, &z);
//	                              fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                              break;
//	                         case DEFIPATH_TAPER:
//	                              fprintf(stdout, "TAPER ");
//	                              break;
//	                         case DEFIPATH_TAPERRULE:
//	                              fprintf(stdout, "TAPERRULE  %s ",
//	                                      p->defiPath::getTaperRule());
//	                              break;
//	                         case DEFIPATH_STYLE:
//	                              fprintf(stdout, "STYLE  %d ",
//	                                      p->defiPath::getStyle());
//	                              break;
//	                       }
//	                    }
//	                 }
//	              }
//	           }
//	         }
//	      }
//	   }
	
//	  if (net->defiNet::numProps())
//	  {
//	    for (i = 0; i < net->defiNet::numProps(); i++) {
//	        fprintf(stdout, "  + PROPERTY %s ", net->defiNet::propName(i));
//	        switch (net->defiNet::propType(i)) {
//	           case 'R': fprintf(stdout, "%g REAL ", net->defiNet::propNumber(i));
//	                     break;
//	           case 'I': fprintf(stdout, "%g INTEGER ", net->defiNet::propNumber(i));
//	                     break;
//	           case 'S': fprintf(stdout, "%s STRING ", net->defiNet::propValue(i));
//	                     break;
//	           case 'Q': fprintf(stdout, "%s QUOTESTRING ", net->defiNet::propValue(i));
//	                     break;
//	           case 'N': fprintf(stdout, "%g NUMBER ", net->defiNet::propNumber(i));
//	                     break;
//	        }
//	        fprintf(stdout, "\n");
//	    }
//	  }
	
//	  if (net->defiNet::hasWeight())
//	    fprintf(stdout, "+ WEIGHT %d ", net->defiNet::weight());
//	  if (net->defiNet::hasCap())
//	    fprintf(stdout, "+ ESTCAP %g ", net->defiNet::cap());
//	  if (net->defiNet::hasSource())
//	    fprintf(stdout, "+ SOURCE %s ", net->defiNet::source());
//	  if (net->defiNet::hasFixedbump())
//	    fprintf(stdout, "+ FIXEDBUMP ");
//	  if (net->defiNet::hasFrequency())
//	    fprintf(stdout, "+ FREQUENCY %g ", net->defiNet::frequency());
//	  if (net->defiNet::hasPattern())
//	    fprintf(stdout, "+ PATTERN %s ", net->defiNet::pattern());
//	  if (net->defiNet::hasOriginal())
//	    fprintf(stdout, "+ ORIGINAL %s ", net->defiNet::original());
	
//	  fprintf (stdout, ";\n");
//	  --numObjs;
//	  if (numObjs <= 0)
//	      fprintf(stdout, "END NETS\n");
	  return 0;
	}

	int DEFSnetpath(defrCallbackType_e c, defiNet* ppath, defiUserData ud) {
	  int         i, j, x, y, z, count, newLayer;
	  char*       layerName;
	  double      dist, left, right;
	  defiPath*   p;
	  defiSubnet  *s;
	  int         path;
	  defiShield* shield;
	  defiWire*   wire;
	  int         numX, numY, stepX, stepY;
		
	  if (c != defrSNetPartialPathCbkType)
	      return 1;
		
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf (stdout, "SPECIALNET partial data\n");
		  fprintf(stdout, "- %s ", ppath->defiNet::name());
		}
		
//	  count = 0;
	  // compName & pinName
//	  for (i = 0; i < ppath->defiNet::numConnections(); i++)
//	  {
//	      // set the limit of only 5 items print out in one line
//	      count++;
//	      if (count >= 5) {
//	          fprintf(stdout, "\n");
//	          count = 0;
//	      }
//	      fprintf (stdout, "( %s %s ) ", ppath->defiNet::instance(i),
//	               ppath->defiNet::pin(i));
//	      if (ppath->defiNet::pinIsSynthesized(i))
//	          fprintf(stdout, "+ SYNTHESIZED ");
//	  }
//	
//	  // specialWiring
//	  // POLYGON
//	  if (ppath->defiNet::numPolygons()) {
//	     struct defiPoints points;
//	    for (i = 0; i < ppath->defiNet::numPolygons(); i++) {
//	      fprintf(stdout, "\n  + POLYGON %s ", ppath->polygonName(i));
//	      points = ppath->getPolygon(i);
//	      for (j = 0; j < points.numPoints; j++)
//	        fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
//	    }
//	  }
//	  // RECT
//	  if (ppath->defiNet::numRectangles()) {
//	     for (i = 0; i < ppath->defiNet::numRectangles(); i++) {
//	       fprintf(stdout, "\n  + RECT %s %d %d %d %d", ppath->defiNet::rectName(i),
//	               ppath->defiNet::xl(i), ppath->defiNet::yl(i),
//	               ppath->defiNet::xh(i), ppath->defiNet::yh(i));
//	     }
//	  }
//	
//	  // COVER, FIXED, ROUTED or SHIELD
//	  if (ppath->defiNet::numWires()) {
//	     newLayer = 0;
//	     for (i = 0; i < ppath->defiNet::numWires(); i++) {
//	        newLayer = 0;
//	        wire = ppath->defiNet::wire(i);
//	        fprintf(stdout, "\n  + %s ", wire->wireType());
//	        if (strcmp (wire->wireType(), "SHIELD") == 0)
//	           fprintf(stdout, "%s ", wire->wireShieldNetName());
//	        for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	           p = wire->defiWire::path(j);
//	           p->initTraverse();
//	           while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	              count++;
//	              // Don'pin want the line to be too long
//	              if (count >= 5) {
//	                  fprintf(stdout, "\n");
//	                  count = 0;
//	              }
//	              switch (path) {
//	                case DEFIPATH_LAYER:
//	                     if (newLayer == 0) {
//	                         fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                         newLayer = 1;
//	                     } else
//	                         fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                     break;
//	                case DEFIPATH_VIA:
//	                     fprintf(stdout, "%s ", p->defiPath::getVia());
//	                     break;
//	                case DEFIPATH_VIAROTATION:
//	                     fprintf(stdout, "%s ",
//	                             DEFOrientStr(p->defiPath::getViaRotation()));
//	                     break;
//	                case DEFIPATH_VIADATA:
//	                     p->defiPath::getViaData(&numX, &numY, &stepX, &stepY);
//	                     fprintf(stdout, "DO %d BY %d STEP %d %d ", numX, numY,
//	                             stepX, stepY);
//	                     break;
//	                case DEFIPATH_WIDTH:
//	                     fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                     break;
//	                case DEFIPATH_POINT:
//	                     p->defiPath::getPoint(&x, &y);
//	                     fprintf(stdout, "( %d %d ) ", x, y);
//	                     break;
//	                case DEFIPATH_FLUSHPOINT:
//	                     p->defiPath::getFlushPoint(&x, &y, &z);
//	                     fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                     break;
//	                case DEFIPATH_TAPER:
//	                     fprintf(stdout, "TAPER ");
//	                     break;
//	                case DEFIPATH_SHAPE:
//	                     fprintf(stdout, "+ SHAPE %s ", p->defiPath::getShape());
//	                     break;
//	                case DEFIPATH_STYLE:
//	                     fprintf(stdout, "+ STYLE %d ", p->defiPath::getStyle());
//	                     break;
//	              }
//	           }
//	        }
//	        fprintf(stdout, "\n");
//	        count = 0;
//	     }
//	  }
//	
//	  if (ppath->defiNet::hasSubnets()) {
//	    for (i = 0; i < ppath->defiNet::numSubnets(); i++) {
//	      s = ppath->defiNet::subnet(i);
//	      if (s->defiSubnet::numConnections()) {
//	          if (s->defiSubnet::pinIsMustJoin(0))
//	              fprintf(stdout, "- MUSTJOIN ");
//	          else
//	              fprintf(stdout, "- %s ", s->defiSubnet::name());
//	          for (j = 0; j < s->defiSubnet::numConnections(); j++) {
//	              fprintf(stdout, " ( %s %s )\n", s->defiSubnet::instance(j),
//	                      s->defiSubnet::pin(j));
//	        }
//	      }
//	
//	      // regularWiring
//	      if (s->defiSubnet::numWires()) {
//	         for (i = 0; i < s->defiSubnet::numWires(); i++) {
//	            wire = s->defiSubnet::wire(i);
//	            fprintf(stdout, "  + %s ", wire->wireType());
//	            for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	              p = wire->defiWire::path(j);
//	              p->defiPath::print(stdout);
//	            }
//	         }
//	      }
//	    }
//	  }
//	
//	  if (ppath->defiNet::numProps()) {
//	    for (i = 0; i < ppath->defiNet::numProps(); i++) {
//	        if (ppath->defiNet::propIsString(i))
//	           fprintf(stdout, "  + PROPERTY %s %s ", ppath->defiNet::propName(i),
//	                   ppath->defiNet::propValue(i));
//	        if (ppath->defiNet::propIsNumber(i))
//	           fprintf(stdout, "  + PROPERTY %s %g ", ppath->defiNet::propName(i),
//	                   ppath->defiNet::propNumber(i));
//	        switch (ppath->defiNet::propType(i)) {
//	           case 'R': fprintf(stdout, "REAL ");
//	                     break;
//	           case 'I': fprintf(stdout, "INTEGER ");
//	                     break;
//	           case 'S': fprintf(stdout, "STRING ");
//	                     break;
//	           case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                     break;
//	           case 'N': fprintf(stdout, "NUMBER ");
//	                     break;
//	        }
//	        fprintf(stdout, "\n");
//	    }
//	  }
//	
//	  // SHIELD
//	  count = 0;
//	  // testing the SHIELD for 5.3, obsolete in 5.4
//	  if (ppath->defiNet::numShields()) {
//	    for (i = 0; i < ppath->defiNet::numShields(); i++) {
//	       shield = ppath->defiNet::shield(i);
//	       fprintf(stdout, "\n  + SHIELD %s ", shield->defiShield::shieldName());
//	       newLayer = 0;
//	       for (j = 0; j < shield->defiShield::numPaths(); j++) {
//	          p = shield->defiShield::path(j);
//	          p->initTraverse();
//	          while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	             count++;
//	             // Don'pin want the line to be too long
//	             if (count >= 5) {
//	                 fprintf(stdout, "\n");
//	                 count = 0;
//	             }
//	             switch (path) {
//	               case DEFIPATH_LAYER:
//	                    if (newLayer == 0) {
//	                        fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                        newLayer = 1;
//	                    } else
//	                        fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                    break;
//	               case DEFIPATH_VIA:
//	                    fprintf(stdout, "%s ", p->defiPath::getVia());
//	                    break;
//	               case DEFIPATH_VIAROTATION:
//	                    if (newLayer)
//	                       fprintf(stdout, "%s ",
//	                               DEFOrientStr(p->defiPath::getViaRotation()));
//	                    else
//	                       fprintf(stdout, "Str %s ",
//	                               p->defiPath::getViaRotationStr());
//	                    break;
//	               case DEFIPATH_WIDTH:
//	                    fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                    break;
//	               case DEFIPATH_POINT:
//	                    p->defiPath::getPoint(&x, &y);
//	                    fprintf(stdout, "( %d %d ) ", x, y);
//	                    break;
//	               case DEFIPATH_FLUSHPOINT:
//	                    p->defiPath::getFlushPoint(&x, &y, &z);
//	                    fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                    break;
//	               case DEFIPATH_TAPER:
//	                    fprintf(stdout, "TAPER ");
//	                    break;
//	               case DEFIPATH_SHAPE:
//	                    fprintf(stdout, "+ SHAPE %s ", p->defiPath::getShape());
//	                    break;
//	               case DEFIPATH_STYLE:
//	                    fprintf(stdout, "+ STYLE %d ", p->defiPath::getStyle());
//	             }
//	          }
//	       }
//	    }
//	  }
//	
//	  // layerName width
//	  if (ppath->defiNet::hasWidthRules()) {
//	    for (i = 0; i < ppath->defiNet::numWidthRules(); i++) {
//	        ppath->defiNet::widthRule(i, &layerName, &dist);
//	        fprintf (stdout, "\n  + WIDTH %s %g ", layerName, dist);
//	    }
//	  }
//	
//	  // layerName spacing
//	  if (ppath->defiNet::hasSpacingRules()) {
//	    for (i = 0; i < ppath->defiNet::numSpacingRules(); i++) {
//	        ppath->defiNet::spacingRule(i, &layerName, &dist, &left, &right);
//	        if (left == right)
//	            fprintf (stdout, "\n  + SPACING %s %g ", layerName, dist);
//	        else
//	            fprintf (stdout, "\n  + SPACING %s %g RANGE %g %g ",
//	                     layerName, dist, left, right);
//	    }
//	  }
//	
//	  if (ppath->defiNet::hasFixedbump())
//	    fprintf(stdout, "\n  + FIXEDBUMP ");
//	  if (ppath->defiNet::hasFrequency())
//	    fprintf(stdout, "\n  + FREQUENCY %g ", ppath->defiNet::frequency());
//	  if (ppath->defiNet::hasVoltage())
//	    fprintf(stdout, "\n  + VOLTAGE %g ", ppath->defiNet::voltage());
//	  if (ppath->defiNet::hasWeight())
//	    fprintf(stdout, "\n  + WEIGHT %d ", ppath->defiNet::weight());
//	  if (ppath->defiNet::hasCap())
//	    fprintf(stdout, "\n  + ESTCAP %g ", ppath->defiNet::cap());
//	  if (ppath->defiNet::hasSource())
//	    fprintf(stdout, "\n  + SOURCE %s ", ppath->defiNet::source());
//	  if (ppath->defiNet::hasPattern())
//	    fprintf(stdout, "\n  + PATTERN %s ", ppath->defiNet::pattern());
//	  if (ppath->defiNet::hasOriginal())
//	    fprintf(stdout, "\n  + ORIGINAL %s ", ppath->defiNet::original());
//	  if (ppath->defiNet::hasUse())
//	    fprintf(stdout, "\n  + USE %s ", ppath->defiNet::use());
//	
//	  fprintf(stdout, "\n");
//	
	  return 0;
	}

	int DEFSnetwire(defrCallbackType_e c, defiNet* ppath, defiUserData ud) {
	  int         i, j, x, y, z, count = 0, newLayer;
	  defiPath*   p;
	  int         path;
	  defiWire*   wire;
	  defiShield* shield;
	  int         numX, numY, stepX, stepY;
	
	  if (c != defrSNetWireCbkType)
	      return 1;
	
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf (stdout, "SPECIALNET wire data\n");
		  fprintf(stdout, "- %s ", ppath->defiNet::name());
		}
//	
//	  // specialWiring
//	  if (ppath->defiNet::numWires()) {
//	     newLayer = 0;
//	     for (i = 0; i < ppath->defiNet::numWires(); i++) {
//	        newLayer = 0;
//	        wire = ppath->defiNet::wire(i);
//	        fprintf(stdout, "\n  + %s ", wire->wireType());
//	        if (strcmp (wire->wireType(), "SHIELD") == 0)
//	           fprintf(stdout, "%s ", wire->wireShieldNetName());
//	        for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	           p = wire->defiWire::path(j);
//	           p->initTraverse();
//	           while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	              count++;
//	              // Don'pin want the line to be too long
//	              if (count >= 5) {
//	                  fprintf(stdout, "\n");
//	                  count = 0;
//	              }
//	              switch (path) {
//	                case DEFIPATH_LAYER:
//	                     if (newLayer == 0) {
//	                         fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                         newLayer = 1;
//	                     } else
//	                         fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                     break;
//	                case DEFIPATH_VIA:
//	                     fprintf(stdout, "%s ", p->defiPath::getVia());
//	                     break;
//	                case DEFIPATH_VIAROTATION:
//	                     fprintf(stdout, "%s ",
//	                             DEFOrientStr(p->defiPath::getViaRotation()));
//	                     break;
//	                case DEFIPATH_VIADATA:
//	                     p->defiPath::getViaData(&numX, &numY, &stepX, &stepY);
//	                     fprintf(stdout, "DO %d BY %d STEP %d %d ", numX, numY,
//	                             stepX, stepY);
//	                     break;
//	                case DEFIPATH_WIDTH:
//	                     fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                     break;
//	                case DEFIPATH_POINT:
//	                     p->defiPath::getPoint(&x, &y);
//	                     fprintf(stdout, "( %d %d ) ", x, y);
//	                     break;
//	                case DEFIPATH_FLUSHPOINT:
//	                     p->defiPath::getFlushPoint(&x, &y, &z);
//	                     fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                     break;
//	                case DEFIPATH_TAPER:
//	                     fprintf(stdout, "TAPER ");
//	                     break;
//	                case DEFIPATH_SHAPE:
//	                     fprintf(stdout, "+ SHAPE %s ", p->defiPath::getShape());
//	                     break;
//	                case DEFIPATH_STYLE:
//	                     fprintf(stdout, "+ STYLE %d ", p->defiPath::getStyle());
//	                     break;
//	              }
//	           }
//	        }
//	        fprintf(stdout, "\n");
//	        count = 0;
//	     }
//	  } else if (ppath->defiNet::numShields()) {
//	    for (i = 0; i < ppath->defiNet::numShields(); i++) {
//	       shield = ppath->defiNet::shield(i);
//	       fprintf(stdout, "\n  + SHIELD %s ", shield->defiShield::shieldName());
//	       newLayer = 0;
//	       for (j = 0; j < shield->defiShield::numPaths(); j++) {
//	          p = shield->defiShield::path(j);
//	          p->initTraverse();
//	          while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	             count++;
//	             // Don'pin want the line to be too long
//	             if (count >= 5) {
//	                 fprintf(stdout, "\n");
//	                 count = 0;
//	             } 
//	             switch (path) {
//	               case DEFIPATH_LAYER:
//	                    if (newLayer == 0) {
//	                        fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                        newLayer = 1;
//	                    } else
//	                        fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                    break;
//	               case DEFIPATH_VIA:
//	                    fprintf(stdout, "%s ", p->defiPath::getVia());
//	                    break;
//	               case DEFIPATH_VIAROTATION:
//	                    fprintf(stdout, "%s ", 
//	                            DEFOrientStr(p->defiPath::getViaRotation()));
//	                    break;
//	               case DEFIPATH_WIDTH:
//	                    fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                    break;
//	               case DEFIPATH_POINT:
//	                    p->defiPath::getPoint(&x, &y);
//	                    fprintf(stdout, "( %d %d ) ", x, y);
//	                    break;
//	               case DEFIPATH_FLUSHPOINT:
//	                    p->defiPath::getFlushPoint(&x, &y, &z);
//	                    fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                    break;
//	               case DEFIPATH_TAPER:
//	                    fprintf(stdout, "TAPER ");
//	                    break;
//	               case DEFIPATH_SHAPE:
//	                    fprintf(stdout, "+ SHAPE %s ", p->defiPath::getShape());
//	                    break;
//	               case DEFIPATH_STYLE:
//	                    fprintf(stdout, "+ STYLE %d ", p->defiPath::getStyle());
//	                    break;
//	             }
//	          }
//	       }
//	    }
//	  }
//	
//	  fprintf(stdout, "\n");
	
	  return 0;
	}

	int DEFSnetf(defrCallbackType_e c, defiNet* net, defiUserData ud) {
	  // For net and special net.
	  int         i, j, x, y, z, count, newLayer;
	  char*       layerName;
	  double      dist, left, right;
	  defiPath*   p;
	  defiSubnet  *s;
	  int         path;
	  defiShield* shield;
	  defiWire*   wire;
	  int         numX, numY, stepX, stepY;
	
	  DEFCheckType(c);
	  
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  if (c != defrSNetCbkType)
		      fprintf(stdout, "BOGUS NET TYPE  ");
		}
	
	// 5/6/2004 - don'pin need since I have a callback for the name
	//  fprintf(stdout, "- %s ", net->defiNet::name());
	
//	  count = 0;
//	  // compName & pinName
//	  for (i = 0; i < net->defiNet::numConnections(); i++) {
//	      // set the limit of only 5 items print out in one line
//	      count++;
//	      if (count >= 5) {
//	          fprintf(stdout, "\n");
//	          count = 0;
//	      }
//	      fprintf (stdout, "( %s %s ) ", net->defiNet::instance(i),
//	               net->defiNet::pin(i));
//	      if (net->defiNet::pinIsSynthesized(i))
//	          fprintf(stdout, "+ SYNTHESIZED ");
//	  }
//	
//	  // specialWiring
//	  if (net->defiNet::numWires()) {
//	     newLayer = 0;
//	     for (i = 0; i < net->defiNet::numWires(); i++) {
//	        newLayer = 0;
//	        wire = net->defiNet::wire(i);
//	        fprintf(stdout, "\n  + %s ", wire->wireType());
//	        if (strcmp (wire->wireType(), "SHIELD") == 0)
//	           fprintf(stdout, "%s ", wire->wireShieldNetName());
//	        for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	           p = wire->defiWire::path(j);
//	           p->initTraverse();
//	           while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	              count++;
//	              // Don'pin want the line to be too long
//	              if (count >= 5) {
//	                  fprintf(stdout, "\n");
//	                  count = 0;
//	              }
//	              switch (path) {
//	                case DEFIPATH_LAYER:
//	                     if (newLayer == 0) {
//	                         fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                         newLayer = 1;
//	                     } else
//	                         fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                     break;
//	                case DEFIPATH_VIA:
//	                     fprintf(stdout, "%s ", p->defiPath::getVia());
//	                     break;
//	                case DEFIPATH_VIAROTATION:
//	                     fprintf(stdout, "%s ", 
//	                             DEFOrientStr(p->defiPath::getViaRotation()));
//	                     break;
//	                case DEFIPATH_VIADATA:
//	                     p->defiPath::getViaData(&numX, &numY, &stepX, &stepY);
//	                     fprintf(stdout, "DO %d BY %d STEP %d %d ", numX, numY,
//	                             stepX, stepY);
//	                     break;
//	                case DEFIPATH_WIDTH:
//	                     fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                     break;
//	                case DEFIPATH_POINT:
//	                     p->defiPath::getPoint(&x, &y);
//	                     fprintf(stdout, "( %d %d ) ", x, y);
//	                     break;
//	                case DEFIPATH_FLUSHPOINT:
//	                     p->defiPath::getFlushPoint(&x, &y, &z);
//	                     fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                     break;
//	                case DEFIPATH_TAPER:
//	                     fprintf(stdout, "TAPER ");
//	                     break;
//	                case DEFIPATH_SHAPE:
//	                     fprintf(stdout, "+ SHAPE %s ", p->defiPath::getShape());
//	                     break;
//	                case DEFIPATH_STYLE:
//	                     fprintf(stdout, "+ STYLE %d ", p->defiPath::getStyle());
//	                     break;
//	              }
//	           }
//	        }
//	        fprintf(stdout, "\n");
//	        count = 0;
//	     }
//	  }
//	  // POLYGON
//	  if (net->defiNet::numPolygons()) {
//	    struct defiPoints points;
//	    for (i = 0; i < net->defiNet::numPolygons(); i++) {
//	      fprintf(stdout, "\n  + POLYGON %s ", net->polygonName(i));
//	      points = net->getPolygon(i);
//	      for (j = 0; j < points.numPoints; j++)
//	        fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
//	    }
//	  }
//	  // RECT
//	  if (net->defiNet::numRectangles()) {
//	     for (i = 0; i < net->defiNet::numRectangles(); i++) {
//	       fprintf(stdout, "\n  + RECT %s %d %d %d %d", net->defiNet::rectName(i),
//	               net->defiNet::xl(i), net->defiNet::yl(i), net->defiNet::xh(i),
//	               net->defiNet::yh(i));
//	     }
//	  }
//	
//	  if (net->defiNet::hasSubnets()) {
//	    for (i = 0; i < net->defiNet::numSubnets(); i++) {
//	      s = net->defiNet::subnet(i);
//	      if (s->defiSubnet::numConnections()) {
//	          if (s->defiSubnet::pinIsMustJoin(0))
//	              fprintf(stdout, "- MUSTJOIN ");
//	          else
//	              fprintf(stdout, "- %s ", s->defiSubnet::name());
//	          for (j = 0; j < s->defiSubnet::numConnections(); j++) {
//	              fprintf(stdout, " ( %s %s )\n", s->defiSubnet::instance(j),
//	                      s->defiSubnet::pin(j));
//	        }
//	      }
//	 
//	      // regularWiring
//	      if (s->defiSubnet::numWires()) {
//	         for (i = 0; i < s->defiSubnet::numWires(); i++) {
//	            wire = s->defiSubnet::wire(i);
//	            fprintf(stdout, "  + %s ", wire->wireType());
//	            for (j = 0; j < wire->defiWire::numPaths(); j++) {
//	              p = wire->defiWire::path(j);
//	              p->defiPath::print(stdout);
//	            }
//	         }
//	      }
//	    }
//	  }
//	
//	  if (net->defiNet::numProps()) {
//	    for (i = 0; i < net->defiNet::numProps(); i++) {
//	        if (net->defiNet::propIsString(i))
//	           fprintf(stdout, "  + PROPERTY %s %s ", net->defiNet::propName(i),
//	                   net->defiNet::propValue(i));
//	        if (net->defiNet::propIsNumber(i))
//	           fprintf(stdout, "  + PROPERTY %s %g ", net->defiNet::propName(i),
//	                   net->defiNet::propNumber(i));
//	        switch (net->defiNet::propType(i)) {
//	           case 'R': fprintf(stdout, "REAL ");
//	                     break;
//	           case 'I': fprintf(stdout, "INTEGER ");
//	                     break;
//	           case 'S': fprintf(stdout, "STRING ");
//	                     break;
//	           case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                     break;
//	           case 'N': fprintf(stdout, "NUMBER ");
//	                     break;
//	        }
//	        fprintf(stdout, "\n");
//	    }
//	  }
//	
//	  // SHIELD
//	  count = 0;
//	  // testing the SHIELD for 5.3, obsolete in 5.4
//	  if (net->defiNet::numShields()) {
//	    for (i = 0; i < net->defiNet::numShields(); i++) {
//	       shield = net->defiNet::shield(i);
//	       fprintf(stdout, "\n  + SHIELD %s ", shield->defiShield::shieldName());
//	       newLayer = 0;
//	       for (j = 0; j < shield->defiShield::numPaths(); j++) {
//	          p = shield->defiShield::path(j);
//	          p->initTraverse();
//	          while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
//	             count++;
//	             // Don'pin want the line to be too long
//	             if (count >= 5) {
//	                 fprintf(stdout, "\n");
//	                 count = 0;
//	             } 
//	             switch (path) {
//	               case DEFIPATH_LAYER:
//	                    if (newLayer == 0) {
//	                        fprintf(stdout, "%s ", p->defiPath::getLayer());
//	                        newLayer = 1;
//	                    } else
//	                        fprintf(stdout, "NEW %s ", p->defiPath::getLayer());
//	                    break;
//	               case DEFIPATH_VIA:
//	                    fprintf(stdout, "%s ", p->defiPath::getVia());
//	                    break;
//	               case DEFIPATH_VIAROTATION:
//	                    fprintf(stdout, "%s ", 
//	                            DEFOrientStr(p->defiPath::getViaRotation()));
//	                    break;
//	               case DEFIPATH_WIDTH:
//	                    fprintf(stdout, "%d ", p->defiPath::getWidth());
//	                    break;
//	               case DEFIPATH_POINT:
//	                    p->defiPath::getPoint(&x, &y);
//	                    fprintf(stdout, "( %d %d ) ", x, y);
//	                    break;
//	               case DEFIPATH_FLUSHPOINT:
//	                    p->defiPath::getFlushPoint(&x, &y, &z);
//	                    fprintf(stdout, "( %d %d %d ) ", x, y, z);
//	                    break;
//	               case DEFIPATH_TAPER:
//	                    fprintf(stdout, "TAPER ");
//	                    break;
//	               case DEFIPATH_SHAPE:
//	                    fprintf(stdout, "+ SHAPE %s ", p->defiPath::getShape());
//	                    break;
//	               case DEFIPATH_STYLE:
//	                    fprintf(stdout, "+ STYLE %d ", p->defiPath::getStyle());
//	                    break;
//	             }
//	          }
//	       }
//	    }
//	  }
//	
//	  // layerName width
//	  if (net->defiNet::hasWidthRules()) {
//	    for (i = 0; i < net->defiNet::numWidthRules(); i++) {
//	        net->defiNet::widthRule(i, &layerName, &dist);
//	        fprintf (stdout, "\n  + WIDTH %s %g ", layerName, dist);
//	    }
//	  }
//	
//	  // layerName spacing
//	  if (net->defiNet::hasSpacingRules()) {
//	    for (i = 0; i < net->defiNet::numSpacingRules(); i++) {
//	        net->defiNet::spacingRule(i, &layerName, &dist, &left, &right);
//	        if (left == right)
//	            fprintf (stdout, "\n  + SPACING %s %g ", layerName, dist);
//	        else
//	            fprintf (stdout, "\n  + SPACING %s %g RANGE %g %g ",
//	                     layerName, dist, left, right);
//	    }
//	  }
//	
//	  if (net->defiNet::hasFixedbump())
//	    fprintf(stdout, "\n  + FIXEDBUMP ");
//	  if (net->defiNet::hasFrequency())
//	    fprintf(stdout, "\n  + FREQUENCY %g ", net->defiNet::frequency());
//	  if (net->defiNet::hasVoltage())
//	    fprintf(stdout, "\n  + VOLTAGE %g ", net->defiNet::voltage());
//	  if (net->defiNet::hasWeight())
//	    fprintf(stdout, "\n  + WEIGHT %d ", net->defiNet::weight());
//	  if (net->defiNet::hasCap())
//	    fprintf(stdout, "\n  + ESTCAP %g ", net->defiNet::cap());
//	  if (net->defiNet::hasSource())
//	    fprintf(stdout, "\n  + SOURCE %s ", net->defiNet::source());
//	  if (net->defiNet::hasPattern())
//	    fprintf(stdout, "\n  + PATTERN %s ", net->defiNet::pattern());
//	  if (net->defiNet::hasOriginal())
//	    fprintf(stdout, "\n  + ORIGINAL %s ", net->defiNet::original());
//	  if (net->defiNet::hasUse())
//	    fprintf(stdout, "\n  + USE %s ", net->defiNet::use());
//	
//	  fprintf (stdout, ";\n");
//	  --numObjs;
//	  if (numObjs <= 0)
//	      fprintf(stdout, "END SPECIALNETS\n");

	  return 0;
	}

	int DEFNdr(defrCallbackType_e c, defiNonDefault* nd, defiUserData ud) {
	  // For nondefaultrule
	  int i;
	
	  DEFCheckType(c);
//	  
//	  if (c != defrNonDefaultCbkType)
//	      fprintf(stdout, "BOGUS NONDEFAULTRULE TYPE  ");
//	  fprintf(stdout, "- %s\n", nd->defiNonDefault::name());
//	  if (nd->defiNonDefault::hasHardspacing())
//	      fprintf(stdout, "   + HARDSPACING\n");
//	  for (i = 0; i < nd->defiNonDefault::numLayers(); i++) {
//	    fprintf(stdout, "   + LAYER %s", nd->defiNonDefault::layerName(i));
//	    fprintf(stdout, " WIDTH %d", nd->defiNonDefault::layerWidthVal(i));
//	    if (nd->defiNonDefault::hasLayerDiagWidth(i)) 
//	      fprintf(stdout, " DIAGWIDTH %d",
//	              nd->defiNonDefault::layerDiagWidthVal(i));
//	    if (nd->defiNonDefault::hasLayerSpacing(i)) 
//	      fprintf(stdout, " SPACING %d", nd->defiNonDefault::layerSpacingVal(i));
//	    if (nd->defiNonDefault::hasLayerWireExt(i)) 
//	      fprintf(stdout, " WIREEXT %d", nd->defiNonDefault::layerWireExtVal(i));
//	    fprintf(stdout, "\n");
//	  }
//	  for (i = 0; i < nd->defiNonDefault::numVias(); i++)
//	    fprintf(stdout, "   + VIA %s\n", nd->defiNonDefault::viaName(i));
//	  for (i = 0; i < nd->defiNonDefault::numViaRules(); i++)
//	    fprintf(stdout, "   + VIARULE %s\n", nd->defiNonDefault::viaRuleName(i));
//	  for (i = 0; i < nd->defiNonDefault::numMinCuts(); i++)
//	    fprintf(stdout, "   + MINCUTS %s %d\n", nd->defiNonDefault::cutLayerName(i),
//	            nd->defiNonDefault::numCuts(i));
//	  for (i = 0; i < nd->defiNonDefault::numProps(); i++) {
//	    fprintf(stdout, "   + PROPERTY %s %s ", nd->defiNonDefault::propName(i),
//	            nd->defiNonDefault::propValue(i));
//	    switch (nd->defiNonDefault::propType(i)) {
//	      case 'R': fprintf(stdout, "REAL\n");
//	                break;
//	      case 'I': fprintf(stdout, "INTEGER\n");
//	                break;
//	      case 'S': fprintf(stdout, "STRING\n");
//	                break;
//	      case 'Q': fprintf(stdout, "QUOTESTRING\n");
//	                break;
//	      case 'N': fprintf(stdout, "NUMBER\n");
//	                break;
//	    }
//	  }
//	  --numObjs;
//	  if (numObjs <= 0)
//	    fprintf(stdout, "END NONDEFAULTRULES\n");
	  return 0;
	}

	int DEFTname(defrCallbackType_e c, const char* string, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "TECHNOLOGY %s ;\n", string);
		}
	  return 0;
	}

	int DEFDname(defrCallbackType_e c, const char* string, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "DESIGN %s ;\n", string);
		}
		
	  return 0;
	}

	char* DEFAddress(const char* in) {
	  return ((char*)in);
	}

	int DEFCs(defrCallbackType_e c, int num, defiUserData ud) {
	  char* name;
		Design *design = (Design *)ud;
		
	  DEFCheckType(c);
		
	  switch (c)
	  {
		  case defrComponentStartCbkType :
		  	name = DEFAddress("COMPONENTS");
				design->setExpGateNum(num);
		  	break;
		  case defrNetStartCbkType :
		  	name = DEFAddress("NETS");
				design->setExpNetNum(num);
		  	break;
		  case defrStartPinsCbkType :
		  	name = DEFAddress("PINS");
				design->setExpPortNum(num);
		  	break;
		  case defrSNetStartCbkType :
		  	name = DEFAddress("SPECIALNETS");
				design->setExpSpecialNetNum(num);
		  	break;
		  case defrViaStartCbkType : name = DEFAddress("VIAS"); break;
		  case defrRegionStartCbkType : name = DEFAddress("REGIONS"); break;
		  case defrGroupsStartCbkType : name = DEFAddress("GROUPS"); break;
		  case defrScanchainsStartCbkType : name = DEFAddress("SCANCHAINS"); break;
		  case defrIOTimingsStartCbkType : name = DEFAddress("IOTIMINGS"); break;
		  case defrFPCStartCbkType : name = DEFAddress("FLOORPLANCONSTRAINTS"); break;
		  case defrTimingDisablesStartCbkType : name = DEFAddress("TIMING DISABLES"); break;
		  case defrPartitionsStartCbkType : name = DEFAddress("PARTITIONS"); break;
		  case defrPinPropStartCbkType : name = DEFAddress("PINPROPERTIES"); break;
		  case defrBlockageStartCbkType : name = DEFAddress("BLOCKAGES"); break;
		  case defrSlotStartCbkType : name = DEFAddress("SLOTS"); break;
		  case defrFillStartCbkType : name = DEFAddress("FILLS"); break;
		  case defrNonDefaultStartCbkType : name = DEFAddress("NONDEFAULTRULES"); break;
		  case defrStylesStartCbkType : name = DEFAddress("STYLES"); break;
		  default : name = DEFAddress("BOGUS"); return 1;
	  }
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 0)
		{
	  	fprintf(stdout, "\n%s %d ;\n", name, num);
		}
	  numObjs = num;
	  return 0;
	}

	int DEFConstraintst(defrCallbackType_e c, int num, defiUserData ud) {
	  // Handles both constraints and assertions
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  if (c == defrConstraintsStartCbkType)
	      fprintf(stdout, "\nCONSTRAINTS %d ;\n\n", num);
		  else
	      fprintf(stdout, "\nASSERTIONS %d ;\n\n", num);
	  }
	  numObjs = num;
	  return 0;
	}

	void DEFOperand(defrCallbackType_e c, defiAssertion* a, int ind) {
	  int i, first = 1;
	  char* netName;
	  char* fromInst, * fromPin, * toInst, * toPin;
	
	  if (a->defiAssertion::isSum())
	  {
	      // Sum in DEFOperand, recursively call DEFOperand
//	      fprintf(stdout, "- SUM ( ");
	      a->defiAssertion::unsetSum();
	      isSumSet = 1;
	      begOperand = 0;
	      DEFOperand (c, a, ind);
//	      fprintf(stdout, ") ");
	  } else {
	      // DEFOperand
	      if (ind >= a->defiAssertion::numItems()) {
	          fprintf(stdout, "ERROR: when writing out SUM in Constraints.\n");
	          return;
	       }
	      if (begOperand) {
//	         fprintf(stdout, "- ");
	         begOperand = 0;
	      }
	      for (i = ind; i < a->defiAssertion::numItems(); i++) {
	          if (a->defiAssertion::isNet(i))
	          {
	              a->defiAssertion::net(i, &netName);
//	              if (!first)
//	                  fprintf(stdout, ", "); // print , as separator
//	              fprintf(stdout, "NET %s ", netName); 
	          }
	          else if (a->defiAssertion::isPath(i))
	          {
	              a->defiAssertion::path(i, &fromInst, &fromPin, &toInst, &toPin);
//	              if (!first)
//	                  fprintf(stdout, ", ");
//	              fprintf(stdout, "PATH %s %s %s %s ", fromInst, fromPin, toInst,
//	                      toPin);
	          } else if (isSumSet) {
	              // SUM within SUM, reset the flag
	              a->defiAssertion::setSum();
	              DEFOperand(c, a, i);
	          }
	          first = 0;
	      } 
	      
	  }
	}

	int DEFConstraint(defrCallbackType_e c, defiAssertion* a, defiUserData ud) {
	  // Handles both constraints and assertions
	
	  DEFCheckType(c);
//	  if (a->defiAssertion::isWiredlogic())
//	      // Wirelogic
//	      fprintf(stdout, "- WIREDLOGIC %s + MAXDIST %g ;\n",
//	// Wiredlogic dist is also store in fallMax
//	//              a->defiAssertion::netName(), a->defiAssertion::distance());
//	              a->defiAssertion::netName(), a->defiAssertion::fallMax());
//	  else 
	  	{
	      // Call the DEFOperand function
	      isSumSet = 0;    // reset the global variable
	      begOperand = 1;
	      DEFOperand (c, a, 0);
	      // Get the Rise and Fall
//	      if (a->defiAssertion::hasRiseMax())
//	          fprintf(stdout, "+ RISEMAX %g ", a->defiAssertion::riseMax());
//	      if (a->defiAssertion::hasFallMax())
//	          fprintf(stdout, "+ FALLMAX %g ", a->defiAssertion::fallMax());
//	      if (a->defiAssertion::hasRiseMin())
//	          fprintf(stdout, "+ RISEMIN %g ", a->defiAssertion::riseMin());
//	      if (a->defiAssertion::hasFallMin())
//	          fprintf(stdout, "+ FALLMIN %g ", a->defiAssertion::fallMin());
//	      fprintf(stdout, ";\n");
	  }
//	  --numObjs;
//	  if (numObjs <= 0) {
//	      if (c == defrConstraintCbkType)
//	          fprintf(stdout, "END CONSTRAINTS\n");
//	      else 
//	          fprintf(stdout, "END ASSERTIONS\n");
//	  }
	  return 0;
	}

	int DEFPropstart(defrCallbackType_e c, void* dummy, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "\nPROPERTYDEFINITIONS\n");
		}
	  isProp = 1;
	
	  return 0;
	}

	int DEFProp(defrCallbackType_e c, defiProp* p, defiUserData ud) {
	  DEFCheckType(c);
//	  if (strcmp(p->defiProp::propType(), "design") == 0)
//	      fprintf(stdout, "DESIGN %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "net") == 0)
//	      fprintf(stdout, "NET %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "component") == 0)
//	      fprintf(stdout, "COMPONENT %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "specialnet") == 0)
//	      fprintf(stdout, "SPECIALNET %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "group") == 0)
//	      fprintf(stdout, "GROUP %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "row") == 0)
//	      fprintf(stdout, "ROW %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "componentpin") == 0)
//	      fprintf(stdout, "COMPONENTPIN %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "region") == 0)
//	      fprintf(stdout, "REGION %s ", p->defiProp::propName());
//	  else if (strcmp(p->defiProp::propType(), "nondefaultrule") == 0)
//	      fprintf(stdout, "NONDEFAULTRULE %s ", p->defiProp::propName());
//	  if (p->defiProp::dataType() == 'I')
//	      fprintf(stdout, "INTEGER ");
//	  if (p->defiProp::dataType() == 'R')
//	      fprintf(stdout, "REAL ");
//	  if (p->defiProp::dataType() == 'S')
//	      fprintf(stdout, "STRING ");
//	  if (p->defiProp::dataType() == 'Q')
//	      fprintf(stdout, "STRING ");
//	  if (p->defiProp::hasRange()) {
//	      fprintf(stdout, "RANGE %g %g ", p->defiProp::left(),
//	              p->defiProp::right());
//	  }
//	  if (p->defiProp::hasNumber())
//	      fprintf(stdout, "%g ", p->defiProp::number());
//	  if (p->defiProp::hasString())
//	      fprintf(stdout, "\"%s\" ", p->defiProp::string());
//	  fprintf(stdout, ";\n");
	
	  return 0;
	}

	int DEFPropend(defrCallbackType_e c, void* dummy, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
	  if (isProp) {
			if (v.getForMajStats() > 2)
			{
	      fprintf(stdout, "END PROPERTYDEFINITIONS\n\n");
	    }
	      isProp = 0;
	  }
	
	  defrSetCaseSensitivity(1);
	  return 0;
	}

	int DEFHist(defrCallbackType_e c, const char* h, defiUserData ud) {
	  DEFCheckType(c);
	  defrSetCaseSensitivity(0);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "HISTORY %s ;\n", h);
		}
	  defrSetCaseSensitivity(1);
	  return 0;
	}

	int DEFAn(defrCallbackType_e c, const char* h, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "ARRAY %s ;\n", h);
		}
	  return 0;
	}

	int DEFFn(defrCallbackType_e c, const char* h, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "FLOORPLAN %s ;\n", h);
		}
	  return 0;
	}

	int DEFBbn(defrCallbackType_e c, const char* h, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "BUSBITCHARS \"%s\" ;\n", h);
		}
	  return 0;
	}

	int DEFVers(defrCallbackType_e c, double d, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "VERSION %g ;\n", d);
		}
	    curVer = d;
	
	  // wmd -- testing the alias
	  defrAddAlias ("alias1", "aliasValue1", 1);
	  defrAddAlias ("alias2", "aliasValue2", 0);
	  defiAlias_itr *aliasStore;
	  aliasStore = (defiAlias_itr*)malloc(sizeof(defiAlias_itr*));
	  aliasStore->Init();
//	  while (aliasStore->defiAlias_itr::Next()) {
//	     fprintf(stdout, "ALIAS %s %s %d ;\n", aliasStore->defiAlias_itr::Key(),
//	                   aliasStore->defiAlias_itr::Data(),
//	                   aliasStore->defiAlias_itr::Marked());
//	  } 
	  free(aliasStore);
	  return 0;
	}

	int DEFVersStr(defrCallbackType_e c, const char* versionName, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "VERSION %s ;\n", versionName);
		}
	  return 0;
	}

	int DEFCasesens(defrCallbackType_e c, int d, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  if (d == 1)
	     fprintf(stdout, "NAMESCASESENSITIVE ON ;\n", d);
		  else
	     fprintf(stdout, "NAMESCASESENSITIVE OFF ;\n", d);
	  }
	  return 0;
	}

	int DEFCls(defrCallbackType_e c, void* cl, defiUserData ud)
	{
		static int padIndex = 0;
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		Chip &chip = design->getChip();
	  defiSite* site;  // Site and Canplace and CannotOccupy
	  defiBox* box;  // DieArea and 
	  defiPinCap* pc;
	  defiPin* pin;
	  int i, j, k;
	  defiRow* row;
	  defiTrack* track;
	  defiGcellGrid* gcg;
	  defiVia* via;
	  defiRegion* re;
	  defiGroup* group;
	  defiScanchain* sc;
	  defiIOTiming* iot;
	  defiFPC* fpc;
	  defiTimingDisable* td;
	  defiPartition* part;
	  defiPinProp* pprop;
	  defiBlockage* block;
	  defiSlot* slots;
	  defiFill* fills;
	  defiStyles* styles;
	  int xl, yl, xh, yh;
	  char *name, *a1, *b1;
	  char **inst, **inPin, **outPin;
	  int  *bits;
	  int  size;
	  int corner, typ;
	  const char *itemT;
	  char dir;
	  defiPinAntennaModel* aModel;
	  struct defiPoints points;
		
	  DEFCheckType(c);
	  switch (c) {
		
	  case defrSiteCbkType :
	         site = (defiSite*)cl;
//	         fprintf(stdout, "SITE %s %g %g %s ", site->defiSite::name(),
//	                 site->defiSite::x_orig(), site->defiSite::y_orig(),
//	                 DEFOrientStr(site->defiSite::orient()));
//	         fprintf(stdout, "DO %g BY %g STEP %g %g ;\n",
//	                 site->defiSite::x_num(), site->defiSite::y_num(),
//	                 site->defiSite::x_step(), site->defiSite::y_step());
	         break;
	  case defrCanplaceCbkType :
	         site = (defiSite*)cl;
//	         fprintf(stdout, "CANPLACE %s %g %g %s ", site->defiSite::name(),
//	                 site->defiSite::x_orig(), site->defiSite::y_orig(),
//	                 DEFOrientStr(site->defiSite::orient()));
//	         fprintf(stdout, "DO %g BY %g STEP %g %g ;\n",
//	                 site->defiSite::x_num(), site->defiSite::y_num(),
//	                 site->defiSite::x_step(), site->defiSite::y_step());
	         break;
	  case defrCannotOccupyCbkType : 
	         site = (defiSite*)cl;
//	         fprintf(stdout, "CANNOTOCCUPY %s %g %g %s ",
//	                 site->defiSite::name(), site->defiSite::x_orig(),
//	                 site->defiSite::y_orig(), DEFOrientStr(site->defiSite::orient()));
//	         fprintf(stdout, "DO %g BY %g STEP %g %g ;\n",
//	                 site->defiSite::x_num(), site->defiSite::y_num(),
//	                 site->defiSite::x_step(), site->defiSite::y_step());
	         break;
	  case defrDieAreaCbkType :
	         box = (defiBox*)cl;
					chip.setDim(box->defiBox::xl(), box->defiBox::yl(), box->defiBox::xh(), box->defiBox::yh());
//	         fprintf(stdout, "DIEAREA %d %d %d %d ;\n",
//	                 box->defiBox::xl(), box->defiBox::yl(), box->defiBox::xh(),
//	                 box->defiBox::yh());
//	         fprintf(stdout, "DIEAREA ");
//	         points = box->defiBox::getPoint();
//	         for (i = 0; i < points.numPoints; i++)
//	           fprintf(stdout, "%d %d ", points.x[i], points.y[i]);
//	         fprintf(stdout, ";\n");
	         break;
	  case defrPinCapCbkType :
	         pc = (defiPinCap*)cl;
//	         fprintf(stdout, "MINPINS %d WIRECAP %g ;\n",
//	                 pc->defiPinCap::pin(), pc->defiPinCap::cap());
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END DEFAULTCAP\n");
	         break;
	  case defrPinCbkType :
	  	{
						PadType padType = InputOutput;
						pin = (defiPin*)cl;
	  				const char *padName = pin->defiPin::pinName();
	  				const char *netName = pin->defiPin::netName();
	  				
//	         fprintf(stdout, "- %s + NET %s ", pin->defiPin::pinName(), pin->defiPin::netName());
	//         pin->defiPin::changePinName("pinName");
	//         fprintf(stdout, "%s ", pin->defiPin::pinName());
	         if (pin->defiPin::hasDirection())
	         {
	         		const char *dir = pin->defiPin::direction();
	         		if (strcmp(dir, "INPUT") == 0)
	         		{
	         			padType = PrimiaryInput;
	         		}
	         		if (strcmp(dir, "OUTPUT") == 0)
	         		{
	         			padType = PrimiaryOutput;
	         		}
//							fprintf(stdout, "+ DIRECTION %s ", pin->defiPin::direction());
	         }

					Chip &chip = design->getChip();
					int chipLeft = chip.getLX();
					int chipBottom = chip.getBY();
					int lx = INT_MAX, rx = INT_MIN, by = INT_MAX, ty = INT_MIN;
					const char *orientStr = "N";
					int x = 0, y = 0, originX = 0, originY = 0;
	         	
//	         if (pin->defiPin::hasUse())
//	             fprintf(stdout, "+ USE %s ", pin->defiPin::use());
//	         if (pin->defiPin::hasNetExpr())
//	             fprintf(stdout, "+ NETEXPR \"%s\" ", pin->defiPin::netExpr());
//	         if (pin->defiPin::hasSupplySensitivity())
//	             fprintf(stdout, "+ SUPPLYSENSITIVITY %s ", pin->defiPin::supplySensitivity());
//	         if (pin->defiPin::hasGroundSensitivity())
//	             fprintf(stdout, "+ GROUNDSENSITIVITY %s ", pin->defiPin::groundSensitivity());
	         if (pin->defiPin::hasLayer())
	         {
	             struct defiPoints points;
	             for (i = 0; i < pin->defiPin::numLayer(); i++)
	             {
//	                fprintf(stdout, "\n  + LAYER %s ", pin->defiPin::layer(i));
//	                if (pin->defiPin::hasLayerSpacing(i))
//	                  fprintf(stdout, "SPACING %d ", pin->defiPin::layerSpacing(i));
//	                if (pin->defiPin::hasLayerDesignRuleWidth(i))
//	                  fprintf(stdout, "DESIGNRULEWIDTH %d ", pin->defiPin::layerDesignRuleWidth(i));
	                pin->defiPin::bounds(i, &xl, &yl, &xh, &yh);
	                if (lx > xl)
										lx = xl;
									if (rx < xh)
										rx = xh;
									if (by > yl)
										by = yl;
									if (ty < yh)
										ty = yh;
//	                fprintf(stdout, "%d %d %d %d ", xl, yl, xh, yh);
	             }
	             for (i = 0; i < pin->defiPin::numPolygons(); i++)
	             {
//	                fprintf(stdout, "\n  + POLYGON %s ", pin->defiPin::polygonName(i));
//	                if (pin->defiPin::hasPolygonSpacing(i))
//	                  fprintf(stdout, "SPACING %d ", pin->defiPin::polygonSpacing(i));
//	                if (pin->defiPin::hasPolygonDesignRuleWidth(i))
//	                  fprintf(stdout, "DESIGNRULEWIDTH %d ", pin->defiPin::polygonDesignRuleWidth(i));
	                points = pin->defiPin::getPolygon(i);
	                for (j = 0; j < points.numPoints; j++)
	                {
//	                  fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
		                if (lx > points.x[j])
											lx = points.x[j];
										if (rx < points.x[j])
											rx = points.x[j];
										if (by > points.y[j])
											by = points.y[j];
										if (ty < points.y[j])
											ty = points.y[j];
	                }
	             }
	             for (i = 0; i < pin->defiPin::numVias(); i++)
	             {
//	               fprintf(stdout, "\n  + VIA %s %d %d ", pin->defiPin::viaName(i), pin->defiPin::viaPtX(i), pin->defiPin::viaPtY(i));
	                if (lx > pin->defiPin::viaPtX(i))
										lx = pin->defiPin::viaPtX(i);
									if (rx < pin->defiPin::viaPtX(i))
										rx = pin->defiPin::viaPtX(i);
									if (by > pin->defiPin::viaPtY(i))
										by = pin->defiPin::viaPtY(i);
									if (ty < pin->defiPin::viaPtY(i))
										ty = pin->defiPin::viaPtY(i);
	             }
	         }
//	         if (pin->defiPin::hasPort())
//	         {
//	             struct defiPoints points;
//	             defiPinPort* port;
//	             for (j = 0; j < pin->defiPin::numPorts(); j++)
//	             {
//	                port = pin->defiPin::pinPort(j);
//	                fprintf(stdout, "\n  + PORT");
//	                for (i = 0; i < port->defiPinPort::numLayer(); i++)
//	                {
//	                   fprintf(stdout, "\n     + LAYER %s ", port->defiPinPort::layer(i));
//	                   if (port->defiPinPort::hasLayerSpacing(i))
//	                     fprintf(stdout, "SPACING %d ", port->defiPinPort::layerSpacing(i));
//	                   if (port->defiPinPort::hasLayerDesignRuleWidth(i))
//	                     fprintf(stdout, "DESIGNRULEWIDTH %d ", port->defiPinPort::layerDesignRuleWidth(i));
//	                   port->defiPinPort::bounds(i, &xl, &yl, &xh, &yh);
//	                   fprintf(stdout, "%d %d %d %d ", xl, yl, xh, yh);
//	                }
//	                for (i = 0; i < port->defiPinPort::numPolygons(); i++)
//	                {
//	                   fprintf(stdout, "\n     + POLYGON %s ", port->defiPinPort::polygonName(i));
//	                   if (port->defiPinPort::hasPolygonSpacing(i))
//	                     fprintf(stdout, "SPACING %d ", port->defiPinPort::polygonSpacing(i));
//	                   if (port->defiPinPort::hasPolygonDesignRuleWidth(i))
//	                     fprintf(stdout, "DESIGNRULEWIDTH %d ", port->defiPinPort::polygonDesignRuleWidth(i));
//	                   points = port->defiPinPort::getPolygon(i);
//	                   for (k = 0; k < points.numPoints; k++)
//	                     fprintf(stdout, "( %d %d ) ", points.x[k], points.y[k]);
//	                }
//	                for (i = 0; i < port->defiPinPort::numVias(); i++)
//	                {
//	                   fprintf(stdout, "\n     + VIA %s ( %d %d ) ", port->defiPinPort::viaName(i),
//	                   				 port->defiPinPort::viaPtX(i), port->defiPinPort::viaPtY(i));
//	                }
//	                if (port->defiPinPort::hasPlacement())
//	                {
//	                   if (port->defiPinPort::isPlaced())
//	                   {
//	                      fprintf(stdout, "\n     + PLACED ");
//	                      fprintf(stdout, "( %d %d ) %s ", port->defiPinPort::placementX(),
//	                         port->defiPinPort::placementY(), DEFOrientStr(port->defiPinPort::orient()));
//	                   }
//	                   if (port->defiPinPort::isCover())
//	                   {
//	                      fprintf(stdout, "\n     + COVER ");
//	                      fprintf(stdout, "( %d %d ) %s ", port->defiPinPort::placementX(),
//	                         port->defiPinPort::placementY(), DEFOrientStr(port->defiPinPort::orient()));
//	                   }
//	                   if (port->defiPinPort::isFixed())
//	                   {
//	                      fprintf(stdout, "\n     + FIXED ");
//	                      fprintf(stdout, "( %d %d ) %s ", port->defiPinPort::placementX(),
//	                         port->defiPinPort::placementY(), DEFOrientStr(port->defiPinPort::orient()));
//	                   }
//	                }
//	            }
//	         }
						
	         if (pin->defiPin::hasPlacement())
	         {
	             if (pin->defiPin::isPlaced())
	             {
//	               fprintf(stdout, "+ PLACED ");
//	               fprintf(stdout, "( %d %d ) %s ", pin->defiPin::placementX(), pin->defiPin::placementY(), DEFOrientStr(pin->defiPin::orient()));
	                orientStr = DEFOrientStr(pin->defiPin::orient());
									originX = pin->defiPin::placementX();
									originY = pin->defiPin::placementY();
	             }
	             if (pin->defiPin::isCover())
	             {
//	                 fprintf(stdout, "+ COVER ");
//	                 fprintf(stdout, "( %d %d ) %s ", pin->defiPin::placementX(), pin->defiPin::placementY(), DEFOrientStr(pin->defiPin::orient()));
	                orientStr = DEFOrientStr(pin->defiPin::orient());
									originX = pin->defiPin::placementX();
									originY = pin->defiPin::placementY();
	             }
	             if (pin->defiPin::isFixed())
	             {
//	                 fprintf(stdout, "+ FIXED ");
//	                 fprintf(stdout, "( %d %d ) %s ", pin->defiPin::placementX(), pin->defiPin::placementY(), DEFOrientStr(pin->defiPin::orient()));
	                orientStr = DEFOrientStr(pin->defiPin::orient());
									originX = pin->defiPin::placementX();
									originY = pin->defiPin::placementY();
	             }
//	             if (pin->defiPin::isUnplaced())
//	                 fprintf(stdout, "+ UNPLACED ");
	         }
	        originX += lx;
	        originY += by;
	        int width = rx - lx;
	        int height = ty - by;
	        
					// Get and print the origin of the instance.
					if (strcmp(orientStr, "N") == 0)
					{
						x = originX - chipLeft;
						y = originY - chipBottom;
					}
					else if (strcmp(orientStr, "S") == 0)
					{
						x = originX - width - chipLeft;
						y = originY - height - chipBottom;
					}
					else if (strcmp(orientStr, "W") == 0)
					{
						x = originX - width - chipLeft;
						y = originY - chipBottom;
						int tmp = width;
						width = height;
						height = tmp;
					}
					else if (strcmp(orientStr, "E") == 0)
					{
						x = originX - chipLeft;
						y = originY - height - chipBottom;
						int tmp = width;
						width = height;
						height = tmp;
					}
					else if (strcmp(orientStr, "FN") == 0)
					{
						x = originX - width - chipLeft;
						y = originY - chipBottom;
					}
					else if (strcmp(orientStr, "FS") == 0)
					{
						x = originX - chipLeft;
						y = originY - height - chipBottom;
					}
					else if (strcmp(orientStr, "FW") == 0)
					{
						x = originX - chipLeft;
						y = originY - chipBottom;
						int tmp = width;
						width = height;
						height = tmp;
					}
					else if (strcmp(orientStr, "FE") == 0)
					{
						x = originX - width - chipLeft;
						y = originY - height - chipBottom;
						int tmp = width;
						width = height;
						height = tmp;
					}
					else
					{
						fprintf(stderr, "Error: unknown orient of cell instance: %s\n", orientStr);
					}
					
          Pad *pad = new Pad(padIndex++, padName, netName, padType, x, y, x+width, y+height);
          design->addPad(pad);
//	         if (pin->defiPin::hasSpecial())
//	         {
//	             fprintf(stdout, "+ SPECIAL ");
//	         }
//	         if (pin->hasAPinPartialMetalArea())
//	         {
//	             for (i = 0; i < pin->defiPin::numAPinPartialMetalArea(); i++)
//	             {
//	                fprintf(stdout, "ANTENNAPINPARTIALMETALAREA %d", pin->APinPartialMetalArea(i));
//	                if (*(pin->APinPartialMetalAreaLayer(i)))
//	                    fprintf(stdout, " LAYER %s", pin->APinPartialMetalAreaLayer(i));
//	                fprintf(stdout, "\n");
//	             }
//	         }
//	         if (pin->hasAPinPartialMetalSideArea())
//	         {
//	             for (i = 0; i < pin->defiPin::numAPinPartialMetalSideArea(); i++)
//	             {
//	                fprintf(stdout, "ANTENNAPINPARTIALMETALSIDEAREA %d", pin->APinPartialMetalSideArea(i));
//	                if (*(pin->APinPartialMetalSideAreaLayer(i)))
//	                    fprintf(stdout, " LAYER %s", pin->APinPartialMetalSideAreaLayer(i));
//	                fprintf(stdout, "\n");
//	             }
//	         }
//	         if (pin->hasAPinDiffArea())
//	         {
//	             for (i = 0; i < pin->defiPin::numAPinDiffArea(); i++)
//	             {
//	                fprintf(stdout, "ANTENNAPINDIFFAREA %d", pin->APinDiffArea(i));
//	                if (*(pin->APinDiffAreaLayer(i)))
//	                    fprintf(stdout, " LAYER %s", pin->APinDiffAreaLayer(i));
//	                fprintf(stdout, "\n");
//	             }
//	         }
//	         if (pin->hasAPinPartialCutArea())
//	         {
//	             for (i = 0; i < pin->defiPin::numAPinPartialCutArea(); i++)
//	             {
//	                fprintf(stdout, "ANTENNAPINPARTIALCUTAREA %d", pin->APinPartialCutArea(i));
//	                if (*(pin->APinPartialCutAreaLayer(i)))
//	                    fprintf(stdout, " LAYER %s", pin->APinPartialCutAreaLayer(i));
//	                fprintf(stdout, "\n");
//	             }
//	         }

//	         for (j = 0; j < pin->numAntennaModel(); j++)
//	         {
//	            aModel = pin->antennaModel(j);
//	 
//	            fprintf(stdout, "ANTENNAMODEL %s\n",
//	                    aModel->defiPinAntennaModel::antennaOxide()); 
//	 
//	            if (aModel->hasAPinGateArea())
//	            {
//	                for (i = 0; i < aModel->defiPinAntennaModel::numAPinGateArea(); i++)
//	                {
//	                   fprintf(stdout, "ANTENNAPINGATEAREA %d", aModel->APinGateArea(i));
//	                   if (aModel->hasAPinGateAreaLayer(i))
//	                       fprintf(stdout, " LAYER %s", aModel->APinGateAreaLayer(i));
//	                   fprintf(stdout, "\n");
//	                }
//	            }
//	            if (aModel->hasAPinMaxAreaCar()) {
//	                for (i = 0; i < aModel->defiPinAntennaModel::numAPinMaxAreaCar(); i++)
//	                {
//	                   fprintf(stdout, "ANTENNAPINMAXAREACAR %d", aModel->APinMaxAreaCar(i));
//	                   if (aModel->hasAPinMaxAreaCarLayer(i))
//	                       fprintf(stdout, " LAYER %s", aModel->APinMaxAreaCarLayer(i));
//	                   fprintf(stdout, "\n");
//	                }
//	            }
//	            if (aModel->hasAPinMaxSideAreaCar())
//	            {
//	                for (i = 0;
//	                     i < aModel->defiPinAntennaModel::numAPinMaxSideAreaCar(); 
//	                     i++) {
//	                   fprintf(stdout, "ANTENNAPINMAXSIDEAREACAR %d",
//	                           aModel->APinMaxSideAreaCar(i));
//	                   if (aModel->hasAPinMaxSideAreaCarLayer(i))
//	                       fprintf(stdout,
//	                           " LAYER %s", aModel->APinMaxSideAreaCarLayer(i));
//	                   fprintf(stdout, "\n");
//	                }
//	            }
//	            if (aModel->hasAPinMaxCutCar()) {
//	                for (i = 0; i < aModel->defiPinAntennaModel::numAPinMaxCutCar();
//	                   i++) {
//	                   fprintf(stdout, "ANTENNAPINMAXCUTCAR %d",
//	                       aModel->APinMaxCutCar(i));
//	                   if (aModel->hasAPinMaxCutCarLayer(i))
//	                       fprintf(stdout, " LAYER %s",
//	                       aModel->APinMaxCutCarLayer(i));
//	                   fprintf(stdout, "\n");
//	                }
//	            }
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END PINS\n");
	         break;
	      }
	      
	  case defrDefaultCapCbkType :
	         i = (long)cl;
//	         fprintf(stdout, "DEFAULTCAP %d\n", i);
	         numObjs = i;
	         break;
	  case defrRowCbkType :
	         row = (defiRow*)cl;
//	         fprintf(stdout, "ROW %s %s %g %g %s ", row->defiRow::name(),
//	                 row->defiRow::macro(), row->defiRow::x(), row->defiRow::y(),
//	                 DEFOrientStr(row->defiRow::orient()));
//	         if (row->defiRow::hasDo()) {
//	             fprintf(stdout, "DO %g BY %g ",
//	                     row->defiRow::xNum(), row->defiRow::yNum());
//	             if (row->defiRow::hasDoStep())
//	                 fprintf(stdout, "STEP %g %g ;\n",
//	                         row->defiRow::xStep(), row->defiRow::yStep());
//	             else
//	                 fprintf(stdout, ";\n");
//	         } else
//	            fprintf(stdout, ";\n");
//	         if (row->defiRow::numProps() > 0) {
//	            for (i = 0; i < row->defiRow::numProps(); i++) {
//	                fprintf(stdout, "  + PROPERTY %s %s ",
//	                        row->defiRow::propName(i),
//	                        row->defiRow::propValue(i));
//	                switch (row->defiRow::propType(i)) {
//	                   case 'R': fprintf(stdout, "REAL ");
//	                             break;
//	                   case 'I': fprintf(stdout, "INTEGER ");
//	                             break;
//	                   case 'S': fprintf(stdout, "STRING ");
//	                             break;
//	                   case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                             break;
//	                   case 'N': fprintf(stdout, "NUMBER ");
//	                             break;
//	                }
//	            }
//	            fprintf(stdout, ";\n");
//	         }
	         break;
	  case defrTrackCbkType :
	         track = (defiTrack*)cl;
//	         fprintf(stdout, "TRACKS %s %g DO %g STEP %g LAYER ",
//	                 track->defiTrack::macro(), track->defiTrack::x(),
//	                 track->defiTrack::xNum(), track->defiTrack::xStep());
//	         for (i = 0; i < track->defiTrack::numLayers(); i++)
//	            fprintf(stdout, "%s ", track->defiTrack::layer(i));
//	         fprintf(stdout, ";\n"); 
	         break;
	  case defrGcellGridCbkType :
	         gcg = (defiGcellGrid*)cl;
//	         fprintf(stdout, "GCELLGRID %s %d DO %d STEP %g ;\n",
//	                 gcg->defiGcellGrid::macro(), gcg->defiGcellGrid::x(),
//	                 gcg->defiGcellGrid::xNum(), gcg->defiGcellGrid::xStep());
	         break;
	  case defrViaCbkType :
	         via = (defiVia*)cl;
//	         fprintf(stdout, "- %s ", via->defiVia::name());
//	         if (via->defiVia::hasPattern())
//	             fprintf(stdout, "+ PATTERNNAME %s ", via->defiVia::pattern());
//	         for (i = 0; i < via->defiVia::numLayers(); i++) {
//	             via->defiVia::layer(i, &name, &xl, &yl, &xh, &yh);
//	             fprintf(stdout, "+ RECT %s %d %d %d %d \n",
//	                     name, xl, yl, xh, yh);
//	         }
//	         // POLYGON
//	         if (via->defiVia::numPolygons()) {
//	           struct defiPoints points;
//	           for (i = 0; i < via->defiVia::numPolygons(); i++) {
//	             fprintf(stdout, "\n  + POLYGON %s ", via->polygonName(i));
//	             points = via->getPolygon(i);
//	             for (j = 0; j < points.numPoints; j++)
//	               fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
//	           }
//	         }
//	         fprintf(stdout, " ;\n");
//	         if (via->defiVia::hasViaRule()) {
//	             char *vrn, *bl, *cl, *tl;
//	             int xs, ys, xcs, ycs, xbe, ybe, xte, yte;
//	             int cr, cc, xo, yo, xbo, ybo, xto, yto;
//	             (void)via->defiVia::viaRule(&vrn, &xs, &ys, &bl, &cl, &tl, &xcs,
//	                                         &ycs, &xbe, &ybe, &xte, &yte);
//	             fprintf(stdout, "+ VIARULE '%s'\n", vrn);
//	             fprintf(stdout, "  + CUTSIZE %d %d\n", xs, ys);
//	             fprintf(stdout, "  + LAYERS %s %s %s\n", bl, cl, tl);
//	             fprintf(stdout, "  + CUTSPACING %d %d\n", xcs, ycs);
//	             fprintf(stdout, "  + ENCLOSURE %d %d %d %d\n", xbe, ybe, xte, yte);
//	             if (via->defiVia::hasRowCol()) {
//	                (void)via->defiVia::rowCol(&cr, &cc);
//	                fprintf(stdout, "  + ROWCOL %d %d\n", cr, cc);
//	             }
//	             if (via->defiVia::hasOrigin()) {
//	                (void)via->defiVia::origin(&xo, &yo);
//	                fprintf(stdout, "  + ORIGIN %d %d\n", xo, yo);
//	             }
//	             if (via->defiVia::hasOffset()) {
//	                (void)via->defiVia::offset(&xbo, &ybo, &xto, &yto);
//	                fprintf(stdout, "  + OFFSET %d %d %d %d\n", xbo, ybo, xto, yto);
//	             }
//	             if (via->defiVia::hasCutPattern())
//	                fprintf(stdout, "  + PATTERN '%s'\n", via->defiVia::cutPattern());
//	         }
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END VIAS\n");
	         break;
	  case defrRegionCbkType :
	         re = (defiRegion*)cl;
//	         fprintf(stdout, "- %s ", re->defiRegion::name());
//	         for (i = 0; i < re->defiRegion::numRectangles(); i++)
//	             fprintf(stdout, "%d %d %d %d \n", re->defiRegion::xl(i),
//	                     re->defiRegion::yl(i), re->defiRegion::xh(i),
//	                     re->defiRegion::yh(i));
//	         if (re->defiRegion::hasType())
//	             fprintf(stdout, "+ TYPE %s\n", re->defiRegion::type());
//	         if (re->defiRegion::numProps()) {
//	             for (i = 0; i < re->defiRegion::numProps(); i++) {
//	                 fprintf(stdout, "+ PROPERTY %s %s ", re->defiRegion::propName(i),
//	                         re->defiRegion::propValue(i));
//	                 switch (re->defiRegion::propType(i)) {
//	                    case 'R': fprintf(stdout, "REAL ");
//	                              break;
//	                    case 'I': fprintf(stdout, "INTEGER ");
//	                              break;
//	                    case 'S': fprintf(stdout, "STRING ");
//	                              break;
//	                    case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                              break;
//	                    case 'N': fprintf(stdout, "NUMBER ");
//	                              break;
//	                 }
//	             }
//	         }
//	         fprintf(stdout, ";\n"); 
//	         --numObjs;
//	         if (numObjs <= 0) {
//	             fprintf(stdout, "END REGIONS\n");
//	         }
	         break;
	  case defrGroupNameCbkType :
//	         if ((char*)cl) {
//	             fprintf(stdout, "- %s", (char*)cl);
//	         }
	         break;
	  case defrGroupMemberCbkType :
//	         if ((char*)cl) {
//	             fprintf(stdout, " %s", (char*)cl);
//	         }
	         break;
	  case defrGroupCbkType :
	         group = (defiGroup*)cl;
//	         if (group->defiGroup::hasMaxX() | group->defiGroup::hasMaxY()
//	             | group->defiGroup::hasPerim()) {
//	             fprintf(stdout, "\n  + SOFT ");
//	             if (group->defiGroup::hasPerim()) 
//	                 fprintf(stdout, "MAXHALFPERIMETER %d ",
//	                         group->defiGroup::perim());
//	             if (group->defiGroup::hasMaxX())
//	                 fprintf(stdout, "MAXX %d ", group->defiGroup::maxX());
//	             if (group->defiGroup::hasMaxY()) 
//	                 fprintf(stdout, "MAXY %d ", group->defiGroup::maxY());
//	         } 
//	         if (group->defiGroup::hasRegionName())
//	             fprintf(stdout, "\n  + REGION %s ", group->defiGroup::regionName());
//	         if (group->defiGroup::hasRegionBox()) {
//	             int *gxl, *gyl, *gxh, *gyh;
//	             int size;
//	             group->defiGroup::regionRects(&size, &gxl, &gyl, &gxh, &gyh);
//	             for (i = 0; i < size; i++)
//	                 fprintf(stdout, "REGION %d %d %d %d ", gxl[i], gyl[i],
//	                         gxh[i], gyh[i]);
//	         }
//	         if (group->defiGroup::numProps()) {
//	             for (i = 0; i < group->defiGroup::numProps(); i++) {
//	                 fprintf(stdout, "\n  + PROPERTY %s %s ",
//	                         group->defiGroup::propName(i),
//	                         group->defiGroup::propValue(i));
//	                 switch (group->defiGroup::propType(i)) {
//	                    case 'R': fprintf(stdout, "REAL ");
//	                              break;
//	                    case 'I': fprintf(stdout, "INTEGER ");
//	                              break;
//	                    case 'S': fprintf(stdout, "STRING ");
//	                              break;
//	                    case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                              break;
//	                    case 'N': fprintf(stdout, "NUMBER ");
//	                              break;
//	                 }
//	             }
//	         }
//	         fprintf(stdout, " ;\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END GROUPS\n");
	         break;
	  case defrScanchainCbkType :
	         sc = (defiScanchain*)cl;
//	         fprintf(stdout, "- %s\n", sc->defiScanchain::name());
//	         if (sc->defiScanchain::hasStart()) {
//	             sc->defiScanchain::start(&a1, &b1);
//	             fprintf(stdout, "  + START %s %s\n", a1, b1);
//	         }
//	         if (sc->defiScanchain::hasStop()) {
//	             sc->defiScanchain::stop(&a1, &b1);
//	             fprintf(stdout, "  + STOP %s %s\n", a1, b1);
//	         }
//	         if (sc->defiScanchain::hasCommonInPin() ||
//	             sc->defiScanchain::hasCommonOutPin()) {
//	             fprintf(stdout, "  + COMMONSCANPINS ");
//	             if (sc->defiScanchain::hasCommonInPin())
//	                fprintf(stdout, " ( IN %s ) ", sc->defiScanchain::commonInPin());
//	             if (sc->defiScanchain::hasCommonOutPin())
//	                fprintf(stdout, " ( OUT %s ) ",sc->defiScanchain::commonOutPin());
//	             fprintf(stdout, "\n");
//	         }
//	         if (sc->defiScanchain::hasFloating()) {
//	            sc->defiScanchain::floating(&size, &inst, &inPin, &outPin, &bits);
//	            if (size > 0)
//	                fprintf(stdout, "  + FLOATING\n");
//	            for (i = 0; i < size; i++) {
//	                fprintf(stdout, "    %s ", inst[i]);
//	                if (inPin[i])
//	                   fprintf(stdout, "( IN %s ) ", inPin[i]);
//	                if (outPin[i])
//	                   fprintf(stdout, "( OUT %s ) ", outPin[i]);
//	                if (bits[i] != -1)
//	                   fprintf(stdout, "( BITS %d ) ", bits[i]);
//	                fprintf(stdout, "\n");
//	            }
//	         }
//	
//	         if (sc->defiScanchain::hasOrdered()) {
//	            for (i = 0; i < sc->defiScanchain::numOrderedLists(); i++) {
//	                sc->defiScanchain::ordered(i, &size, &inst, &inPin, &outPin,
//	                                           &bits);
//	                if (size > 0)
//	                    fprintf(stdout, "  + ORDERED\n");
//	                for (j = 0; j < size; j++) {
//	                    fprintf(stdout, "    %s ", inst[j]); 
//	                    if (inPin[j])
//	                       fprintf(stdout, "( IN %s ) ", inPin[j]);
//	                    if (outPin[j])
//	                       fprintf(stdout, "( OUT %s ) ", outPin[j]);
//	                    if (bits[j] != -1)
//	                       fprintf(stdout, "( BITS %d ) ", bits[j]);
//	                    fprintf(stdout, "\n");
//	                }
//	            }
//	         }
//	
//	         if (sc->defiScanchain::hasPartition()) {
//	            fprintf(stdout, "  + PARTITION %s ",
//	                    sc->defiScanchain::partitionName());
//	            if (sc->defiScanchain::hasPartitionMaxBits())
//	              fprintf(stdout, "MAXBITS %d ",
//	                      sc->defiScanchain::partitionMaxBits());
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END SCANCHAINS\n");
	         break;
	  case defrIOTimingCbkType :
	         iot = (defiIOTiming*)cl;
//	         fprintf(stdout, "- ( %s %s )\n", iot->defiIOTiming::inst(),
//	                 iot->defiIOTiming::pin());
//	         if (iot->defiIOTiming::hasSlewRise())
//	             fprintf(stdout, "  + RISE SLEWRATE %g %g\n",
//	                     iot->defiIOTiming::slewRiseMin(),
//	                     iot->defiIOTiming::slewRiseMax());
//	         if (iot->defiIOTiming::hasSlewFall())
//	             fprintf(stdout, "  + FALL SLEWRATE %g %g\n",
//	                     iot->defiIOTiming::slewFallMin(),
//	                     iot->defiIOTiming::slewFallMax());
//	         if (iot->defiIOTiming::hasVariableRise())
//	             fprintf(stdout, "  + RISE VARIABLE %g %g\n",
//	                     iot->defiIOTiming::variableRiseMin(),
//	                     iot->defiIOTiming::variableRiseMax());
//	         if (iot->defiIOTiming::hasVariableFall())
//	             fprintf(stdout, "  + FALL VARIABLE %g %g\n",
//	                     iot->defiIOTiming::variableFallMin(),
//	                     iot->defiIOTiming::variableFallMax());
//	         if (iot->defiIOTiming::hasCapacitance())
//	             fprintf(stdout, "  + CAPACITANCE %g\n",
//	                     iot->defiIOTiming::capacitance());
//	         if (iot->defiIOTiming::hasDriveCell()) {
//	             fprintf(stdout, "  + DRIVECELL %s ",
//	                     iot->defiIOTiming::driveCell());
//	             if (iot->defiIOTiming::hasFrom())
//	                 fprintf(stdout, "  FROMPIN %s ",
//	                         iot->defiIOTiming::from());
//	             if (iot->defiIOTiming::hasTo())
//	                 fprintf(stdout, "  TOPIN %s ",
//	                         iot->defiIOTiming::to());
//	             if (iot->defiIOTiming::hasParallel())
//	                 fprintf(stdout, "PARALLEL %g",
//	                         iot->defiIOTiming::parallel());
//	             fprintf(stdout, "\n");
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END IOTIMINGS\n");
	         break;
	  case defrFPCCbkType :
	         fpc = (defiFPC*)cl;
//	         fprintf(stdout, "- %s ", fpc->defiFPC::name());
//	         if (fpc->defiFPC::isVertical())
//	             fprintf(stdout, "VERTICAL ");
//	         if (fpc->defiFPC::isHorizontal())
//	             fprintf(stdout, "HORIZONTAL ");
//	         if (fpc->defiFPC::hasAlign())
//	             fprintf(stdout, "ALIGN ");
//	         if (fpc->defiFPC::hasMax())
//	             fprintf(stdout, "%g ", fpc->defiFPC::alignMax());
//	         if (fpc->defiFPC::hasMin())
//	             fprintf(stdout, "%g ", fpc->defiFPC::alignMin());
//	         if (fpc->defiFPC::hasEqual())
//	             fprintf(stdout, "%g ", fpc->defiFPC::equal());
//	         for (i = 0; i < fpc->defiFPC::numParts(); i++) {
//	             fpc->defiFPC::getPart(i, &corner, &typ, &name);
//	             if (corner == 'B')
//	                 fprintf(stdout, "BOTTOMLEFT ");
//	             else
//	                 fprintf(stdout, "TOPRIGHT ");
//	             if (typ == 'R')
//	                 fprintf(stdout, "ROWS %s ", name);
//	             else
//	                 fprintf(stdout, "COMPS %s ", name);
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END FLOORPLANCONSTRAINTS\n");
	         break;
	  case defrTimingDisableCbkType :
	         td = (defiTimingDisable*)cl;
//	         if (td->defiTimingDisable::hasFromTo())
//	             fprintf(stdout, "- FROMPIN %s %s ",
//	                     td->defiTimingDisable::fromInst(),
//	                     td->defiTimingDisable::fromPin(),
//	                     td->defiTimingDisable::toInst(),
//	                     td->defiTimingDisable::toPin());
//	         if (td->defiTimingDisable::hasThru())
//	             fprintf(stdout, "- THRUPIN %s %s ",
//	                     td->defiTimingDisable::thruInst(),
//	                     td->defiTimingDisable::thruPin());
//	         if (td->defiTimingDisable::hasMacroFromTo())
//	             fprintf(stdout, "- MACRO %s FROMPIN %s %s ",
//	                     td->defiTimingDisable::macroName(),
//	                     td->defiTimingDisable::fromPin(),
//	                     td->defiTimingDisable::toPin());
//	         if (td->defiTimingDisable::hasMacroThru())
//	             fprintf(stdout, "- MACRO %s THRUPIN %s %s ",
//	                     td->defiTimingDisable::macroName(),
//	                     td->defiTimingDisable::fromPin());
//	         fprintf(stdout, ";\n");
	         break;
	  case defrPartitionCbkType :
	         part = (defiPartition*)cl;
//	         fprintf(stdout, "- %s ", part->defiPartition::name());
//	         if (part->defiPartition::isSetupRise() |
//	             part->defiPartition::isSetupFall() |
//	             part->defiPartition::isHoldRise() |
//	             part->defiPartition::isHoldFall()) {
//	             // has turnoff 
//	             fprintf(stdout, "TURNOFF "); 
//	             if (part->defiPartition::isSetupRise())
//	                 fprintf(stdout, "SETUPRISE "); 
//	             if (part->defiPartition::isSetupFall())
//	                 fprintf(stdout, "SETUPFALL "); 
//	             if (part->defiPartition::isHoldRise())
//	                 fprintf(stdout, "HOLDRISE "); 
//	             if (part->defiPartition::isHoldFall())
//	                 fprintf(stdout, "HOLDFALL "); 
//	         }
//	         itemT = part->defiPartition::itemType();
//	         dir = part->defiPartition::direction();
//	         if (strcmp(itemT, "CLOCK") == 0) {
//	             if (dir == 'T')    // toclockpin
//	                 fprintf(stdout, "+ TOCLOCKPIN %s %s ",
//	                         part->defiPartition::instName(),
//	                         part->defiPartition::pinName());
//	             if (dir == 'F')    // fromclockpin
//	                 fprintf(stdout, "+ FROMCLOCKPIN %s %s ",
//	                         part->defiPartition::instName(),
//	                         part->defiPartition::pinName());
//	             if (part->defiPartition::hasMin())
//	                 fprintf(stdout, "MIN %g %g ",
//	                         part->defiPartition::partitionMin(),
//	                         part->defiPartition::partitionMax());
//	             if (part->defiPartition::hasMax())
//	                 fprintf(stdout, "MAX %g %g ",
//	                         part->defiPartition::partitionMin(),
//	                         part->defiPartition::partitionMax());
//	             fprintf(stdout, "PINS ");
//	             for (i = 0; i < part->defiPartition::numPins(); i++)
//	                  fprintf(stdout, "%s ", part->defiPartition::pin(i));
//	         } else if (strcmp(itemT, "IO") == 0) {
//	             if (dir == 'T')    // toiopin
//	                 fprintf(stdout, "+ TOIOPIN %s %s ",
//	                         part->defiPartition::instName(),
//	                         part->defiPartition::pinName());
//	             if (dir == 'F')    // fromiopin
//	                 fprintf(stdout, "+ FROMIOPIN %s %s ",
//	                         part->defiPartition::instName(),
//	                         part->defiPartition::pinName());
//	         } else if (strcmp(itemT, "COMP") == 0) {
//	             if (dir == 'T')    // tocomppin
//	                 fprintf(stdout, "+ TOCOMPPIN %s %s ",
//	                         part->defiPartition::instName(),
//	                         part->defiPartition::pinName());
//	             if (dir == 'F')    // fromcomppin
//	                 fprintf(stdout, "+ FROMCOMPPIN %s %s ",
//	                         part->defiPartition::instName(),
//	                         part->defiPartition::pinName());
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END PARTITIONS\n");
	         break;
//	
	  case defrPinPropCbkType :
	         pprop = (defiPinProp*)cl;
//	         if (pprop->defiPinProp::isPin())
//	            fprintf(stdout, "- PIN %s ", pprop->defiPinProp::pinName());
//	         else 
//	            fprintf(stdout, "- %s %s ",
//	                    pprop->defiPinProp::instName(),
//	                    pprop->defiPinProp::pinName());
//	         fprintf(stdout, ";\n");
//	         if (pprop->defiPinProp::numProps() > 0) {
//	            for (i = 0; i < pprop->defiPinProp::numProps(); i++) {
//	                fprintf(stdout, "  + PROPERTY %s %s ",
//	                        pprop->defiPinProp::propName(i),
//	                        pprop->defiPinProp::propValue(i));
//	                switch (pprop->defiPinProp::propType(i)) {
//	                   case 'R': fprintf(stdout, "REAL ");
//	                             break;
//	                   case 'I': fprintf(stdout, "INTEGER ");
//	                             break;
//	                   case 'S': fprintf(stdout, "STRING ");
//	                             break;
//	                   case 'Q': fprintf(stdout, "QUOTESTRING ");
//	                             break;
//	                   case 'N': fprintf(stdout, "NUMBER ");
//	                             break;
//	                }
//	            }
//	            fprintf(stdout, ";\n");
//	         }
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END PINPROPERTIES\n");
	         break;
	  case defrBlockageCbkType :
	         block = (defiBlockage*)cl;
//	         if (block->defiBlockage::hasLayer()) {
//	            fprintf(stdout, "- LAYER %s\n", block->defiBlockage::layerName());
//	            if (block->defiBlockage::hasComponent())
//	               fprintf(stdout, "   + COMPONENT %s\n",
//	                       block->defiBlockage::layerComponentName());
//	            if (block->defiBlockage::hasSlots())
//	               fprintf(stdout, "   + SLOTS\n");
//	            if (block->defiBlockage::hasFills())
//	               fprintf(stdout, "   + FILLS\n");
//	            if (block->defiBlockage::hasPushdown())
//	               fprintf(stdout, "   + PUSHDOWN\n");
//	            if (block->defiBlockage::hasExceptpgnet())
//	               fprintf(stdout, "   + EXCEPTPGNET\n");
//	            if (block->defiBlockage::hasSpacing())
//	               fprintf(stdout, "   + SPACING %d\n",
//	                       block->defiBlockage::minSpacing());
//	            if (block->defiBlockage::hasDesignRuleWidth())
//	               fprintf(stdout, "   + DESIGNRULEWIDTH %d\n",
//	                       block->defiBlockage::designRuleWidth());
//	         }
//	         else if (block->defiBlockage::hasPlacement()) {
//	            fprintf(stdout, "- PLACEMENT\n");
//	            if (block->defiBlockage::hasSoft())
//	               fprintf(stdout, "   + SOFT\n");
//	            if (block->defiBlockage::hasPartial())
//	               fprintf(stdout, "   + PARTIAL %g\n",
//	                       block->defiBlockage::placementMaxDensity());
//	            if (block->defiBlockage::hasComponent())
//	               fprintf(stdout, "   + COMPONENT %s\n",
//	                       block->defiBlockage::placementComponentName());
//	            if (block->defiBlockage::hasPushdown())
//	               fprintf(stdout, "   + PUSHDOWN\n");
//	         }
//	
//	         for (i = 0; i < block->defiBlockage::numRectangles(); i++) {
//	            fprintf(stdout, "   RECT %d %d %d %d\n", 
//	                    block->defiBlockage::xl(i), block->defiBlockage::yl(i),
//	                    block->defiBlockage::xh(i), block->defiBlockage::yh(i));
//	         } 
//	
//	         for (i = 0; i < block->defiBlockage::numPolygons(); i++) {
//	            fprintf(stdout, "   POLYGON ");
//	            points = block->getPolygon(i);
//	            for (j = 0; j < points.numPoints; j++)
//	               fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
//	            fprintf(stdout, "\n");
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END BLOCKAGES\n");
	         break;
	  case defrSlotCbkType :
	         slots = (defiSlot*)cl;
//	         if (slots->defiSlot::hasLayer())
//	            fprintf(stdout, "- LAYER %s\n", slots->defiSlot::layerName());
//	
//	         for (i = 0; i < slots->defiSlot::numRectangles(); i++) {
//	            fprintf(stdout, "   RECT %d %d %d %d\n", 
//	                    slots->defiSlot::xl(i), slots->defiSlot::yl(i),
//	                    slots->defiSlot::xh(i), slots->defiSlot::yh(i));
//	         } 
//	         for (i = 0; i < slots->defiSlot::numPolygons(); i++) {
//	            fprintf(stdout, "   POLYGON ");
//	            points = slots->getPolygon(i);
//	            for (j = 0; j < points.numPoints; j++)
//	              fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
//	            fprintf(stdout, ";\n");
//	         }
//	         fprintf(stdout, ";\n");
//	         --numObjs;
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END SLOTS\n");
	         break;
	  case defrFillCbkType :
	         fills = (defiFill*)cl;
//	         if (fills->defiFill::hasLayer()) {
//	            fprintf(stdout, "- LAYER %s", fills->defiFill::layerName());
//	            if (fills->defiFill::hasLayerOpc())
//	               fprintf(stdout, " + OPC");
//	            fprintf(stdout, "\n");
//	
//	            for (i = 0; i < fills->defiFill::numRectangles(); i++) {
//	               fprintf(stdout, "   RECT %d %d %d %d\n", 
//	                       fills->defiFill::xl(i), fills->defiFill::yl(i),
//	                       fills->defiFill::xh(i), fills->defiFill::yh(i));
//	            } 
//	            for (i = 0; i < fills->defiFill::numPolygons(); i++) {
//	               fprintf(stdout, "   POLYGON "); 
//	               points = fills->getPolygon(i);
//	               for (j = 0; j < points.numPoints; j++)
//	                 fprintf(stdout, "%d %d ", points.x[j], points.y[j]);
//	               fprintf(stdout, ";\n");
//	            } 
//	            fprintf(stdout, ";\n");
//	         }
//	         --numObjs;
//	         if (fills->defiFill::hasVia()) {
//	            fprintf(stdout, "- VIA %s", fills->defiFill::viaName());
//	            if (fills->defiFill::hasViaOpc())
//	               fprintf(stdout, " + OPC");
//	            fprintf(stdout, "\n");
//	
//	            for (i = 0; i < fills->defiFill::numViaPts(); i++) {
//	               points = fills->getViaPts(i);
//	               for (j = 0; j < points.numPoints; j++)
//	                  fprintf(stdout, " %d %d", points.x[j], points.y[j]);
//	               fprintf(stdout, ";\n"); 
//	            }
//	            fprintf(stdout, ";\n");
//	         }
//	         if (numObjs <= 0)
//	             fprintf(stdout, "END FILLS\n");
	         break;
	  case defrStylesCbkType :
        {
         struct defiPoints points;
         styles = (defiStyles*)cl;
         points = styles->defiStyles::getPolygon();
         break;
	    }
      default:
//	  	fprintf(stdout, "BOGUS callback to DEFCls.\n"); 
	  	return 1;
	  }
	  return 0;
	}

	int DEFDn(defrCallbackType_e c, const char* h, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "DIVIDERCHAR \"%s\" ;\n",h);
		}
	  return 0;
	}

	int DEFExt(defrCallbackType_e pin, const char* c, defiUserData ud) {
	  char* name;
	
	  DEFCheckType(pin);
	
	  switch (pin) {
	  case defrNetExtCbkType : name = DEFAddress("net"); break;
	  case defrComponentExtCbkType : name = DEFAddress("component"); break;
	  case defrPinExtCbkType : name = DEFAddress("pin"); break;
	  case defrViaExtCbkType : name = DEFAddress("via"); break;
	  case defrNetConnectionExtCbkType : name = DEFAddress("net connection"); break;
	  case defrGroupExtCbkType : name = DEFAddress("group"); break;
	  case defrScanChainExtCbkType : name = DEFAddress("scanchain"); break;
	  case defrIoTimingsExtCbkType : name = DEFAddress("io timing"); break;
	  case defrPartitionsExtCbkType : name = DEFAddress("partition"); break;
	  default: name = DEFAddress("BOGUS"); return 1;
	  }
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "  %s DEFExtension %s\n", name, c);
		}
	  return 0;
	}

	int DEFExtension(defrCallbackType_e c, const char* extsn, defiUserData ud) {
	  DEFCheckType(c);
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
		  fprintf(stdout, "BEGINEXT %s\n", extsn);
		}
	  return 0;
	}

	void* DEFMallocCB(long unsigned int size) {
	  return malloc(size);
	}

	void* DEFReallocCB(void* name, long unsigned int size) {
	  return realloc(name, size);
	}

	void DEFFreeCB(void* name) {
	  free(name);
	  return;
	}

	void DEFLineNumberCB(int lineNo) {
	  fprintf(stdout, "Parsed %d number of lines!!\n", lineNo);
	  return;
	}

	int DEFUnitsfunc(defrCallbackType_e c, double units, defiUserData ud)
	{
		Design *design = (Design *)ud;
		Verbosity & v = design->getVerb();
		if (v.getForMajStats() > 2)
		{
			fprintf(stdout, "LEF/DEF factor: %f\n", units);
		}
		
		design->setLefDefFactor(units);
		return 0;
	}

	void LDReader::readDef(Design *design)
	{
	  FILE* f;
		Verbosity & v = design->getVerb();
		
    defrSetUserData((void*)3);
    defrSetComponentCbk(DEFCompf);
    defrSetUnitsCbk(DEFUnitsfunc);
    defrSetDesignCbk(DEFDname);
    defrSetTechnologyCbk(DEFTname);
    defrSetExtensionCbk(DEFExtension);
    defrSetDesignEndCbk(DEFDone);
    defrSetPropDefStartCbk(DEFPropstart);
    defrSetPropCbk(DEFProp);
    defrSetPropDefEndCbk(DEFPropend);
    defrSetNetCbk(DEFNetf);
    defrSetNetNameCbk(DEFNetNamef);
    defrSetNetNonDefaultRuleCbk(DEFNondefRulef);
    defrSetNetSubnetNameCbk(DEFSubnetNamef);
    defrSetNetPartialPathCbk(DEFNetpath);
    defrSetSNetCbk(DEFSnetf);
    defrSetSNetPartialPathCbk(DEFSnetpath);
    defrSetSNetWireCbk(DEFSnetwire);
    defrSetAddPathToNet();
    defrSetHistoryCbk(DEFHist);
    defrSetConstraintCbk(DEFConstraint);
    defrSetAssertionCbk(DEFConstraint);
    defrSetArrayNameCbk(DEFAn);
    defrSetFloorPlanNameCbk(DEFFn);
    defrSetDividerCbk(DEFDn);
    defrSetBusBitCbk(DEFBbn);
    defrSetNonDefaultCbk(DEFNdr);
    defrSetAssertionsStartCbk(DEFConstraintst);
    defrSetConstraintsStartCbk(DEFConstraintst);
    defrSetComponentStartCbk(DEFCs);
    defrSetPinPropStartCbk(DEFCs);
    defrSetNetStartCbk(DEFCs);
    defrSetStartPinsCbk(DEFCs);
    defrSetViaStartCbk(DEFCs);
    defrSetRegionStartCbk(DEFCs);
    defrSetSNetStartCbk(DEFCs);
    defrSetGroupsStartCbk(DEFCs);
    defrSetScanchainsStartCbk(DEFCs);
    defrSetIOTimingsStartCbk(DEFCs);
    defrSetFPCStartCbk(DEFCs);
    defrSetTimingDisablesStartCbk(DEFCs);
    defrSetPartitionsStartCbk(DEFCs);
    defrSetBlockageStartCbk(DEFCs);
    defrSetSlotStartCbk(DEFCs);
    defrSetFillStartCbk(DEFCs);
    defrSetNonDefaultStartCbk(DEFCs);
    defrSetStylesStartCbk(DEFCs);

    // All of the extensions point to the same function.
    defrSetNetExtCbk(DEFExt);
    defrSetComponentExtCbk(DEFExt);
    defrSetPinExtCbk(DEFExt);
    defrSetViaExtCbk(DEFExt);
    defrSetNetConnectionExtCbk(DEFExt);
    defrSetGroupExtCbk(DEFExt);
    defrSetScanChainExtCbk(DEFExt);
    defrSetIoTimingsExtCbk(DEFExt);
    defrSetPartitionsExtCbk(DEFExt);

		defrSetVersionStrCbk(DEFVersStr);
    defrSetCaseSensitiveCbk(DEFCasesens);

    // The following calls are an example of using one function "DEFCls"
    // to be the callback for many DIFFERENT types of constructs.
    // We have to cast the function type to meet the requirements
    // of each different set function.
    defrSetSiteCbk((defrSiteCbkFnType)DEFCls);
    defrSetCanplaceCbk((defrSiteCbkFnType)DEFCls);
    defrSetCannotOccupyCbk((defrSiteCbkFnType)DEFCls);
    defrSetDieAreaCbk((defrBoxCbkFnType)DEFCls);
    defrSetPinCapCbk((defrPinCapCbkFnType)DEFCls);
    defrSetPinCbk((defrPinCbkFnType)DEFCls);
    defrSetPinPropCbk((defrPinPropCbkFnType)DEFCls);
    defrSetDefaultCapCbk((defrIntegerCbkFnType)DEFCls);
    defrSetRowCbk((defrRowCbkFnType)DEFCls);
    defrSetTrackCbk((defrTrackCbkFnType)DEFCls);
    defrSetGcellGridCbk((defrGcellGridCbkFnType)DEFCls);
    defrSetViaCbk((defrViaCbkFnType)DEFCls);
    defrSetRegionCbk((defrRegionCbkFnType)DEFCls);
    defrSetGroupNameCbk((defrStringCbkFnType)DEFCls);
    defrSetGroupMemberCbk((defrStringCbkFnType)DEFCls);
    defrSetGroupCbk((defrGroupCbkFnType)DEFCls);
    defrSetScanchainCbk((defrScanchainCbkFnType)DEFCls);
    defrSetIOTimingCbk((defrIOTimingCbkFnType)DEFCls);
    defrSetFPCCbk((defrFPCCbkFnType)DEFCls);
    defrSetTimingDisableCbk((defrTimingDisableCbkFnType)DEFCls);
    defrSetPartitionCbk((defrPartitionCbkFnType)DEFCls);
    defrSetBlockageCbk((defrBlockageCbkFnType)DEFCls);
    defrSetSlotCbk((defrSlotCbkFnType)DEFCls);
    defrSetFillCbk((defrFillCbkFnType)DEFCls);
    defrSetStylesCbk((defrStylesCbkFnType)DEFCls);

    defrSetAssertionsEndCbk(DEFEndfunc);
    defrSetComponentEndCbk(DEFEndfunc);
    defrSetConstraintsEndCbk(DEFEndfunc);
    defrSetNetEndCbk(DEFEndfunc);
    defrSetFPCEndCbk(DEFEndfunc);
    defrSetFPCEndCbk(DEFEndfunc);
    defrSetGroupsEndCbk(DEFEndfunc);
    defrSetIOTimingsEndCbk(DEFEndfunc);
    defrSetNetEndCbk(DEFEndfunc);
    defrSetPartitionsEndCbk(DEFEndfunc);
    defrSetRegionEndCbk(DEFEndfunc);
    defrSetSNetEndCbk(DEFEndfunc);
    defrSetScanchainsEndCbk(DEFEndfunc);
    defrSetPinEndCbk(DEFEndfunc);
    defrSetTimingDisablesEndCbk(DEFEndfunc);
    defrSetViaEndCbk(DEFEndfunc);
    defrSetPinPropEndCbk(DEFEndfunc);
    defrSetBlockageEndCbk(DEFEndfunc);
    defrSetSlotEndCbk(DEFEndfunc);
    defrSetFillEndCbk(DEFEndfunc);
    defrSetNonDefaultEndCbk(DEFEndfunc);
    defrSetStylesEndCbk(DEFEndfunc);
    
    defrSetMallocFunction(DEFMallocCB);
    defrSetReallocFunction(DEFReallocCB);
    defrSetFreeFunction(DEFFreeCB);
		
		if (v.getForActions() > 0)
		{
	    defrSetLineNumberFunction(DEFLineNumberCB);
	    defrSetDeltaNumberLines(50000);
	  }
		
    //defrSetRegisterUnusedCallbacks();
		
    // Testing to set the number of warnings
    defrSetAssertionWarnings(3);
    defrSetBlockageWarnings(3);
    defrSetCaseSensitiveWarnings(3);
    defrSetComponentWarnings(3);
    defrSetConstraintWarnings(0);
    defrSetDefaultCapWarnings(3);
    defrSetGcellGridWarnings(3);
    defrSetIOTimingWarnings(3);
    defrSetNetWarnings(3);
    defrSetNonDefaultWarnings(3);
    defrSetPinExtWarnings(3);
    defrSetPinWarnings(3);
    defrSetRegionWarnings(3);
    defrSetRowWarnings(3);
    defrSetScanchainWarnings(3);
    defrSetSNetWarnings(3);
    defrSetStylesWarnings(3);
    defrSetTrackWarnings(3);
    defrSetUnitsWarnings(3);
    defrSetVersionWarnings(3);
    defrSetViaWarnings(3);
    
		defrInit();
		
		for (int i = 0; i < design->getDefFileNum(); ++ i)
		{
			if(v.getForSysRes() > 0)
			{
				MemUsage mu;
				
				double availMem = VMemUsage::getPhysTotal();
				availMem = min(availMem, static_cast<double>(1UL << (sizeof(void*)*8 - 20)));
				
				cout << "==================Mem info=================="<<endl
							<<"Current memory usage is " << mu.getEstimate() << "MB" << endl
							<< "Available physical memory is " << availMem << "MB" << endl
							<<"============================================"<<endl;
	    }
			string defName = design->getDefName(i);
	    if ((f = fopen(defName.c_str(),"r")) == 0)
	    {
	      fprintf(stderr, "Cannot open input file '%s'\n", defName.c_str());
	      continue;
	    }
			
			if (v.getForActions() > 0)
			{
		    fprintf(stdout, "\nStart to read in the def file %s ...\n", defName.c_str());
		  }
			defrReset();
			
			int res = defrRead(f, defName.c_str(), (void*)design, 1);
			if (res)
			{
				fprintf(stderr, "Reader returns bad status.\n");
				exit(-1);
			}
			
//			(void)defrPrintUnusedCallbacks(stdout);
			(void)defrReleaseNResetMemory();
			
			fclose(f);
		}
		
		//remove all gates with no SIGNAL nets or with specified cell name
		int gIndex = 0;
    GateMap &gateMap = design->getGateMap();
		for (GateMapItr itr = gateMap.begin(); itr != gateMap.end(); ++ itr)
		{
			Gate *gate = (*itr).second;
			if (gate->getNetNum() == 0)
			{
		  	fprintf(stdout, "\nWarning: remove gate %s without signal nets\n", gate->getName().c_str());
		  	delete gate;
			}
			else
			{
				gate->setId(gIndex++);
				design->addChipGate(gate);
			}
		}
		gateMap.clear();
		
    //there construct the netlist graph
    int netNum = design->getNetNum();
		if (v.getForMajStats() > 0)
		{
	    cout<<"Total number of nets: "<<design->getExpNetNum()<<endl;
	    cout<<"Number of signal nets: "<<netNum<<endl;
	    cout<<"Number of components: "<<design->getExpGateNum()<<endl;
	    cout<<"Number of actual cell instances: "<<design->getGateNum()<<endl;
	    cout<<"Number of pads: "<<design->getPadNum()<<endl;
	  }
		
	  char str[1024];
	  sprintf(str, "Error in readDef(): %d nets expected in DEF file, actually read %d nets", design->getExpNetNum(), netNum);
    abkassert(design->getExpNetNum() >= netNum, str);
	  sprintf(str, "Error in readDef(): %d cell instances expected in DEF file, actually read %d cell instances", design->getExpGateNum(), design->getGateNum());
		abkassert(design->getExpGateNum() >= design->getGateNum(), str);
	  sprintf(str, "Error in readDef(): %d PIs/POs expected in DEF file, actually read %d PIs/POs", design->getExpPortNum(), design->getPadNum());
		abkassert(design->getExpPortNum() == design->getPadNum(), str);
		
		if (netNum <= 5 || design->getGateNum() <= 5)
		{
			cout<<"Too few nets or cell instances in the design. Exiting ..."<<endl;
			exit(0);
		}
		
    int subnetIndex = 0;
    for (int i = 0; i < netNum; ++ i)
    {
      Net *n = design->getNet(i);
      abkassert(n, "Error in readDef()");
      n->checkPins();
      int nId = n->getId();
      int padNum = n->getPadNum();
      int pinNum = n->getPinNum();
      if (pinNum == 0)
      {
	    	fprintf(stdout, "\nWarning: Net %s has no pins\n\n", n->getName().c_str());
      	continue;
      }
			for (int j = 0; j < pinNum; ++ j)
			{
				Pin *tmpP = n->getPin(j);
				Gate *tmpG = tmpP->getGate();
				abkassert(tmpG != NULL, "Error in readDef(): pin is not related to cell instance");
				tmpG->incEffPinNum(1);
		    design->addPin(tmpP);
	  	}
			
			if (v.getForMajStats() > 2)
			{
				cout<<"Net "<<i<<" Name: "<<n->getName()<<endl;
				cout<<"Pad number: "<<padNum<<endl;
				cout<<"Pin number: "<<pinNum<<endl;
			}
			
      //consider the input and output pads
      //connect pads to cells
      
      bool PIExist = false;
      if (padNum > 0)
      {
        for (int j = 0; j < padNum; j ++)
        {
          Pad *pad = n->getPad(j);
          abkassert(pad, "Error in readDef()");
          string pName = pad->getName();
          for (int k = 0; k < pinNum; k ++)
          {
            Pin *pin = n->getPin(k);
            abkassert(pin, "Error in readDef()");
            string pinName = pin->getName();
            Gate *g = pin->getGate();
            abkassert(g, "Error in readDef()");
            string gName = g->getName();
            if (pad->getPadType() == PrimiaryInput || pad->getPadType() == InputOutput)
            {
            	PIExist = true;
            	Pin *sPin = n->getSourcePin();
            	if (sPin)
            		sPin->setType(INOUT);
              string sName = pName + "-" + gName + "/" + pinName;
            	if (pin->getType() == OUTPUT)
            	{
            		pin->setType(INOUT);
            	}
              Subnet *subnet = new Subnet(subnetIndex++, sName, true, false);
              subnet->setNetId(nId);
              subnet->setInputPad(pad);
              subnet->setOutputPin(pin);
              design->addSubnet(subnet);
              g->addInputSubnet(subnet);
              pin->addSubnet(subnet);
//              g->addFaninPad(pad);
//                        cout<<"Pad "<<pad->getName()<<" => "<<pin->getGate()->getName()<<"."<<pin->getName()<<endl;
            }
            else
            {
            	abkassert(pad->getPadType() == PrimiaryOutput, "Error in readDef()");
              string sName = gName + "/" + pinName + "-" + pName;
            	if (pin->getType() == INPUT)
            	{
//                      		cout<<"Warning: output pad "<<pad->getName()<<" connects to input pin "<<sName<<endl;
//                      		getchar();
            		continue;
            	}
              Subnet *subnet = new Subnet(subnetIndex++, sName, false, true);
              subnet->setNetId(nId);
              subnet->setOutputPad(pad);
              subnet->setInputPin(pin);
              design->addSubnet(subnet);
              g->addOutputSubnet(subnet);
              pin->addSubnet(subnet);
//                        cout<<pin->getGate()->getName()<<"."<<pin->getName()<<" => Pad "<<pad->getName()<<" Gate Type: "<<g->getGateType()<<endl;
//              g->addFanoutPad(pad);
            }
          }
        }
      }
      
      if (!PIExist)
      {
				if (n->getSourcePin() == NULL)
				{
					n->makeSourcePin();
				}
      }
      n->makeSinkPins();
      
      //connects between cells
      Pin *sourcePin = n->getSourcePin();
      if (sourcePin)
      {
        abkassert(sourcePin->getType() == OUTPUT || sourcePin->getType() == INOUT, "Error in readDef()");
        string sPinName = sourcePin->getName();
        Gate *sg = sourcePin->getGate();
        abkassert(sg, "Error in readDef()");
        string sGName = sg->getName();
        for (int j = 1; j < n->getPinNum(); j ++)
        {
          Pin *sink = n->getPin(j);
          abkassert(sink, "Error in readDef()");
          abkassert(sink->getType() == INPUT || sink->getType() == INOUT, "Error in readDef()");
          string tPinName = sink->getName();
          Gate *tg = sink->getGate();
          abkassert(tg, "Error in readDef()");
          string tGName = tg->getName();
          string sName = sGName + "/" + sPinName + "-" + tGName + "/" + tPinName;
          Subnet *subnet = new Subnet(subnetIndex++, sName, false, false);
          subnet->setNetId(nId);
          subnet->setInputPin(sourcePin);
          subnet->setOutputPin(sink);
          design->addSubnet(subnet);
          sg->addOutputSubnet(subnet);
          tg->addInputSubnet(subnet);
          sourcePin->addSubnet(subnet);
          sink->addSubnet(subnet);
//                  cout<<sourcePin->getGate()->getName()<<"."<<sourcePin->getName()<<" => "<<sink->getGate()->getName()<<"."<<sink->getName()<<endl;
          abkassert(sourcePin->getGate(), "Error in readDef()");
          abkassert(sink->getGate(), "Error in readDef()");
//          sourcePin->getGate()->addFanoutGate(sink->getGate());
//          sink->getGate()->addFaninGate(sourcePin->getGate());
          sourcePin->getGate()->addAdjGate(sink->getGate());
          sink->getGate()->addAdjGate(sourcePin->getGate());
        }
      }
    }
		
		if (v.getForActions() > 0)
		{
			fprintf(stdout, "Finished reading def files\n");
		}
	}
	
  void LDReader::readDesign(Design *design)
  {
  	readLef(design);
  	readDef(design);
	}
  
}
