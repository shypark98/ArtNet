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

#ifndef __MODULE_H__
#define __MODULE_H__

#include <cassert>
#include <cstdio>

#include "Gate.h"
#include "typedefs.h"
#include "Net.h"

namespace DESIGN {
  class Gate;
  
  class Module {
  public:
    //constructors
    Module(int i): id(i)
    {
    }
    
    //modifiers
    void setId(int i) { id = i; }
    void setAvgG(double aG) { avgG = aG; }
    void setGeomAvgG(double aG) { geomAvgG = aG; }
    void setAvgT1(double aT1) { avgT1 = aT1; }
    void setAvgT2(double aT2) { avgT2 = aT2; }
    void setAvgT3(double aT3) { avgT3 = aT3; }
    void setAvgT4(double aT4) { avgT4 = aT4; }
    void setGeomAvgT1(double aT1) { geomAvgT1 = aT1; }
    void setGeomAvgT2(double aT2) { geomAvgT2 = aT2; }
    void setGeomAvgT3(double aT3) { geomAvgT3 = aT3; }
    void setGeomAvgT4(double aT4) { geomAvgT4 = aT4; }
    //accossers
    int getId() { return id; }
	  void print(FILE *f = stdout);
    double getAvgG() { return avgG; }
    double getGeomAvgG() { return geomAvgG; }
    double getAvgT1() { return avgT1; }
    double getAvgT2() { return avgT2; }
    double getAvgT3() { return avgT3; }
    double getAvgT4() { return avgT4; }
    double getGeomAvgT1() { return geomAvgT1; }
    double getGeomAvgT2() { return geomAvgT2; }
    double getGeomAvgT3() { return geomAvgT3; }
    double getGeomAvgT4() { return geomAvgT4; }
    
  private:
    int id;
    double avgG;
    double avgT1;
    double avgT2;
    double avgT3;
    double avgT4;
    double geomAvgG;
    double geomAvgT1;
    double geomAvgT2;
    double geomAvgT3;
    double geomAvgT4;
  };
  
  class ModuleAvgGS2L {
  	public:
  		inline bool operator () (Module *m1, Module *m2)
  		{
  			return m1->getAvgG() < m2->getAvgG();
  		}
  };
  
  class ModuleGeomAvgGS2L {
  	public:
  		inline bool operator () (Module *m1, Module *m2)
  		{
  			return m1->getGeomAvgG() < m2->getGeomAvgG();
  		}
  };

}

#endif //__MODULE_H__
