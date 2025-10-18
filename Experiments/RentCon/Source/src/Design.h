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

#ifndef __DESIGN_H__
#define __DESIGN_H__

#include <map>
#include <cassert>
#include <fstream>
#include <iostream>

#include "Gate.h"
#include "typedefs.h"
#include "Net.h"
#include "Subnet.h"
#include "Chip.h"
#include "CellMaster.h"
#include "Method.h"

#include "ABKCommon/abkcommon.h"
#include "ABKCommon/abktimer.h"
#include "ABKCommon/abkmessagebuf.h"

using namespace std;

namespace DESIGN {

  class Design {
    public:
      //constructors
      Design() : currentCellMaster(NULL), m_verb("1_1_1"), m_expGateNum(0), m_expNetNum(0), m_expSpecialNetNum(0), m_expPortNum(0)
      {
      }
      //modifiers
      void addGate(Gate *g) { m_gateMap[g->getName()] = g; }
      void addChipGate(Gate *g) { chip.addGate(g); }
      void addPad(Pad *p) { pads[p->getName()] = p; }
      void addPin(Pin *t) { pins.push_back(t); }
      void addNet(Net *n) { nets.push_back(n); }
      void setDim(int x1, int y1, int x2, int y2)
      {
        chip.setDim(x1, y1, x2, y2);
      }
      void addSubnet(Subnet *s) { subnets.push_back(s); }
			void addLefName(char *lName) { m_lefNames.push_back(lName); }
			void addDefName(char *dName) { m_defNames.push_back(dName); }
			void addCellKeyName(char *kName) { m_cellKeyNames.push_back(kName); }
			void addCellFullName(char *fName) { m_cellFullNames.push_back(fName); }
      //accossers
      PadMap &getPadMap() { return pads; }
      
      Gate *getGateByName(string gateName)
      {
      	GateMapItr itr = m_gateMap.find(gateName);
      	if (itr == m_gateMap.end())
      		return NULL;
      	return (*itr).second;
      }
      Pad *getPadByName(string padName)
      {
      	PadMapItr itr = pads.find(padName);
      	if (itr == pads.end())
      		return NULL;
      	return (*itr).second;
      }
      bool isPadName(string name)
      {
      	PadMapItr itr = pads.find(name);
      	if (itr == pads.end())
      		return false;
      	return true;
      }
      bool isGateName(string name)
      {
      	GateMapItr itr = m_gateMap.find(name);
      	if (itr == m_gateMap.end())
      		return false;
      	return true;
      }

