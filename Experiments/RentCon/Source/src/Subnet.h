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

#ifndef __SUBNET_H__
#define __SUBNET_H__

#include <string>
#include "typedefs.h"
#include "Pin.h"
#include "Pad.h"
#include "ABKCommon/abkassert.h"

using namespace std;

namespace DESIGN {
  class Subnet
    {
    public:
      Subnet() {}
      Subnet(int sId, string sName):id(sId), name(sName), inputPin(NULL), outputPin(NULL)
      {
      }
      Subnet(int sId, string sName, bool iPad, bool oPad):id(sId), name(sName), iPad(iPad), oPad(oPad), inputPin(NULL), outputPin(NULL), inputPad(NULL), outputPad(NULL) {  }
      ~Subnet() {}
      //modifiers
      void setInputPin(Pin *t) { inputPin = t; }
      void setOutputPin(Pin *t) { outputPin = t; }
      void setInputPad(Pad *p) { inputPad = p; }
      void setOutputPad(Pad *p) { outputPad = p; }
      Pin *getInputPin()
      {
        abkassert(!iPad && inputPin, "Error in getInputPin()");
        return inputPin;
      }

      Pin *getOutputPin()
      {
        abkassert(!oPad && outputPin, "Error in getOutputPin()");
        return outputPin;
      }
      
      bool inputIsPad() { return iPad; }
      bool outputIsPad() { return oPad; }
      void setInputIsPad(bool b) { iPad = b; }
      void setOutputIsPad(bool b) { oPad = b; }
      void setNetId(int i) { nId = i; }
      
      //accessors
      int getId() { return id; }
      string getName() { return name; }
      int getNetId() { return nId; }
      
      //algs
      void print();
    private:
      int id;
      int nId;
      string name;
      Pin *inputPin;
      Pin *outputPin;
      Pad *inputPad;
      Pad *outputPad;
      bool iPad;
      bool oPad;
    };
}
#endif //__SUBNET_H__
