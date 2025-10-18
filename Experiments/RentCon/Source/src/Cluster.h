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

#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include <cassert>
#include "typedefs.h"
#include "Gate.h"
#include "ABKCommon/abkassert.h"

namespace DESIGN {
  
  class Cluster {
  public:
    //constructors
    ~Cluster()
    {
    	l_gates.clear();
    }
    Cluster(int i) : l_id(i)
    {
    }
    //modifiers
    void addGate(Gate *g) { l_gates.push_back(g); }
    //accossers
    int &getId() { return l_id; }
    Gate *getGate(int index) { abkassert(0 <= index && index < l_gates.size(), "Error in getGate()"); return l_gates[index]; }
    int getGateNum() { return l_gates.size(); }
    GateVector &getGates() { return l_gates; }
  	void setLevel(int l) { levelIndex = l; }
  	int getLevel() { return levelIndex; }
    void print();
  private:
    int l_id;
    GateVector l_gates;
    int levelIndex;
  };
  
}

#endif //__CLUSTER_H__
