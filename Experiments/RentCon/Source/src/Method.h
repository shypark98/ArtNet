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

#ifndef __METHOD_H__
#define __METHOD_H__

#include <cassert>
#include "typedefs.h"
#include "Module.h"
#include "ABKCommon/abkcommon.h"
#include "ABKCommon/abkassert.h"

namespace DESIGN {
  
  class Method {
  public:
//    //constructors
    Method() { }
    Method(int i) : id(i) { }
    
    int getId() { return id; }
    int getModuleNum() { return modules.size(); }
		void addModule(Module *m) { modules.push_back(m); }
    ModuleVector &getModules() { return modules; }
    Module* getModule(int i) { abkassert(0 <= i && i < (int)modules.size(), "Error in getModule()"); return modules[i]; }
    void setMSG(string m) { msg = m; }
    void setAvgK(double k) { avgK = k; }
		void setGeomAvgK(double k) { geomAvgK = k; }
    double getRentP1() { return rentP1; }
    double getRentP2() { return rentP2; }
		void clearMem();
    void print(FILE *file);
	  void printRents(FILE *file);
	  void printModules(FILE *file);
		void linCurvFit();
		double fitRent(double *x, double *y, int n);
  private:
    int id;
    string msg;
    ModuleVector modules;
    double avgK;
    double geomAvgK;
    double rentP1, rentP2, rentP3, rentP4;
    double geomRentP1, geomRentP2, geomRentP3, geomRentP4;
  };
}

#endif //__METHOD_H__
