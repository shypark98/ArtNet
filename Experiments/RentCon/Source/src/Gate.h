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

#ifndef __GATE_H__
#define __GATE_H__

#include <cassert>
#include "Box.h"
#include "typedefs.h"
#include "Subnet.h"
#include "Net.h"
#include "Pin.h"
#include "ABKCommon/abkassert.h"

namespace DESIGN {
  
  class Pin;
  
  enum GateType {ORDINARY_GATE, SPECIAL_GATE};
  
  class Gate {
  public:
    //constructors
    Gate(int i, string gName, string cName):id(i), l_name(gName), cellName(cName), type(ORDINARY_GATE), m_effPinNum(0), m_adjNumOutCluster(0), m_incConNum(0)
    {
    }
    Gate(int i, string gName, string cName, int tlx, int tby, int twidth, int theight):id(i), l_name(gName), cellName(cName), type(ORDINARY_GATE), lx(tlx), by(tby), width(twidth), height(theight), m_effPinNum(0), m_adjNumOutCluster(0), m_incConNum(0)
    {
      bbox.set(tlx, tby, tlx+twidth, tby+theight);
      centerX = tlx + twidth/2;
      centerY = tby + theight/2;
    }
    Gate(int i, bool ffflag, string gName, string cName, double l, int tlx, int tby, int twidth, int theight):id(i), FFFlag(ffflag), l_name(gName), cellName(cName), type(ORDINARY_GATE), lx(tlx), by(tby), width(twidth), height(theight), m_effPinNum(0), m_adjNumOutCluster(0), m_incConNum(0)
    {
      bbox.set(tlx, tby, tlx+twidth, tby+theight);
      centerX = tlx + twidth/2;
      centerY = tby + theight/2;
    }
    ~Gate();
    //modifiers
    void setCoord(int tlx, int tby, int twidth, int theight)
    {
    	width = twidth;
    	height = theight;
    	bbox.set(tlx, tby, tlx+twidth, tby+theight);
    	centerX = tlx + twidth/2;
    	centerY = tby + theight/2;
    }
    void incEffPinNum(int i) { m_effPinNum += i; }
    int getEffPinNum() { return m_effPinNum; }
    void setId(int i) { id = i; }
    void setFFFlag(bool flag) { FFFlag = flag; }
    void setGateType(GateType t) { type = t; }
    void addGate(Gate *g);
		void addPin(Pin *p);
    Pin *getPinByName(string name)
    {
    	PinMapItr itr = m_pinMap.find(name);
    	if (itr == m_pinMap.end())
    		return NULL;
    	
  		return (*itr).second;
    }
    int getPinNum() { return m_pinMap.size(); }
	  void addAdjGate(Gate *g);
    int getAdjGateNum() { return m_adjGates.size(); }
    Gate *getAdjGate(int index) { abkassert(0 <= index && index < m_adjGates.size(), "Error in index in getAdjGate()"); return m_adjGates[index]; }
//    void addFaninGate(Gate *g);
//    void addFanoutGate(Gate *g);
//    int getFaninGateNum() { return faninGates.size(); }
//    Gate *getFaninGate(int index) { abkassert(0 <= index && index < faninGates.size(), "Error in index in getFaninGate()"); return faninGates[index]; }
//    int getFanoutGateNum() { return fanoutGates.size(); }
//    Gate *getFanoutGate(int index) { abkassert(0 <= index && index < fanoutGates.size(), "Error in index in getFanoutGate()"); return fanoutGates[index]; }
//    void addFaninPad(Pad *p);
//    void addFanoutPad(Pad *p);
//    int getFaninPadNum() { return faninPads.size(); }
//    Pad *getFaninPad(int index) { abkassert(0 <= index && index < faninPads.size(), "Error in index in getFaninPad()"); return faninPads[index]; }
//    int getFanoutPadNum() { return fanoutPads.size(); }
//    Pad *getFanoutPad(int index) { abkassert(0 <= index && index < fanoutPads.size(), "Error in index in getFanoutPad()"); return fanoutPads[index]; }
    void addInputSubnet(Subnet *s) { inputSubnets.push_back(s); }
    void addOutputSubnet(Subnet *s) { outputSubnets.push_back(s); }
    int getInputSubnetNum() { return inputSubnets.size(); }
    int getOutputSubnetNum() { return outputSubnets.size(); }
    Subnet *getInputSubnet(int index) { abkassert(0 <= index && index < inputSubnets.size(), "Error in index in getInputSubnet()"); return inputSubnets[index]; }
    Subnet *getOutputSubnet(int index) { abkassert(0 <= index && index < outputSubnets.size(), "Error in index in getOutputSubnet()"); return outputSubnets[index]; }
    //accossers
    int getId() { return id; }
    bool getFFFlag() { return FFFlag; }
    string getName() { return l_name; }
    string getCellName() { return cellName; }
    void setType(GateType t) { type = t; }
    GateType getType() { return type; }
    int getLX() { return lx; }
    int getRX() { return lx+width; }
    int getBY() { return by; }
    int getTY() { return by+height; }
    int getWidth() { return width; }
    int getHeight() { return height; }
    void setGridX(int gx) { gridX = gx; }
    void setGridY(int gy) { gridY = gy; }
    
