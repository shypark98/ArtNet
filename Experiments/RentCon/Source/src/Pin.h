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

#ifndef __PIN_H__
#define __PIN_H__
#include <string>

#include "Box.h"
#include "typedefs.h"
#include "Gate.h"
#include "Subnet.h"
#include "ABKCommon/abkassert.h"

using namespace std;

namespace DESIGN {
  
  enum PinType { INPUT, OUTPUT, INOUT, UNKNOWN };
  
  class Gate;
  
  class Pin {
  public:
    Pin() {}
    Pin(int pId, string pName, PinType t, int cX, int cY)
      :id(pId), name(pName), type(t), centerX(cX), centerY(cY)
    {
    }
    Pin(int pId, string pName, PinType t, int leftIn, int bottomIn, int rightIn, int topIn)
      :id(pId), name(pName), type(t), bbox(leftIn, bottomIn, rightIn, topIn), centerX((leftIn+rightIn)/2), centerY((bottomIn+topIn)/2)
    {
    }
    ~Pin();
    
    //modifiers
    void setX(int x) { centerX = x; }
    void setY(int y) { centerY = y; }
    void setType(PinType t) { type = t; }
    void setGate(Gate *g) { gatePtr = g; }
    void setPinType(PinType t) { type = t; }
    //accessors
    int getId() { return id; }
    string getName() { return name; }
    int &x() { return centerX; }
    int &y() { return centerY; }
    PinType getType() { return type; }
    Gate *getGate() { abkassert(gatePtr, "Error in getGate()"); return gatePtr; }
    void addSubnet(Subnet *s) { subnets.push_back(s); }
    int getSubnetNum() { return subnets.size(); }
    Subnet *getSubnet(int index) { abkassert(0 <= index && index < (int)subnets.size(), "Error in getSubnet()"); return subnets[index]; }
    
    void print();
    
  private:
    int id;
    string name;
    Box bbox;
    PinType type;
    int centerX, centerY;
    Gate *gatePtr;
    SubnetVector subnets;
  };
  
  class PinXS2L
    {
    public:
      inline bool operator()(Pin *pin1, Pin *pin2) { return pin1->x() < pin2->x(); }
    };
  
  class PinYS2L
    {
    public:
      inline bool operator()(Pin *pin1, Pin *pin2) { return pin1->y() < pin2->y(); }
    };
}
#endif //__PIN_H__
