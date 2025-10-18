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

#ifndef __CHIP_H__
#define __CHIP_H__

#include <cassert>
#include "Gate.h"
#include "typedefs.h"
#include "ABKCommon/abkassert.h"

namespace DESIGN {
  class Gate;
  
  class Chip {
  public:
    //constructors
    Chip()
    {
    	m_gates.clear();
    }
    Chip(int i, int rNum, int cNum, int rW, int rH, int x1, int y1, int x2, int y2) : id(i), siteRowNum(rNum), siteColNum(cNum), lx(x1), rx(x2), by(y1), ty(y2)
    {
    	m_gates.clear();
    }
    //modifiers
    void setId(int i) { id = i; }
    void addGate(Gate *g) { m_gates.push_back(g); }
    void removeGate(int gId);
    void setDim(int x1, int y1, int x2, int y2)
    {
      lx = x1;
      rx = x2;
      by = y1;
      ty = y2;
      width = x2-x1;
      height = y2-y1;
    }
    //accossers
    int getId() { return id; }
    int getLX() { return lx; }
    int getRX() { return rx; }
    int getBY() { return by; }
    int getTY() { return ty; }
    Gate* getGate(int i) { abkassert(0 <= i && i < (int)m_gates.size(), "Error in getGate()"); return m_gates[i]; }
    GateVector &getGates() { return m_gates; }
    int getGateNum() { return m_gates.size(); }
    void setRowW(int w) { rowW = w; }
    void setRowH(int h) { rowH = h; }
    int getRowW() { return rowW; }
    int getRowH() { return rowH; }
		void setSiteRowNum(int n) { siteRowNum = n; }
		void setSiteColNum(int n) { siteColNum = n; }
		int getSiteRowNum() { return siteRowNum; }
		int getSiteColNum() { return siteColNum; }
		int getWidth() { return rx-lx; }
		int getHeight() { return ty-by; }
		void initSiteOrient(int numH) { siteOrient.assign(numH, 0); }
		void setSiteOrient(int index, int value) { abkassert(0 <= index && index <= siteOrient.size(), "Error in setSiteOrient()"); siteOrient[index] = value; }
		int getSiteOrient(int index) { abkassert(0 <= index && index <= siteOrient.size(), "Error in getSiteOrient()"); return siteOrient[index]; }
		void setChipLeft(int l) { lx = l; }
		void setChipRight(int r) { rx = r; }
		void setChipBottom(int b) { by = b; }
		void setChipTop(int t) { ty = t; }
		void setChipX(int x) { width = x; }
		void setChipY(int y) { height = y; }
		int getChipLeft() { return lx; }
		int getChipRight() { return rx; }
		int getChipBottom() { return by; }
		int getChipTop() { return ty; }
		int getChipX() { return width; }
		int getChipY() { return height; }
    void print();
    
  private:
    int id;
    int siteRowNum, siteColNum;
    int rowW, rowH;
    int lx;
    int rx;
    int by;
    int ty;
    int width, height;
    GateVector m_gates;
		IntVector siteOrient;
  };

}

#endif //__CHIP_H__
