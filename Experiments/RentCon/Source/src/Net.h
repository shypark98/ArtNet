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

#ifndef __NET_H__
#define __NET_H__

#include <string>
#include "Pin.h"
#include "typedefs.h"
#include "Pad.h"

using namespace std;

namespace DESIGN {
  enum NetType {SIGNAL, PG};

  class Net
    {
    public:
      Net() : type(SIGNAL) {}
      Net(int nId, string nName):id(nId), name(nName), type(SIGNAL)
      {
      }
      ~Net();
      //modifiers
      void addSourcePin(Pin *t) { pins.insert(pins.begin(), t); }
      void addSinkPin(Pin *t) { pins.push_back(t); }
      void addPin(Pin *t) { pins.push_back(t); }
      void addPad(Pad *p) { pads.push_back(p); }
      void addSourcePad(Pad *p) { pads.insert(pads.begin(), p); }
      void addSinkPad(Pad *p) { pads.push_back(p); }
      int getPinNum() { return pins.size(); }
      Pin *getPin(int index)
      {
      	if(!(0 <= index && index < pins.size()))
		    {
		    	fprintf(stdout, "Error in getPin of Net %s: index exceeds bounds %s. Exiting ...", name.c_str());
		    	exit(-1);
		    }
      	return pins[index];
      }
      Pin *getSourcePin();
      int getPadNum() { return pads.size(); }
      Pad *getPad(int index)
      {
      	if(!(0 <= index && index < pads.size()))
		    {
		    	fprintf(stdout, "Error in getPad of Net %s: index exceeds bounds %s. Exiting ...", name.c_str());
		    	exit(-1);
		    }
				return pads[index];
			}
      Pad *getSourcePad();
      void makeSourcePin();
      void makeSinkPins();
      void checkPins();
      //accessors
      int getId() { return id; }
      string getName() { return name; }
      void setType(NetType t) { type = t; }
      NetType getType() { return type; }
      //algs
      void print();
    private:
      int id;
      string name;
      PinVector pins;
      PadVector pads;
      NetType type;
    };
}
#endif //__NET_H__