    int getGridX() { return gridX; }
    int getGridY() { return gridY; }
    int getCenterX() { return centerX; }
    int getCenterY() { return centerY; }
    void print();
    //for graph traversal method
    //new
		void initIncConNum() { m_incConNum = m_nets.size(); }
    void setIncConNum(int i) { m_incConNum = i; }
    void incIncConNum() { ++ m_incConNum; }
    void decIncConNum() { -- m_incConNum; }
    //accossers
    int getIncConNum() { return m_incConNum; }

    //old
    //modifiers
		void initAdjNumOutCluster() { m_adjNumOutCluster = m_adjGates.size(); }
    void setAdjNumOutCluster(int i) { m_adjNumOutCluster = i; }
    void incAdjNumOutCluster() { ++ m_adjNumOutCluster; }
    void decAdjNumOutCluster() { -- m_adjNumOutCluster; }
    //accossers
    int getAdjNumOutCluster() { return m_adjNumOutCluster; }
    
    NetVector &getNets() { return m_nets; }
    void addNet(Net *n);
    int getNetNum() { return m_nets.size(); }
    Net *getNet(int index) { abkassert(0 <= index && index < m_nets.size(), "Error in index in getNet()"); return m_nets[index]; }
    bool insideBox(Box &b);
    //(graph edge model with differentiated pin directions) counts each source-sink connection crossing the boundary as one pin;
		double getType1Terms(BoolVector &inside);
    //(graph edge model without pin directions) counts each connection crossing the boundary as one pin;
		double getType3Terms(BoolVector &inside);
		
  private:
    int id;
    string l_name;
    string cellName;
    bool FFFlag;
    GateType type;
//    GateVector faninGates;
//    GateVector fanoutGates;
//    PadVector faninPads;
//    PadVector fanoutPads;
		GateVector m_adjGates;
    PinMap m_pinMap;
    Box bbox;
    int gridX;
    int gridY;
    int centerX;
    int centerY;
    int lx, by;
    int width, height;
    SubnetVector inputSubnets;
    SubnetVector outputSubnets;
    NetVector m_nets;
    int m_effPinNum;
    int m_adjNumOutCluster;
    int m_incConNum;
  };
  
  class GateL2S {
		public:
			inline bool operator () (Gate *g1, Gate *g2) { return g1->getAdjNumOutCluster() > g2->getAdjNumOutCluster(); }
  };

  class GateIncConL2S {
		public:
			inline bool operator () (Gate *g1, Gate *g2) { return g1->getIncConNum() > g2->getIncConNum(); }
  };
}

#endif //__GATE_H__