      StrIntMap &getGateNameIdMap() { return gateNameIdMap; }
      StrIntMap &getPadNameIdMap() { return padNameIdMap; }
      StrIntMap &getSubnetNameIdMap() { return subnetNameIdMap; }
      StrVector &getCellNameVector() { return cellNameVect; }
      void print();
      Pin *getPin(int i) { abkassert(0 <= i && i < pins.size(), "Error in index in getPin()"); return pins[i]; }
      int getNetNum() { return nets.size(); }
      Net *getNet(int i) { abkassert(0 <= i && i < nets.size(), "Error in index in getNet()"); return nets[i]; }
      int getLX() { return chip.getLX(); }
      int getRX() { return chip.getRX(); }
      int getBY() { return chip.getBY(); }
      int getTY() { return chip.getTY(); }
      int getChipWidth() { return chip.getWidth(); }
      int getChipHeight() { return chip.getHeight(); }
      Chip & getChip() { return chip; }
      void printBox(int lx, int by, int rx, int ty) { cout<<"["<<lx<<","<<by<<"]->["<<rx<<","<<ty<<"]"; }
      void printPoint(int x, int y) { cout<<"["<<x<<","<<y<<"]"; }
      //functions
      void chipPartition();
      void setRowHeight(int h) { chip.setRowH(h); }
      void setRowWidth(int w) { chip.setRowW(w); }
      int getRowHeight() { return chip.getRowH(); }
      int getRowWidth() { return chip.getRowW(); }
			void initSiteOrient(int numH) { chip.initSiteOrient(numH); }
			void setSiteOrient(int index, int value) { chip.setSiteOrient(index, value); }
			int getSiteOrient(int index) { chip.getSiteOrient(index); }
			void setChipLeft(int l) { chip.setChipLeft(l); }
			void setChipRight(int r) { chip.setChipRight(r); }
			void setChipBottom(int b) { chip.setChipBottom(b); }
			void setChipTop(int t) { chip.setChipTop(t); }
			void setChipX(int x) { chip.setChipX(x); }
			void setChipY(int y) { chip.setChipY(y); }
			int getChipLeft() { return chip.getChipLeft(); }
			int getChipRight() { return chip.getChipRight(); }
			int getChipBottom() { return chip.getChipBottom(); }
			int getChipTop() { return chip.getChipTop(); }
			int getChipX() { return chip.getChipX(); }
			int getChipY() { return chip.getChipY(); }
			int getLefFileNum() { return m_lefNames.size(); }
			string getLefName(int index) { abkassert(0 <= index && index < m_lefNames.size(), "Error in index in getLefName()"); return m_lefNames[index]; }
			int getDefFileNum() { return m_defNames.size(); }
			string getDefName(int index) { abkassert(0 <= index && index < m_defNames.size(), "Error in index in getDefName()"); return m_defNames[index]; }
			void rectSamplingRent();
			void countPins(int partIndex);
			void evaluateRent();
			void addCellMaster(CellMaster *c) { cellMasterMap[c->getName()] = c; currentCellMaster = c; }
			CellMaster *getCellMaster(string cellName);
			CellMaster *getCurrentCellMaster() { return currentCellMaster; }
			CellMasterMap & getCellMasterMap() { return cellMasterMap; }
			void setLefDefFactor (double d) { lefDefFactor = d; }
			double getLefDefFactor () { return lefDefFactor; }
			void addModulesEachRectSize(BoxVector &bv);
			void addModulesEachGateNum(BoxVector &bv);
			void addModulesEachRectSize(BoxVector &bv, Method *method);
			void addModulesEachGateNum(BoxVector &bv, Method *method);
			void compAvgK();
			void BFSVisit(int gId, int cellNum, BoolVector &inside);
			void graphTraversalRent();
			void saveRent();
//			void addModulesBFS(IntVector &gIds, int cellNum, Method *method);
//			void BFSVisit(int gId, int cellNum, BoolVector &inside);
			void compTermsBFS(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms);
			void compTermsClustering(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms);
			void compTermsClusteringNew(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms);
			void compTermsClusteringNew2(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms);
	    //(hyper edge model with differentiated pin directions) counts all source-sink connections of the same net crossing the boundary as one pin;
			int getType2Terms(BoolVector &inside, NetSet &netSet);
			void MLPartRent();
			void MLPart(Cluster *c, ClusterVector &resultCV);
			bool oneLevelPartition(ClusterVector &cv, Method *method);
			void setStartsPerRun(int i) { m_startsPerRun = i; }
			void setTotalRuns(int i) { m_totalRuns = i; }
			void setSampleSizeNum(int i) { m_sampleSizeNum = i; }
			int getStartsPerRun() { return m_startsPerRun; }
			int getTotalRuns() { return m_totalRuns; }
			int getSampleNumGT() { return m_sampleNumGT; }
			int getSampleNumRS() { return m_sampleNumRS; }
			int getSampleSizeNum() { return m_sampleSizeNum; }
			void countTerms(BoolVector &inside, double &sumG, double &sumT1, double &sumT2, double &sumT3);
			Verbosity & getVerb() { return m_verb; }
			void readAUX(string fileName);
      GateMap &getGateMap() { return m_gateMap; }
			void setExpGateNum(int i) { m_expGateNum = i; }
			void setExpNetNum(int i) { m_expNetNum = i; }
			void setExpSpecialNetNum(int i) { m_expSpecialNetNum = i; }
			void setExpPortNum(int i) { m_expPortNum = i; }
			int getExpGateNum() { return m_expGateNum; }
			int getExpNetNum() { return m_expNetNum; }
			int getExpSpecialNetNum() { return m_expSpecialNetNum; }
			int getExpPortNum() { return m_expPortNum; }
			int getGateNum() { return chip.getGateNum(); }
      int getPinNum() { return pins.size(); }
			int getPadNum() { return pads.size(); }
			void reportMem();
			StrVector &getCellKeyNames() { return m_cellKeyNames; }
			StrVector &getCellFullNames() { return m_cellFullNames; }
			void setSeedCP(unsigned s) { m_seedCP = s; }
			void setSeedGT(unsigned s) { m_seedGT = s; }
			void setSeedRS(unsigned s) { m_seedRS = s; }
			unsigned getSeedCP() { return m_seedCP; }
			unsigned getSeedGT() { return m_seedGT; }
			unsigned getSeedRS() { return m_seedRS; }
			void setSampleNumGT(int n) { m_sampleNumGT = n; }
			void setSampleNumRS(int n) { m_sampleNumRS = n; }
			void setNoCP(bool b) { m_noCP = b; }
			void setNoGT(bool b) { m_noGT = b; }
			void setNoRS(bool b) { m_noRS = b; }
			
		private:
      GateMap m_gateMap;
      PinVector pins;
      PadMap pads;
      
      NetVector nets;
      SubnetVector subnets;
      CellMasterMap cellMasterMap;
      CellMaster *currentCellMaster;
      Chip chip;
      StrIntMap gateNameIdMap;
			StrIntMap subnetNameIdMap;
      StrIntMap padNameIdMap;
      StrVector cellNameVect;
      StrIntMap cellNameIdMap;
      StrVector m_lefNames;
      StrVector m_defNames;
      StrVector m_cellKeyNames;
      StrVector m_cellFullNames;
			
	    //evaluate rent parameter
	    PartitionVector partitions;
	    IntIntMap siteGateMap;
	    IntPartMap partMap;
	    double lefDefFactor;
	    MethodVector methods;
	    double m_avgK;
			double m_geomAvgK;
			ClusterVV clusterVV;
			int m_startsPerRun;
			int m_totalRuns;
	  	int m_sampleNumGT;
	  	int m_sampleNumRS;
	  	int m_sampleSizeNum;
	  	Verbosity m_verb;
	  	//expected nets, cell instances
	  	int m_expGateNum;
	  	int m_expNetNum;
	  	int m_expSpecialNetNum;
	  	int m_expPortNum;
			unsigned m_seedCP;
			unsigned m_seedGT;
			unsigned m_seedRS;
			bool m_noCP;
			bool m_noGT;
			bool m_noRS;
  };
}
#endif //__DESIGN_H__
