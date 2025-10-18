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

#ifndef __PAD_H__
#define __PAD_H__
#include <string>

#include "typedefs.h"
#include "Box.h"
#include "Net.h"
#include "ABKCommon/abkassert.h"

namespace DESIGN {
  enum PadType {PrimiaryInput, PrimiaryOutput, InputOutput};
  
  class Pad {
  public:
    Pad(int tId, string tName, string nName, PadType t, int lx, int by, int rx, int ty):id(tId), name(tName), netName(nName), type(t)
    {
      bbox.set(lx, by, rx, ty);
    }

    Pad(int tId, string tName):id(tId), name(tName)
      {
      }
    Pad(int tId, string tName, PadType t):id(tId), name(tName), type(t)
      {
      }
    Pad(int tId, string tName, PadType t, int lx, int by, int rx, int ty):id(tId), name(tName), type(t)
      {
        bbox.set(lx, by, rx, ty);
      }
    Pad(int tId, string tName, PadType t, double d, int lx, int by, int rx, int ty):id(tId), name(tName), type(t)
      {
        bbox.set(lx, by, rx, ty);
      }
    
    //accessors
    int getId() { return id; }
    string getName() { return name; }
    PadType getPadType() { return type; }
    int getLX() { return bbox.left(); }
    int getRX() { return bbox.right(); }
    int getBY() { return bbox.bottom(); }
    int getTY() { return bbox.top(); }    
    void print();
    
  private:
    int id;
    string name;
    string netName;
    PadType type;
    Box bbox;
  };
  
}
#endif //__PAD_H__
