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

#ifndef __CELL_MASTER_H__
#define __CELL_MASTER_H__
#include <string>

#include "typedefs.h"
#include "PinMaster.h"

using namespace std;

namespace DESIGN {
  
  class CellMaster {
  public:
    CellMaster() {}
    CellMaster(string pName) : name(pName)
      {
      }
    
    //modifiers
    void addPinMaster(PinMaster *pin) { pinMasterMap[pin->getName()] = pin; }
    //accessors
    string getName() { return name; }
    void setOriginXY(double x, double y) { originX = x; originY = y; }
    double getOriginX() { return originX; }
    double getOriginY() { return originY; }
    void setWidth(double w) { width = w; }
    void setHeight(double h) { height = h; }
    double getWidth() { return width; }
    double getHeight() { return height; }
    PinMasterMap &getPinMasterMap() { return pinMasterMap; }
    void check();
  private:
    string name;
    PinMasterMap pinMasterMap;
    double originX, originY, width, height;
  };
}
#endif //__CELL_MASTER_H__
